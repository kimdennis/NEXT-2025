#pragma once
#include "GameObject.h"
#include "App/app.h"

class Enemy : public GameObject {
private:
    float m_size;
    bool m_isAlive;

public:
    Enemy(float x, float y, float size = 15.0f) 
        : GameObject(x, y), m_size(size), m_isAlive(true) {}
    
    void Update(float deltaTime) override {}
    
    void Draw() override {
        if (!m_isAlive) return;
        
        // Draw triangle
        App::DrawLine(m_posX, m_posY - m_size, 
                     m_posX + m_size, m_posY + m_size, 1.0f, 0.0f, 0.0f);
        App::DrawLine(m_posX + m_size, m_posY + m_size, 
                     m_posX - m_size, m_posY + m_size, 1.0f, 0.0f, 0.0f);
        App::DrawLine(m_posX - m_size, m_posY + m_size, 
                     m_posX, m_posY - m_size, 1.0f, 0.0f, 0.0f);
    }
    
    bool CheckCollision(const GameObject& other) override;
    void Kill() { m_isAlive = false; }
    bool IsAlive() const { return m_isAlive; }
    float GetSize() const { return m_size; }
};
