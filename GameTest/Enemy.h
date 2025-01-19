#pragma once
#include "GameObject.h"
#include "App/app.h"
#include <cmath>

class Enemy : public GameObject {
public:
    enum class Pattern {
        Circular,
        Linear,
        Stationary
    };

private:
    Pattern m_pattern;
    float m_speed;
    float m_patrolRadius;
    float m_patrolDistance;
    float m_angle;
    float m_startX;
    float m_startY;
    float m_time;
    float m_size;
    bool m_isAlive;
    bool m_isExploding;
    float m_explosionTime;
    static constexpr float EXPLOSION_DURATION = 0.5f;

public:
    Enemy(float x, float y, Pattern pattern = Pattern::Circular) 
        : GameObject(x, y)
        , m_pattern(pattern)
        , m_speed(100.0f)
        , m_patrolRadius(50.0f)
        , m_patrolDistance(100.0f)
        , m_angle(0.0f)
        , m_startX(x)
        , m_startY(y)
        , m_time(0.0f)
        , m_size(10.0f)
        , m_isAlive(true)
        , m_isExploding(false)
        , m_explosionTime(0.0f)
    {
        m_width = 20.0f;
        m_height = 20.0f;
    }

    void Update(float deltaTime) override {
        if (m_isExploding) {
            m_explosionTime += deltaTime;
            if (m_explosionTime >= EXPLOSION_DURATION) {
                m_isAlive = false;
            }
            return;
        }

        if (!m_isAlive) return;

        m_time += deltaTime;
        
        switch (m_pattern) {
            case Pattern::Circular:
                m_angle += m_speed * deltaTime;
                m_posX = m_startX + cos(m_angle * 0.05f) * m_patrolRadius;
                m_posY = m_startY + sin(m_angle * 0.05f) * m_patrolRadius;
                break;

            case Pattern::Linear:
                m_posX = m_startX + cos(m_time * m_speed * 0.05f) * m_patrolDistance;
                break;

            case Pattern::Stationary:
                m_angle += m_speed * deltaTime;
                break;
        }
    }

    void Draw() override {
        if (!m_isAlive && !m_isExploding) return;

        if (m_isExploding) {
            // Draw explosion effect
            float explosionSize = m_size * (1.0f + m_explosionTime / EXPLOSION_DURATION * 2.0f);
            float alpha = 1.0f - (m_explosionTime / EXPLOSION_DURATION);
            
            for (float angle = 0; angle < 360; angle += 45) {
                float rad = angle * 3.14159f / 180.0f;
                float x2 = m_posX + cos(rad) * explosionSize;
                float y2 = m_posY + sin(rad) * explosionSize;
                App::DrawLine(m_posX, m_posY, x2, y2, 1.0f * alpha, 0.5f * alpha, 0.0f);
            }
            return;
        }

        // Draw enemy as a red triangle
        float angle = m_angle * 3.14159f / 180.0f;
        
        float x1 = m_posX + m_size * cos(angle);
        float y1 = m_posY + m_size * sin(angle);
        float x2 = m_posX + m_size * cos(angle + 2.0944f); // +120 degrees
        float y2 = m_posY + m_size * sin(angle + 2.0944f);
        float x3 = m_posX + m_size * cos(angle + 4.1888f); // +240 degrees
        float y3 = m_posY + m_size * sin(angle + 4.1888f);

        App::DrawLine(x1, y1, x2, y2, 1.0f, 0.0f, 0.0f);
        App::DrawLine(x2, y2, x3, y3, 1.0f, 0.0f, 0.0f);
        App::DrawLine(x3, y3, x1, y1, 1.0f, 0.0f, 0.0f);
    }

    bool CheckCollision(const GameObject& other) override {
        if (!m_isAlive || m_isExploding) return false;

        float otherX, otherY;
        other.GetPosition(otherX, otherY);
        
        float dx = m_posX - otherX;
        float dy = m_posY - otherY;
        float distanceSquared = dx * dx + dy * dy;
        
        return distanceSquared < (m_width * m_width);
    }

    void Kill() {
        if (m_isAlive && !m_isExploding) {
            m_isExploding = true;
            m_explosionTime = 0.0f;
        }
    }

    // Getters and Setters
    void SetSpeed(float speed) { m_speed = speed; }
    void SetPatrolRadius(float radius) { m_patrolRadius = radius; }
    void SetPatrolDistance(float distance) { m_patrolDistance = distance; }
    float GetSize() const { return m_size; }
    bool IsAlive() const { return m_isAlive; }
    bool IsExploding() const { return m_isExploding; }
};
