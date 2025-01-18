#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include <vector>
#include "App/app.h"

// Declare external constants
extern const float SCREEN_WIDTH;
extern const float SCREEN_HEIGHT;

struct Obstacle {
    float x, y;
    float width, height;
};

class Hole : public GameObject {
private:
    float m_startX, m_startY;      // Starting position
    float m_holeX, m_holeY;        // Hole/target position
    std::vector<Obstacle> m_obstacles;
    int m_par;

public:
    Hole(float startX, float startY, float holeX, float holeY, int par);
    ~Hole() override = default;
    
    // GameObject interface implementation
    void Update(float deltaTime) override;
    void Draw() override;
    bool CheckCollision(const GameObject& other) override;
    
    // Hole-specific methods
    bool IsInHole(float x, float y) const;
    void AddObstacle(float x, float y, float width, float height);
    bool CheckCollision(float x, float y) const;
    
    // Getters
    void GetStartPosition(float& x, float& y) const;
    int GetPar() const { return m_par; }
};
