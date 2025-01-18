#pragma once
#include <vector>
#include "App/app.h"

// Declare external constants
extern const float SCREEN_WIDTH;
extern const float SCREEN_HEIGHT;

struct Obstacle {
    float x, y;
    float width, height;
};

class Hole {
private:
    float m_startX, m_startY;      // Starting position
    float m_holeX, m_holeY;        // Hole/target position
    std::vector<Obstacle> m_obstacles;
    int m_par;

public:
    Hole(float startX, float startY, float holeX, float holeY, int par);
    
    void Draw();
    bool IsInHole(float x, float y) const;
    void AddObstacle(float x, float y, float width, float height);
    bool CheckCollision(float x, float y) const;
    
    // Getters
    void GetStartPosition(float& x, float& y) const;
    int GetPar() const { return m_par; }
};
