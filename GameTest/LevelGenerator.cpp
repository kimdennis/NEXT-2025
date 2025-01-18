#include "stdafx.h"
#include "LevelGenerator.h"

float LevelGenerator::GetRandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_rng);
}

bool LevelGenerator::IsPositionValid(float x, float y, float radius, 
    const std::vector<std::unique_ptr<GameObject>>& objects) {
    // Check if position is too close to other objects
    for (const auto& obj : objects) {
        float objX, objY;
        obj->GetPosition(objX, objY);
        float dx = x - objX;
        float dy = y - objY;
        float minDist = radius + MIN_OBJECT_SPACING;
        if (dx * dx + dy * dy < minDist * minDist) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Level> LevelGenerator::GenerateLevel(int difficulty) {
    auto level = std::make_unique<Level>(3 + difficulty);
    
    // Generate level width and height
    float levelWidth = GetRandomFloat(MIN_LEVEL_WIDTH, MAX_LEVEL_WIDTH);
    float levelHeight = GetRandomFloat(MIN_LEVEL_HEIGHT, MAX_LEVEL_HEIGHT);
    
    // Place ball (start position)
    float startX = EDGE_MARGIN + GetRandomFloat(0, levelWidth * 0.2f);
    float startY = GetRandomFloat(EDGE_MARGIN, levelHeight - EDGE_MARGIN);
    level->SetBall(std::make_unique<Ball>(startX, startY));
    
    // Place hole FIRST with large clear area
    const float HOLE_CLEAR_RADIUS = 50.0f;  // Increased clear area
    float holeX = levelWidth - EDGE_MARGIN - GetRandomFloat(0, levelWidth * 0.2f);
    float holeY = GetRandomFloat(EDGE_MARGIN, levelHeight - EDGE_MARGIN);
    
    // Create hole first so other objects can check against it
    level->SetHole(std::make_unique<Hole>(startX, startY, holeX, holeY, 3 + difficulty));
    
    // Helper function to check if position is too close to hole
    auto isTooCloseToHole = [holeX, holeY, HOLE_CLEAR_RADIUS](float x, float y, float objectRadius) {
        float dx = x - holeX;
        float dy = y - holeY;
        float minDist = HOLE_CLEAR_RADIUS + objectRadius;
        return (dx * dx + dy * dy) < (minDist * minDist);
    };
    
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
