#pragma once
#include <memory>
#include <random>
#include "Level.h"
#include "Ball.h"
#include "Hole.h"
#include "Wall.h"
#include "Enemy.h"
#include "Collectible.h"

class LevelGenerator {
private:
    std::mt19937 m_rng;  // Random number generator
    
    // Level constraints
    const float MIN_LEVEL_WIDTH = 600.0f;
    const float MAX_LEVEL_WIDTH = 800.0f;
    const float MIN_LEVEL_HEIGHT = 400.0f;
    const float MAX_LEVEL_HEIGHT = 600.0f;
    
    // Spacing constraints
    const float MIN_OBJECT_SPACING = 50.0f;
    const float EDGE_MARGIN = 30.0f;
    
    // Generation helpers
    float GetRandomFloat(float min, float max);
    bool IsPositionValid(float x, float y, float radius, const std::vector<std::unique_ptr<GameObject>>& objects);
    void AddRandomWalls(std::unique_ptr<Level>& level, int difficulty);
    void AddRandomEnemies(std::unique_ptr<Level>& level, int difficulty);
    void AddRandomCollectibles(std::unique_ptr<Level>& level, int difficulty);

public:
    LevelGenerator() : m_rng(std::random_device{}()) {}
    
    std::unique_ptr<Level> GenerateLevel(int difficulty);
};
