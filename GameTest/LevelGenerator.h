#pragma once
#include <memory>
#include <vector>
#include <utility>  // for std::pair
#include "Level.h"
#include "Hole.h"

struct WallTemplate {
    float relativeX;      // Position relative to level width (0.0 to 1.0)
    float relativeY;      // Position relative to level height (0.0 to 1.0)
    float width;
    float height;
    float rotation;       // Rotation in degrees
};

struct CourseTemplate {
    std::vector<WallTemplate> walls;
    std::vector<std::pair<float, float>> collectibles;  // Relative positions (x,y)
    std::vector<std::pair<float, float>> enemies;       // Relative positions (x,y)
    float startX;         // Relative start position (0.0 to 1.0)
    float startY;
    float holeX;         // Relative hole position (0.0 to 1.0)
    float holeY;
};

class LevelGenerator {
private:
    static constexpr float MIN_LEVEL_WIDTH = 150.0f;
    static constexpr float MAX_LEVEL_WIDTH = 850.0f;
    static constexpr float MIN_LEVEL_HEIGHT = 150.0f;
    static constexpr float MAX_LEVEL_HEIGHT = 450.0f;
    static constexpr float EDGE_MARGIN = 100.0f;
    static constexpr float HOLE_CLEAR_RADIUS = 60.0f;

    Hole* m_hole;
    std::vector<CourseTemplate> m_courseTemplates;
    
    CourseTemplate GetCourseTemplate(int templateIndex);
    void ApplyCourseTemplate(Level* level, const CourseTemplate& templ);
    void InitializeTemplates();
    bool IsTooCloseToHole(float x, float y, float minDistance);

public:
    LevelGenerator();
    std::unique_ptr<Level> GenerateLevel(int levelNumber);
    void SetHole(Hole* hole) { m_hole = hole; }
    
    bool IsPositionValid(float x, float y, float radius, const std::vector<std::unique_ptr<GameObject>>& existingObjects);
    float GetRandomFloat(float min, float max);
};
