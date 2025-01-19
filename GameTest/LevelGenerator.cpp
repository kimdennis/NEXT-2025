#include "stdafx.h"
#include "LevelGenerator.h"
#include "Enemy.h"
#include "Collectible.h"
#include "Wall.h"
#include <random>
#include <ctime>

float LevelGenerator::GetRandomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

bool LevelGenerator::IsPositionValid(float x, float y, float radius, 
    const std::vector<std::unique_ptr<GameObject>>& existingObjects) {
    const float MIN_DISTANCE = 40.0f;
    const float WALL_SPAWN_SAFE_ZONE = 100.0f; // Larger safe zone for walls near spawn
    
    // Get spawn position first
    float spawnX, spawnY;
    if (m_hole) {
        m_hole->GetStartPosition(spawnX, spawnY);
        
        // If this is a wall being placed (larger radius typically means it's a wall)
        if (radius > 100.0f) {
            // Calculate expanded wall bounds
            float wallLeft = x - radius;
            float wallRight = x + radius;
            float wallTop = y - radius;
            float wallBottom = y + radius;
            
            // Check if wall is too close to spawn point
            if (spawnX + WALL_SPAWN_SAFE_ZONE > wallLeft && 
                spawnX - WALL_SPAWN_SAFE_ZONE < wallRight &&
                spawnY + WALL_SPAWN_SAFE_ZONE > wallTop && 
                spawnY - WALL_SPAWN_SAFE_ZONE < wallBottom) {
                return false;
            }
        }
    }
    
    // Check distance from each existing object
    for (const auto& obj : existingObjects) {
        float objX, objY;
        obj->GetPosition(objX, objY);
        
        // Special handling for walls
        if (const Wall* wall = dynamic_cast<const Wall*>(obj.get())) {
            float wallLeft = objX - wall->GetWidth()/2;
            float wallRight = objX + wall->GetWidth()/2;
            float wallTop = objY - wall->GetHeight()/2;
            float wallBottom = objY + wall->GetHeight()/2;
            
            wallLeft -= MIN_DISTANCE;
            wallRight += MIN_DISTANCE;
            wallTop -= MIN_DISTANCE;
            wallBottom += MIN_DISTANCE;
            
            if (x + radius > wallLeft && x - radius < wallRight &&
                y + radius > wallTop && y - radius < wallBottom) {
                return false;
            }
        }
        else {
            float dx = x - objX;
            float dy = y - objY;
            float distanceSquared = dx * dx + dy * dy;
            
            if (distanceSquared < MIN_DISTANCE * MIN_DISTANCE) {
                return false;
            }
        }
    }
    
    // Check distance from hole and starting position if hole exists
    if (m_hole && IsTooCloseToHole(x, y, MIN_DISTANCE)) {
        return false;
    }
    
    return true;
}

bool LevelGenerator::IsTooCloseToHole(float x, float y, float minDistance) {
    if (!m_hole) return false;
    
    // Use a larger safe zone for the starting position
    const float SPAWN_SAFE_ZONE = 80.0f;  // Larger clear area around spawn
    
    // Check distance from start position with larger safe zone
    float startX, startY;
    m_hole->GetStartPosition(startX, startY);
    float dx = x - startX;
    float dy = y - startY;
    if (dx * dx + dy * dy < SPAWN_SAFE_ZONE * SPAWN_SAFE_ZONE) {
        return true;
    }
    
    // Check distance from hole position with regular minimum distance
    float holeX, holeY;
    m_hole->GetPosition(holeX, holeY);
    dx = x - holeX;
    dy = y - holeY;
    if (dx * dx + dy * dy < minDistance * minDistance) {
        return true;
    }
    
    return false;
}

LevelGenerator::LevelGenerator() : m_hole(nullptr) {
    InitializeTemplates();
}

