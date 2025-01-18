#include "stdafx.h"
#include "hole.h"
#include <cmath>

Hole::Hole(float startX, float startY, float holeX, float holeY, int par) :
    m_startX(startX),
    m_startY(startY),
    m_holeX(holeX),
    m_holeY(holeY),
    m_par(par)
{
}

void Hole::Draw() {
    // Draw the level borders
    App::DrawLine(0, 0, SCREEN_WIDTH, 0, 0.5f, 0.5f, 0.5f);           // Top border
    App::DrawLine(SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.5f, 0.5f);  // Right border
    App::DrawLine(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SCREEN_HEIGHT, 0.5f, 0.5f, 0.5f); // Bottom border
    App::DrawLine(0, SCREEN_HEIGHT, 0, 0, 0.5f, 0.5f, 0.5f);         // Left border

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

bool Hole::IsInHole(float x, float y) const {
    const float HOLE_RADIUS = 10.0f;
    float dx = x - m_holeX;
    float dy = y - m_holeY;
    return (dx * dx + dy * dy) < (HOLE_RADIUS * HOLE_RADIUS);
}

void Hole::AddObstacle(float x, float y, float width, float height) {
    Obstacle obstacle = {x, y, width, height};
    m_obstacles.push_back(obstacle);
}

bool Hole::CheckCollision(float x, float y) const {
    for (const auto& obstacle : m_obstacles) {
        if (x >= obstacle.x - obstacle.width/2 && x <= obstacle.x + obstacle.width/2 &&
            y >= obstacle.y - obstacle.height/2 && y <= obstacle.y + obstacle.height/2) {
            return true;
        }
    }
    return false;
}

void Hole::GetStartPosition(float& x, float& y) const {
    x = m_startX;
    y = m_startY;
}
