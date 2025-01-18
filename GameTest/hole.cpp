#include "stdafx.h"
#include "hole.h"
#include <cmath>

Hole::Hole(float startX, float startY, float holeX, float holeY, int par) :
    GameObject(holeX, holeY),
    m_startX(startX),
    m_startY(startY),
    m_holeX(holeX),
    m_holeY(holeY),
    m_par(par)
{
    m_width = 20.0f;
    m_height = 20.0f;
}

void Hole::Update(float deltaTime) {
    // Holes don't need updating in current implementation
    // But we could add animations or effects here
}

void Hole::Draw() {
    // Draw the level borders
    App::DrawLine(0, 0, SCREEN_WIDTH, 0, 0.5f, 0.5f, 0.5f);
    App::DrawLine(SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.5f, 0.5f);
    App::DrawLine(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SCREEN_HEIGHT, 0.5f, 0.5f, 0.5f);
    App::DrawLine(0, SCREEN_HEIGHT, 0, 0, 0.5f, 0.5f, 0.5f);

    // Draw the hole (target)
    App::DrawLine(m_holeX - 5, m_holeY - 5, m_holeX + 5, m_holeY + 5, 1.0f, 1.0f, 1.0f);
    App::DrawLine(m_holeX - 5, m_holeY + 5, m_holeX + 5, m_holeY - 5, 1.0f, 1.0f, 1.0f);
    
    // Draw obstacles
    for (const auto& obstacle : m_obstacles) {
        App::DrawLine(obstacle.x - obstacle.width/2, obstacle.y - obstacle.height/2,
                     obstacle.x + obstacle.width/2, obstacle.y - obstacle.height/2, 0.0f, 1.0f, 0.0f);
        App::DrawLine(obstacle.x + obstacle.width/2, obstacle.y - obstacle.height/2,
                     obstacle.x + obstacle.width/2, obstacle.y + obstacle.height/2, 0.0f, 1.0f, 0.0f);
        App::DrawLine(obstacle.x + obstacle.width/2, obstacle.y + obstacle.height/2,
                     obstacle.x - obstacle.width/2, obstacle.y + obstacle.height/2, 0.0f, 1.0f, 0.0f);
        App::DrawLine(obstacle.x - obstacle.width/2, obstacle.y + obstacle.height/2,
                     obstacle.x - obstacle.width/2, obstacle.y - obstacle.height/2, 0.0f, 1.0f, 0.0f);
    }
}

bool Hole::CheckCollision(const GameObject& other) {
    float otherX, otherY;
    other.GetPosition(otherX, otherY);
    return IsInHole(otherX, otherY);
}

bool Hole::IsInHole(float x, float y) const {
    float dx = x - m_holeX;
    float dy = y - m_holeY;
    float distanceSquared = dx * dx + dy * dy;
    return distanceSquared < (10.0f * 10.0f);  // 10.0f is the hole radius
}

void Hole::AddObstacle(float x, float y, float width, float height) {
    Obstacle obstacle = {x, y, width, height};
    m_obstacles.push_back(obstacle);
}

void Hole::GetStartPosition(float& x, float& y) const {
    x = m_startX;
    y = m_startY;
}