void LevelGenerator::InitializeTemplates() {
    // Template 1: Dense Zigzag Maze
    CourseTemplate zigzag;
    zigzag.startX = 0.1f;
    zigzag.startY = 0.5f;
    zigzag.holeX = 0.9f;
    zigzag.holeY = 0.5f;
    
    // Create a denser zigzag pattern with multiple layers
    for (float x = 0.2f; x < 0.9f; x += 0.1f) {
        zigzag.walls.push_back({x, 0.3f, 150.0f, 20.0f, 0.0f});
        zigzag.walls.push_back({x + 0.05f, 0.7f, 150.0f, 20.0f, 0.0f});
    }
    // Add vertical barriers
    for (float x = 0.25f; x < 0.85f; x += 0.2f) {
        zigzag.walls.push_back({x, 0.5f, 20.0f, 200.0f, 0.0f});
    }
    
    // Add collectibles along the path
    for (float x = 0.25f; x < 0.85f; x += 0.15f) {
        zigzag.collectibles.push_back(std::make_pair(x, 0.5f));
    }
    
    // Add enemies guarding the path
    zigzag.enemies.push_back(std::make_pair(0.3f, 0.5f));
    zigzag.enemies.push_back(std::make_pair(0.6f, 0.5f));
    zigzag.enemies.push_back(std::make_pair(0.8f, 0.5f));
    
    m_courseTemplates.push_back(zigzag);

    // Template 2: Spiral Maze
    CourseTemplate spiral;
    spiral.startX = 0.1f;
    spiral.startY = 0.1f;
    spiral.holeX = 0.5f;
    spiral.holeY = 0.5f;
    
    // Create a tighter spiral pattern
    float spiralSize = 600.0f;
    for (float scale = 1.0f; scale > 0.3f; scale -= 0.2f) {
        spiral.walls.push_back({0.5f, 0.3f * scale, spiralSize * scale, 20.0f, 0.0f});
        spiral.walls.push_back({0.7f * scale, 0.5f, 20.0f, spiralSize * scale, 0.0f});
        spiral.walls.push_back({0.5f, 0.7f * scale, spiralSize * scale, 20.0f, 0.0f});
        spiral.walls.push_back({0.3f * scale, 0.5f, 20.0f, spiralSize * scale, 0.0f});
    }
    
    // Add collectibles along the spiral
    for (float scale = 0.9f; scale > 0.3f; scale -= 0.2f) {
        spiral.collectibles.push_back(std::make_pair(0.5f + scale * 0.3f, 0.5f));
        spiral.collectibles.push_back(std::make_pair(0.5f, 0.5f + scale * 0.3f));
    }
    
    // Add enemies at strategic points
    spiral.enemies.push_back(std::make_pair(0.5f, 0.7f));
    spiral.enemies.push_back(std::make_pair(0.7f, 0.5f));
    spiral.enemies.push_back(std::make_pair(0.5f, 0.3f));
    
    m_courseTemplates.push_back(spiral);

    // Template 3: Grid Maze
    CourseTemplate grid;
    grid.startX = 0.1f;
    grid.startY = 0.1f;
    grid.holeX = 0.9f;
    grid.holeY = 0.9f;
    
    // Create a dense grid pattern
    for (float x = 0.2f; x < 0.9f; x += 0.15f) {
        for (float y = 0.2f; y < 0.9f; y += 0.15f) {
            if (GetRandomFloat(0.0f, 1.0f) < 0.7f) {
                grid.walls.push_back({x, y, 100.0f, 20.0f, 0.0f});
                grid.walls.push_back({x, y, 20.0f, 100.0f, 0.0f});
            }
        }
    }

    // Add collectibles to grid
    for (float x = 0.3f; x < 0.8f; x += 0.2f) {
        for (float y = 0.3f; y < 0.8f; y += 0.2f) {
            grid.collectibles.push_back(std::make_pair(x, y));
        }
    }
    grid.enemies.push_back(std::make_pair(0.4f, 0.4f));
    grid.enemies.push_back(std::make_pair(0.6f, 0.6f));
    grid.enemies.push_back(std::make_pair(0.4f, 0.6f));
    grid.enemies.push_back(std::make_pair(0.6f, 0.4f));
    
    m_courseTemplates.push_back(grid);

    // Template 4: Pinball Style
    CourseTemplate pinball;
    pinball.startX = 0.1f;
    pinball.startY = 0.1f;
    pinball.holeX = 0.9f;
    pinball.holeY = 0.9f;
    
    // Add circular arrangement of walls
    for (float angle = 0; angle < 360; angle += 30) {
        float x = 0.5f + 0.3f * cos(angle * 3.14159f / 180.0f);
        float y = 0.5f + 0.3f * sin(angle * 3.14159f / 180.0f);
        pinball.walls.push_back({x, y, 80.0f, 20.0f, angle});
    }
    // Add inner obstacles
    for (int i = 0; i < 8; i++) {
        float x = GetRandomFloat(0.3f, 0.7f);
        float y = GetRandomFloat(0.3f, 0.7f);
        pinball.walls.push_back({x, y, 60.0f, 20.0f, GetRandomFloat(0.0f, 360.0f)});
    }

    // Add collectibles and enemies to pinball template
    for (float angle = 0; angle < 360; angle += 45) {
        float x = 0.5f + 0.2f * cos(angle * 3.14159f / 180.0f);
        float y = 0.5f + 0.2f * sin(angle * 3.14159f / 180.0f);
        pinball.collectibles.push_back({x, y});
    }
    pinball.enemies.push_back({0.5f, 0.3f});
    pinball.enemies.push_back({0.3f, 0.5f});
    pinball.enemies.push_back({0.7f, 0.5f});

    m_courseTemplates.push_back(pinball);

    // Template 5: Concentric Circles
    CourseTemplate circles;
    circles.startX = 0.1f;
    circles.startY = 0.5f;
    circles.holeX = 0.9f;
    circles.holeY = 0.5f;
    
    // Create concentric circle pattern using short walls
    for (float radius = 0.1f; radius < 0.4f; radius += 0.08f) {
        for (float angle = 0; angle < 360; angle += 20) {
            float x = 0.5f + radius * cos(angle * 3.14159f / 180.0f);
            float y = 0.5f + radius * sin(angle * 3.14159f / 180.0f);
            circles.walls.push_back({x, y, 40.0f, 20.0f, angle});
        }
    }

    // Add collectibles and enemies to circles template
    for (float radius = 0.15f; radius < 0.35f; radius += 0.1f) {
        for (float angle = 0; angle < 360; angle += 60) {
            float x = 0.5f + radius * cos(angle * 3.14159f / 180.0f);
            float y = 0.5f + radius * sin(angle * 3.14159f / 180.0f);
            circles.collectibles.push_back({x, y});
        }
    }
    circles.enemies.push_back({0.5f, 0.5f});
    circles.enemies.push_back({0.3f, 0.5f});
    circles.enemies.push_back({0.7f, 0.5f});

    m_courseTemplates.push_back(circles);
}

