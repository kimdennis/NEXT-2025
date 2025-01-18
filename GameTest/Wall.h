#pragma once
#include "GameObject.h"

class Wall : public GameObject {
private:
    float m_width;
    float m_height;

public:
    Wall(float x, float y, float width, float height) 
        : GameObject(x, y), m_width(width), m_height(height) {}
    
    void Update(float deltaTime) override {}
    void Draw() override {
        App::DrawLine(m_posX - m_width/2, m_posY - m_height/2, 
                     m_posX + m_width/2, m_posY - m_height/2, 0.0f, 1.0f, 0.0f);
        App::DrawLine(m_posX + m_width/2, m_posY - m_height/2,
                     m_posX + m_width/2, m_posY + m_height/2, 0.0f, 1.0f, 0.0f);
        App::DrawLine(m_posX + m_width/2, m_posY + m_height/2,
                     m_posX - m_width/2, m_posY + m_height/2, 0.0f, 1.0f, 0.0f);
        App::DrawLine(m_posX - m_width/2, m_posY + m_height/2,
                     m_posX - m_width/2, m_posY - m_height/2, 0.0f, 1.0f, 0.0f);
    }
    bool CheckCollision(const GameObject& other) override;
    
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
};
