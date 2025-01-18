#include "stdafx.h"
#include "Wall.h"
#include "Ball.h"

void Wall::Draw() {
    App::DrawLine(m_posX - m_width/2, m_posY - m_height/2, 
                 m_posX + m_width/2, m_posY - m_height/2, 0.0f, 1.0f, 0.0f);
    App::DrawLine(m_posX + m_width/2, m_posY - m_height/2,
                 m_posX + m_width/2, m_posY + m_height/2, 0.0f, 1.0f, 0.0f);
    App::DrawLine(m_posX + m_width/2, m_posY + m_height/2,
                 m_posX - m_width/2, m_posY + m_height/2, 0.0f, 1.0f, 0.0f);
    App::DrawLine(m_posX - m_width/2, m_posY + m_height/2,
                 m_posX - m_width/2, m_posY - m_height/2, 0.0f, 1.0f, 0.0f);
}

bool Wall::CheckCollision(const GameObject& other) {
    const Ball* ball = dynamic_cast<const Ball*>(&other);
    if (!ball) return false;
    
    float ballX, ballY;
    ball->GetPosition(ballX, ballY);
    
    // Calculate wall boundaries
    float wallLeft = m_posX - m_width/2;
    float wallRight = m_posX + m_width/2;
    float wallTop = m_posY - m_height/2;
    float wallBottom = m_posY + m_height/2;
    
    // Check if ball overlaps with wall boundaries
    if (ballX + ball->GetRadius() > wallLeft && 
        ballX - ball->GetRadius() < wallRight &&
        ballY + ball->GetRadius() > wallTop && 
        ballY - ball->GetRadius() < wallBottom) {
        return true;
    }
    
    return false;
}