void LevelGenerator::ApplyCourseTemplate(Level* level, const CourseTemplate& templ) {
    float levelWidth = MAX_LEVEL_WIDTH - MIN_LEVEL_WIDTH;
    float levelHeight = MAX_LEVEL_HEIGHT - MIN_LEVEL_HEIGHT;

    // Keep trying different hole positions until we find a valid one
    float holeX, holeY;
    bool validHolePosition = false;
    int maxAttempts = 50;  // Prevent infinite loops
    
    do {
        // Randomize hole position slightly around template position
        holeX = MIN_LEVEL_WIDTH + (templ.holeX * levelWidth + GetRandomFloat(-50.0f, 50.0f));
        holeY = MIN_LEVEL_HEIGHT + (templ.holeY * levelHeight + GetRandomFloat(-50.0f, 50.0f));
        
        // Check if this position would overlap with any walls from the template
        validHolePosition = true;
        for (const auto& wallTemplate : templ.walls) {
            float wallX = MIN_LEVEL_WIDTH + (wallTemplate.relativeX * levelWidth);
            float wallY = MIN_LEVEL_HEIGHT + (wallTemplate.relativeY * levelHeight);
            
            // Calculate wall bounds
            float wallLeft = wallX - wallTemplate.width/2;
            float wallRight = wallX + wallTemplate.width/2;
            float wallTop = wallY - wallTemplate.height/2;
            float wallBottom = wallY + wallTemplate.height/2;
            
            // Add safety margin around hole
            const float HOLE_SAFETY_MARGIN = 30.0f;
            
            // Check if hole (with safety margin) overlaps with wall
            if (holeX + HOLE_SAFETY_MARGIN > wallLeft && 
                holeX - HOLE_SAFETY_MARGIN < wallRight && 
                holeY + HOLE_SAFETY_MARGIN > wallTop && 
                holeY - HOLE_SAFETY_MARGIN < wallBottom) {
                validHolePosition = false;
                break;
            }
        }
        maxAttempts--;
    } while (!validHolePosition && maxAttempts > 0);
    
    // If we couldn't find a valid position, use the template position without randomization
    if (!validHolePosition) {
        holeX = MIN_LEVEL_WIDTH + (templ.holeX * levelWidth);
        holeY = MIN_LEVEL_HEIGHT + (templ.holeY * levelHeight);
    }

    // Keep start position consistent for fairness
    float startX = MIN_LEVEL_WIDTH + (templ.startX * levelWidth);
    float startY = MIN_LEVEL_HEIGHT + (templ.startY * levelHeight);

    // Create and set the hole
    auto hole = std::make_unique<Hole>(startX, startY, holeX, holeY, level->GetPar());
    m_hole = hole.get();
    level->SetHole(std::move(hole));

    // Create and set the ball
    level->SetBall(std::make_unique<Ball>(startX, startY));

    // Create walls based on template with random variations
    for (const auto& wallTemplate : templ.walls) {
        // Add random variation to position (±10% of level size)
        float randomOffsetX = GetRandomFloat(-levelWidth * 0.1f, levelWidth * 0.1f);
        float randomOffsetY = GetRandomFloat(-levelHeight * 0.1f, levelHeight * 0.1f);
        
        float x = MIN_LEVEL_WIDTH + (wallTemplate.relativeX * levelWidth + randomOffsetX);
        float y = MIN_LEVEL_HEIGHT + (wallTemplate.relativeY * levelHeight + randomOffsetY);
        
        // Randomize wall dimensions slightly (±20% of original size)
        float width = wallTemplate.width * GetRandomFloat(0.8f, 1.2f);
        float height = wallTemplate.height * GetRandomFloat(0.8f, 1.2f);
        
        // Ensure the randomized position is valid
        if (IsPositionValid(x, y, width/2, level->GetObjects())) {
            auto wall = std::make_unique<Wall>(x, y, width, height);
            level->AddObject(std::move(wall));
        }
    }

    // Create collectibles from template
    for (const auto& collectiblePos : templ.collectibles) {
        float x = MIN_LEVEL_WIDTH + (collectiblePos.first * levelWidth);
        float y = MIN_LEVEL_HEIGHT + (collectiblePos.second * levelHeight);
        
        if (IsPositionValid(x, y, 15.0f, level->GetObjects())) {
            auto collectible = std::make_unique<Collectible>(x, y);
            level->AddObject(std::move(collectible));
        }
    }

    // Create enemies from template
    for (const auto& enemyPos : templ.enemies) {
        float x = MIN_LEVEL_WIDTH + (enemyPos.first * levelWidth);
        float y = MIN_LEVEL_HEIGHT + (enemyPos.second * levelHeight);
        
        if (IsPositionValid(x, y, 20.0f, level->GetObjects())) {
            float patternChoice = GetRandomFloat(0.0f, 1.0f);
            std::unique_ptr<Enemy> enemy;
            
            if (patternChoice < 0.5f) {
                enemy = std::make_unique<Enemy>(x, y, Enemy::Pattern::Circular);
                enemy->SetPatrolRadius(GetRandomFloat(30.0f, 80.0f));
            }
            else {
                enemy = std::make_unique<Enemy>(x, y, Enemy::Pattern::Stationary);
            }
            
            enemy->SetSpeed(GetRandomFloat(50.0f, 150.0f));
            level->AddObject(std::move(enemy));
        }
    }

    // Add some random additional obstacles (25% chance per template wall)
    for (const auto& wallTemplate : templ.walls) {
        if (GetRandomFloat(0.0f, 1.0f) < 0.25f) {
            float x = MIN_LEVEL_WIDTH + GetRandomFloat(0.2f, 0.8f) * levelWidth;
            float y = MIN_LEVEL_HEIGHT + GetRandomFloat(0.2f, 0.8f) * levelHeight;
            float width = GetRandomFloat(50.0f, 150.0f);
            float height = 20.0f;
            
            if (IsPositionValid(x, y, width/2, level->GetObjects())) {
                auto wall = std::make_unique<Wall>(x, y, width, height);
                level->AddObject(std::move(wall));
            }
        }
    }
}

std::unique_ptr<Level> LevelGenerator::GenerateLevel(int levelNumber) {
    auto level = std::make_unique<Level>(3 + (levelNumber / 3));
    
    // Select template based on level number
    int templateIndex = levelNumber % m_courseTemplates.size();
    ApplyCourseTemplate(level.get(), m_courseTemplates[templateIndex]);

    // Add some random obstacles and collectibles for variety
    // [Previous random object generation code remains the same]
    
    return level;
}
