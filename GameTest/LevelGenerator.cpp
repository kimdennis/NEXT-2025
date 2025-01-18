#include "stdafx.h"
#include "LevelGenerator.h"
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

std::unique_ptr<Level> LevelGenerator::GenerateLevel(int levelNumber) {
    const float BOUNDARY_MARGIN = 100.0f;
    const float CLEAR_RADIUS = HOLE_CLEAR_RADIUS;
    
    // Calculate difficulty based on level number (1-10 scale)
    int difficulty = (levelNumber + 2) / 2;
    if (difficulty > 10) difficulty = 10;
    
    // Calculate level dimensions
    float levelWidth = MAX_LEVEL_WIDTH - MIN_LEVEL_WIDTH;
    float levelHeight = MAX_LEVEL_HEIGHT - MIN_LEVEL_HEIGHT;
    
    // Calculate safe spawn area
    float safeMinX = MIN_LEVEL_WIDTH + BOUNDARY_MARGIN;
    float safeMaxX = MAX_LEVEL_WIDTH - BOUNDARY_MARGIN;
    float safeMinY = MIN_LEVEL_HEIGHT + BOUNDARY_MARGIN;
    float safeMaxY = MAX_LEVEL_HEIGHT - BOUNDARY_MARGIN;
    
    // Create level with appropriate par based on level number
    int par = 3 + (levelNumber / 3);  // Increase par every 3 levels
    auto level = std::make_unique<Level>(par);
    
    // Generate start and hole positions
    float startY = GetRandomFloat(safeMinY, safeMaxY);
    float holeY = GetRandomFloat(safeMinY, safeMaxY);
    float startX = safeMinX;
    float holeX = safeMaxX - BOUNDARY_MARGIN;
    
    // Create hole position check lambda after hole position is defined
    auto isTooCloseToHole = [holeX, holeY, CLEAR_RADIUS](float x, float y, float objectRadius) {
        float dx = x - holeX;
        float dy = y - holeY;
        return (dx * dx + dy * dy) < (CLEAR_RADIUS * CLEAR_RADIUS);
    };
    
    // Create and set the hole
    auto hole = std::make_unique<Hole>(startX, startY, holeX, holeY, par);
    m_hole = hole.get();
    level->SetHole(std::move(hole));
    
    // Create and set the ball
    level->SetBall(std::make_unique<Ball>(startX, startY));
    
    // Add walls with hole check
    int wallAttempts = difficulty + 2;
    while (wallAttempts > 0) {
        float x = GetRandomFloat(EDGE_MARGIN, levelWidth - EDGE_MARGIN);
        float y = GetRandomFloat(EDGE_MARGIN, levelHeight - EDGE_MARGIN);
        float width = GetRandomFloat(50.0f, 150.0f);
        float height = GetRandomFloat(20.0f, 100.0f);
        float radius = std::sqrt(width*width + height*height) * 0.5f;
        
        if (!isTooCloseToHole(x, y, radius) && IsPositionValid(x, y, radius, level->GetObjects())) {
            level->AddObject(std::make_unique<Wall>(x, y, width, height));
        }
        wallAttempts--;
    }
    
    // Add enemies with hole check
    int enemyAttempts = difficulty;
    while (enemyAttempts > 0) {
        float x = GetRandomFloat(EDGE_MARGIN, levelWidth - EDGE_MARGIN);
        float y = GetRandomFloat(EDGE_MARGIN, levelHeight - EDGE_MARGIN);
        
        if (!isTooCloseToHole(x, y, 15.0f) && IsPositionValid(x, y, 15.0f, level->GetObjects())) {
            level->AddObject(std::make_unique<Enemy>(x, y));
        }
        enemyAttempts--;
    }
    
    // Add collectibles with hole check
    int collectibleAttempts = difficulty + 2;
    while (collectibleAttempts > 0) {
        float x = GetRandomFloat(EDGE_MARGIN, levelWidth - EDGE_MARGIN);
        float y = GetRandomFloat(EDGE_MARGIN, levelHeight - EDGE_MARGIN);
        
        if (!isTooCloseToHole(x, y, 8.0f) && IsPositionValid(x, y, 8.0f, level->GetObjects())) {
            level->AddObject(std::make_unique<Collectible>(x, y));
        }
        collectibleAttempts--;
    }
    
    return level;
}
