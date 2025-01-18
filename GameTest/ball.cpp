#include "stdafx.h"
#include "ball.h"
#include "Wall.h"
#include <cmath>

Ball::Ball(float x, float y) : 
    GameObject(x, y),
    m_prevPosX(x),
    m_prevPosY(y),
    m_velocityX(0.0f),
    m_velocityY(0.0f),
    m_accelerationX(0.0f),
    m_accelerationY(0.0f),
    m_friction(0.075f),
    m_isMoving(false),
    m_radius(6.0f),
    m_mass(45.0f),
    m_accumulator(0.0f)
{
    m_width = m_radius * 2;
    m_height = m_radius * 2;
}

void Ball::Update(float deltaTime) {
    if (!m_isMoving) return;

    const float FIXED_TIMESTEP = 1.0f / 240.0f;
    
    m_accumulator += deltaTime;
    
    while (m_accumulator >= FIXED_TIMESTEP) {
        m_prevPosX = m_posX;
        m_prevPosY = m_posY;

        float speed = sqrt(m_velocityX * m_velocityX + m_velocityY * m_velocityY);
        if (speed > 0.0f) {
            float frictionMagnitude = m_friction * speed * speed * 0.05f;
            float frictionX = -(m_velocityX / speed) * frictionMagnitude;
            float frictionY = -(m_velocityY / speed) * frictionMagnitude;
            
            m_accelerationX = frictionX;
            m_accelerationY = frictionY;
        }

        m_velocityX += m_accelerationX * FIXED_TIMESTEP;
        m_velocityY += m_accelerationY * FIXED_TIMESTEP;
        m_posX += m_velocityX * FIXED_TIMESTEP;
        m_posY += m_velocityY * FIXED_TIMESTEP;

        HandleBoundaryCollisions();

        float speedSquared = m_velocityX * m_velocityX + m_velocityY * m_velocityY;
        if (speedSquared < 0.01f) {
            Stop();
        }

        m_accumulator -= FIXED_TIMESTEP;
    }
}

void Ball::Draw() {
    float alpha = m_accumulator / (1.0f / 240.0f);
    float renderX = m_prevPosX + (m_posX - m_prevPosX) * alpha;
    float renderY = m_prevPosY + (m_posY - m_prevPosY) * alpha;
    
    DrawCircle(renderX, renderY, m_radius, 1.0f, 1.0f, 1.0f, 1.0f);
}

bool Ball::CheckCollision(const GameObject& other) {
    float otherX, otherY;
    other.GetPosition(otherX, otherY);
    
    float dx = m_posX - otherX;
    float dy = m_posY - otherY;
    float distanceSquared = dx * dx + dy * dy;
    
    return distanceSquared < (m_radius * m_radius);
}

void Ball::ApplyForce(float power, float angle) {
    const float MAX_DRAG_LENGTH = 100.0f;
    const float POWER_SCALE = 1.0f;
    
    // Normalize the power based on drag length
    float normalizedPower = (power / MAX_DRAG_LENGTH) * POWER_SCALE;
    
    // Apply velocity directly based on angle and power
    m_velocityX = normalizedPower * cos(angle);
    m_velocityY = normalizedPower * sin(angle);
    m_isMoving = true;
}

void Ball::HandleCollision(Ball& other) {
    float dx = other.m_posX - m_posX;
    float dy = other.m_posY - m_posY;
    float distance = sqrtf(dx * dx + dy * dy);
    
    if (distance < (m_radius + other.m_radius)) {
        float nx = dx / distance;
        float ny = dy / distance;
        
        float rvx = other.m_velocityX - m_velocityX;
        float rvy = other.m_velocityY - m_velocityY;
        
        float velAlongNormal = rvx * nx + rvy * ny;
        
        if (velAlongNormal > 0) return;
        
        float restitution = 0.8f;
        
        float j = -(1.0f + restitution) * velAlongNormal;
        j /= 1.0f / m_mass + 1.0f / other.m_mass;
        
        float impulseX = j * nx;
        float impulseY = j * ny;
        
        m_velocityX -= impulseX / m_mass;
        m_velocityY -= impulseY / m_mass;
        other.m_velocityX += impulseX / other.m_mass;
        other.m_velocityY += impulseY / other.m_mass;
        
        float overlap = (m_radius + other.m_radius) - distance;
        float moveX = (overlap * nx) * 0.5f;
        float moveY = (overlap * ny) * 0.5f;
        
        m_posX -= moveX;
        m_posY -= moveY;
        other.m_posX += moveX;
        other.m_posY += moveY;
        
        m_isMoving = true;
        other.m_isMoving = true;
    }
}

