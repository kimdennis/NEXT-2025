#pragma once
#include "GameObject.h"
#include "App/app.h"
#include "GameEventManager.h"

class Collectible : public GameObject {
private:
    bool m_isCollected;
    float m_radius;

public:
    Collectible(float x, float y) 
        : GameObject(x, y), m_isCollected(false), m_radius(8.0f) {}
    
    void Update(float deltaTime) override {}
    
    void Draw() override {
        if (!m_isCollected) {
            // Draw yellow circle
            for (float angle = 0; angle < 360; angle += 30) {
                float x1 = m_posX + m_radius * cosf(angle * 3.14f / 180.0f);
                float y1 = m_posY + m_radius * sinf(angle * 3.14f / 180.0f);
                float x2 = m_posX + m_radius * cosf((angle + 30) * 3.14f / 180.0f);
                float y2 = m_posY + m_radius * sinf((angle + 30) * 3.14f / 180.0f);
                App::DrawLine(x1, y1, x2, y2, 1.0f, 1.0f, 0.0f);
            }
        }
    }
    
    bool CheckCollision(const GameObject& other) override { return false; }
    
    bool IsCollected() const { return m_isCollected; }
    void Collect() { 
        m_isCollected = true; 
        GameEventManager::GetInstance().Emit(GameEventManager::EventType::CollectibleCollected);
    }
};
