#pragma once
#include "App/app.h"

class GameObject {
protected:
    float m_posX, m_posY;
    float m_width, m_height;

public:
    GameObject(float x, float y) : m_posX(x), m_posY(y), m_width(0), m_height(0) {}
    virtual ~GameObject() = default;
    
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual bool CheckCollision(const GameObject& other) = 0;
    
    virtual void GetPosition(float& x, float& y) const {
        x = m_posX;
        y = m_posY;
    }
    
    virtual void SetPosition(float x, float y) {
        m_posX = x;
        m_posY = y;
    }
};
