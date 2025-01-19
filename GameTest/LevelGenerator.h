#pragma once
#include <memory>
#include "Level.h"
#include "Hole.h"

class LevelGenerator {
private:
    static constexpr float MIN_LEVEL_WIDTH = 150.0f;
    static constexpr float MAX_LEVEL_WIDTH = 850.0f;
    static constexpr float MIN_LEVEL_HEIGHT = 150.0f;
    static constexpr float MAX_LEVEL_HEIGHT = 450.0f;
    static constexpr float EDGE_MARGIN = 100.0f;
    static constexpr float HOLE_CLEAR_RADIUS = 60.0f;

    Hole* m_hole;
    
    bool IsTooCloseToHole(float x, float y, float minDistance);

public:
    LevelGenerator() : m_hole(nullptr) {}
    std::unique_ptr<Level> GenerateLevel(int levelNumber);
    void SetHole(Hole* hole) { m_hole = hole; }
    
    bool IsPositionValid(float x, float y, float radius, const std::vector<std::unique_ptr<GameObject>>& existingObjects);
    float GetRandomFloat(float min, float max);
};
