#pragma once
#include "GameObject.h"
#include "App/app.h"

class Enemy : public GameObject {
private:
    float m_size;
    bool m_isAlive;
    bool m_isExploding;
    float m_explosionTime;
    static constexpr float EXPLOSION_DURATION = 0.5f;  // Duration in seconds
    static constexpr int NUM_PARTICLES = 8;  // Number of explosion particles

public:
    Enemy(float x, float y, float size = 15.0f) 
        : GameObject(x, y), m_size(size), m_isAlive(true), 
          m_isExploding(false), m_explosionTime(0.0f) {}
    
    void Update(float deltaTime) override {
        if (m_isExploding) {
            m_explosionTime += deltaTime;
            if (m_explosionTime >= EXPLOSION_DURATION) {
                m_isExploding = false;
                m_isAlive = false;
            }
        }
    }
    
    void Draw() override {
        if (!m_isAlive && !m_isExploding) return;
        
        if (m_isExploding) {
            // Draw explosion particles
            float progress = m_explosionTime / EXPLOSION_DURATION;
            float particleSize = m_size * (1.0f - progress);
            float spread = m_size * 2.0f * progress;
            
            for (int i = 0; i < NUM_PARTICLES; i++) {
                float angle = (float)i * (2.0f * 3.14159f / NUM_PARTICLES);
                float px = m_posX + cosf(angle) * spread;
                float py = m_posY + sinf(angle) * spread;
                
                // Draw X shaped particles
                App::DrawLine(px - particleSize, py - particleSize,
                            px + particleSize, py + particleSize, 1.0f, 0.0f, 0.0f);
                App::DrawLine(px + particleSize, py - particleSize,
                            px - particleSize, py + particleSize, 1.0f, 0.0f, 0.0f);
            }
        } else {
            // Draw normal triangle
            App::DrawLine(m_posX, m_posY - m_size, 
                         m_posX + m_size, m_posY + m_size, 1.0f, 0.0f, 0.0f);
            App::DrawLine(m_posX + m_size, m_posY + m_size, 
                         m_posX - m_size, m_posY + m_size, 1.0f, 0.0f, 0.0f);
            App::DrawLine(m_posX - m_size, m_posY + m_size, 
                         m_posX, m_posY - m_size, 1.0f, 0.0f, 0.0f);
        }
    }
    
    bool CheckCollision(const GameObject& other) override;
    void Kill() { 
        if (m_isAlive) {
            m_isExploding = true;
            m_explosionTime = 0.0f;
        }
    }
    bool IsAlive() const { return m_isAlive; }
    bool IsExploding() const { return m_isExploding; }
    float GetSize() const { return m_size; }
};
