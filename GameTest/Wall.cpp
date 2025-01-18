#include "stdafx.h"
#include "Wall.h"
#include "Ball.h"

bool Wall::CheckCollision(const GameObject& other) {
    // Check if the other object is a ball
    const Ball* ball = dynamic_cast<const Ball*>(&other);
    if (!ball) return false;
    
    float ballX, ballY;
    other.GetPosition(ballX, ballY);
    float radius = ball->GetRadius();
    
    // Calculate wall boundaries
    float wallLeft = m_posX - m_width/2;
    float wallRight = m_posX + m_width/2;
    float wallTop = m_posY - m_height/2;
    float wallBottom = m_posY + m_height/2;
    
    // Check if ball overlaps with wall boundaries
    if (ballX + radius > wallLeft && ballX - radius < wallRight &&
        ballY + radius > wallTop && ballY - radius < wallBottom) {
        return true;
    }
    
    return false;
}
