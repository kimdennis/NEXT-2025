#include "stdafx.h"
#include "ball.h"
#include "Wall.h"
#include <cmath>
#include <limits>

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

    const float FIXED_TIMESTEP = 1.0f / 60.0f;
    const float MAX_VELOCITY = 25.0f;
    
    m_accumulator += deltaTime;
    
    while (m_accumulator >= FIXED_TIMESTEP) {
        m_prevPosX = m_posX;
        m_prevPosY = m_posY;
        
        // Apply velocity caps
        float speed = sqrt(m_velocityX * m_velocityX + m_velocityY * m_velocityY);
        if (speed > MAX_VELOCITY) {
            float scale = MAX_VELOCITY / speed;
            m_velocityX *= scale;
            m_velocityY *= scale;
        }
        
        // Apply friction
        if (speed > 0.0f) {
            float frictionMagnitude = m_friction * speed * speed * 0.05f;
            float frictionX = -(m_velocityX / speed) * frictionMagnitude;
            float frictionY = -(m_velocityY / speed) * frictionMagnitude;
            
            m_accelerationX = frictionX;
            m_accelerationY = frictionY;
        }
        
        // Update velocity and position
        m_velocityX += m_accelerationX * FIXED_TIMESTEP;
        m_velocityY += m_accelerationY * FIXED_TIMESTEP;
        
        // Update position
        m_posX += m_velocityX * FIXED_TIMESTEP;
        m_posY += m_velocityY * FIXED_TIMESTEP;
        
        // Handle screen boundary collisions
        HandleBoundaryCollisions();
        
        // Check for stopping condition
        float speedSquared = m_velocityX * m_velocityX + m_velocityY * m_velocityY;
        if (speedSquared < 0.01f) {
            Stop();
        }
        
        m_accumulator -= FIXED_TIMESTEP;
    }
}

void Ball::Draw() {
    // Draw the ball
    DrawCircle(m_posX, m_posY, m_radius, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // Display velocity information
    char velocityText[64];
    float totalVelocity = sqrt(m_velocityX * m_velocityX + m_velocityY * m_velocityY);
    sprintf_s(velocityText, "Velocity: %.1f", totalVelocity);
    
    // Position the text above the ball
    App::Print(static_cast<int>(m_posX - 30), static_cast<int>(m_posY - m_radius - 20), velocityText);
    
    // Optionally show X and Y components
    char componentText[64];
    sprintf_s(componentText, "X: %.1f Y: %.1f", m_velocityX, m_velocityY);
    App::Print(static_cast<int>(m_posX - 30), static_cast<int>(m_posY - m_radius - 35), componentText);
}

bool Ball::CheckCollision(const GameObject& other) {
    // Check for wall collision
    if (const Wall* wall = dynamic_cast<const Wall*>(&other)) {
        HandleWallCollision(*wall);
        return true;
    }
    
    // Check for enemy collision
    if (Enemy* enemy = const_cast<Enemy*>(dynamic_cast<const Enemy*>(&other))) {  // Note: const_cast to allow Kill()
        if (enemy->IsAlive() && !enemy->IsExploding()) {
            float enemyX, enemyY;
            enemy->GetPosition(enemyX, enemyY);
            
            float dx = m_posX - enemyX;
            float dy = m_posY - enemyY;
            float distanceSquared = dx * dx + dy * dy;
            float minDistance = m_radius + enemy->GetSize();
            
            if (distanceSquared < minDistance * minDistance) {
                // Kill the enemy
                enemy->Kill();
                
                // Normalize direction vector
                float distance = sqrt(distanceSquared);
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Set new velocity in the direction away from enemy
                const float BOUNCE_SPEED = 15.0f;
                m_velocityX = nx * BOUNCE_SPEED;
                m_velocityY = ny * BOUNCE_SPEED;
                
                // Move ball out of collision
                m_posX = enemyX + nx * minDistance;
                m_posY = enemyY + ny * minDistance;
                
                m_isMoving = true;
                return true;
            }
        }
    }
    
    // Add collectible collision check
    if (Collectible* collectible = const_cast<Collectible*>(dynamic_cast<const Collectible*>(&other))) {
        if (!collectible->IsCollected()) {
            float collectibleX, collectibleY;
            collectible->GetPosition(collectibleX, collectibleY);
            
            float dx = m_posX - collectibleX;
            float dy = m_posY - collectibleY;
            float distanceSquared = dx * dx + dy * dy;
            float minDistance = m_radius + 8.0f; // 8.0f is the collectible's radius
            
            if (distanceSquared < minDistance * minDistance) {
                collectible->Collect();
                return true;
            }
        }
    }
    
    return false;
}

void Ball::ApplyForce(float power, float angle) {
    const float MAX_DRAG_LENGTH = 100.0f;
    const float POWER_SCALE = 2.0f;
    
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

        // Cap velocities after collision
        const float MAX_COLLISION_VELOCITY = 15.0f;
        float speed = sqrt(m_velocityX * m_velocityX + m_velocityY * m_velocityY);
        if (speed > MAX_COLLISION_VELOCITY) {
            float scale = MAX_COLLISION_VELOCITY / speed;
            m_velocityX *= scale;
            m_velocityY *= scale;
        }
        
        speed = sqrt(other.m_velocityX * other.m_velocityX + other.m_velocityY * other.m_velocityY);
        if (speed > MAX_COLLISION_VELOCITY) {
            float scale = MAX_COLLISION_VELOCITY / speed;
            other.m_velocityX *= scale;
            other.m_velocityY *= scale;
        }
        
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
    float wallLeft = wall.GetPosition().x - wall.GetWidth()/2;
    float wallRight = wall.GetPosition().x + wall.GetWidth()/2;
    float wallTop = wall.GetPosition().y - wall.GetHeight()/2;
    float wallBottom = wall.GetPosition().y + wall.GetHeight()/2;

    // Check if ball overlaps with wall
    bool collision = false;
    float penetrationX = 0.0f;
    float penetrationY = 0.0f;

    // Check horizontal overlap
    if (m_posX + m_radius > wallLeft && m_posX - m_radius < wallRight) {
        // Check vertical overlap
        if (m_posY + m_radius > wallTop && m_posY - m_radius < wallBottom) {
            collision = true;

            // Calculate penetration depths
            float rightPen = m_posX + m_radius - wallLeft;
            float leftPen = wallRight - (m_posX - m_radius);
            float bottomPen = m_posY + m_radius - wallTop;
            float topPen = wallBottom - (m_posY - m_radius);

            // Find smallest penetration
            float minPenX = (rightPen < leftPen) ? -rightPen : leftPen;
            float minPenY = (bottomPen < topPen) ? -bottomPen : topPen;

            // Determine which axis to resolve on based on minimum penetration
            if (abs(minPenX) < abs(minPenY)) {
                penetrationX = minPenX;
                m_velocityX = -m_velocityX * BOUNCE_DAMPENING;
            } else {
                penetrationY = minPenY;
                m_velocityY = -m_velocityY * BOUNCE_DAMPENING;
            }

            // Move ball out of wall
            m_posX += penetrationX;
            m_posY += penetrationY;
            m_isMoving = true;
        }
    }
}