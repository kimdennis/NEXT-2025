#pragma once
#include "GameObject.h"
#include "App/app.h"

struct Vector2 {
    float x, y;
};

class Wall : public GameObject {
private:
    float m_width;
    float m_height;

public:
    Wall(float x, float y, float width, float height) 
        : GameObject(x, y), m_width(width), m_height(height) {}
    
    void Update(float deltaTime) override {}
    void Draw() override;
    bool CheckCollision(const GameObject& other) override;
    
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    Vector2 GetPosition() const { return Vector2{m_posX, m_posY}; }
};
