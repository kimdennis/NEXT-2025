#include "stdafx.h"
#include "Enemy.h"
#include "Ball.h"

bool Enemy::CheckCollision(const GameObject& other) {
    if (!m_isAlive) return false;
    
    const Ball* ball = dynamic_cast<const Ball*>(&other);
    if (!ball) return false;
    
    float ballX, ballY;
    other.GetPosition(ballX, ballY);
    float radius = ball->GetRadius();
    
    // Simple circle-triangle collision using circle
    float dx = ballX - m_posX;
    float dy = ballY - m_posY;
    float distanceSquared = dx * dx + dy * dy;
    float minDistance = radius + m_size;
    
    return distanceSquared < minDistance * minDistance;
}