void Ball::DrawCircle(float x, float y, float radius, float r, float g, float b, float a) {
    const int NUM_SEGMENTS = 30;
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        float theta1 = 2.0f * 3.1415926f * float(i) / float(NUM_SEGMENTS);
        float theta2 = 2.0f * 3.1415926f * float(i + 1) / float(NUM_SEGMENTS);

        float x1 = x + radius * cosf(theta1);
        float y1 = y + radius * sinf(theta1);
        float x2 = x + radius * cosf(theta2);
        float y2 = y + radius * sinf(theta2);

        App::DrawLine(x1, y1, x2, y2, r, g, b);
    }
}

void Ball::Stop() {
    m_isMoving = false;
    m_velocityX = 0.0f;
    m_velocityY = 0.0f;
    m_accelerationX = 0.0f;
    m_accelerationY = 0.0f;
}

void Ball::SetVelocity(float vx, float vy) {
    m_velocityX = vx;
    m_velocityY = vy;
    m_isMoving = true;
}

void Ball::GetPosition(float& x, float& y) const {
    x = m_posX;
    y = m_posY;
}

void Ball::SetPosition(float x, float y) {
    m_posX = x;
    m_posY = y;
}

bool Ball::IsPointInside(float px, float py) {
    float dx = px - m_posX;
    float dy = py - m_posY;
    return (dx * dx + dy * dy) <= (m_radius * m_radius);
}

void Ball::GetInterpolatedPosition(float alpha, float& x, float& y) const {
    x = m_prevPosX + (m_posX - m_prevPosX) * alpha;
    y = m_prevPosY + (m_posY - m_prevPosY) * alpha;
}

void Ball::HandleBoundaryCollisions() {
    const float BOUNCE_DAMPENING = 0.95f;
    
    if (m_posX < m_radius) {
        m_posX = m_radius;
        m_velocityX = fabs(m_velocityX) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
    if (m_posX > SCREEN_WIDTH - m_radius) {
        m_posX = SCREEN_WIDTH - m_radius;
        m_velocityX = -fabs(m_velocityX) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
    if (m_posY < m_radius) {
        m_posY = m_radius;
        m_velocityY = fabs(m_velocityY) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
    if (m_posY > SCREEN_HEIGHT - m_radius) {
        m_posY = SCREEN_HEIGHT - m_radius;
        m_velocityY = -fabs(m_velocityY) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
}

void Ball::HandleWallCollision(const Wall& wall) {
    float wallX, wallY;
    wall.GetPosition(wallX, wallY);
    float wallWidth = wall.GetWidth();
    float wallHeight = wall.GetHeight();
    
    // Calculate wall boundaries
    float wallLeft = wallX - wallWidth/2;
    float wallRight = wallX + wallWidth/2;
    float wallTop = wallY - wallHeight/2;
    float wallBottom = wallY + wallHeight/2;
    
    const float BOUNCE_DAMPENING = 0.95f;
    
    // Determine which side was hit and bounce accordingly
    if (m_posX < wallLeft) {
        m_posX = wallLeft - m_radius;
        m_velocityX = -fabs(m_velocityX) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
    if (m_posX > wallRight) {
        m_posX = wallRight + m_radius;
        m_velocityX = fabs(m_velocityX) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
    if (m_posY < wallTop) {
        m_posY = wallTop - m_radius;
        m_velocityY = -fabs(m_velocityY) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
    if (m_posY > wallBottom) {
        m_posY = wallBottom + m_radius;
        m_velocityY = fabs(m_velocityY) * BOUNCE_DAMPENING;
        m_isMoving = true;
    }
}