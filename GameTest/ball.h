#pragma once
#include "GameObject.h"
#include "App/app.h"
#include <vector>

// Add screen constants
extern const float SCREEN_WIDTH;
extern const float SCREEN_HEIGHT;

class Ball : public GameObject {
private:
    float m_prevPosX;
    float m_prevPosY;
    float m_velocityX;
    float m_velocityY;
    float m_accelerationX;
    float m_accelerationY;
    float m_friction;
    float m_mass;
    float m_radius;
    bool m_isMoving;
    float m_accumulator;

    static void DrawCircle(float x, float y, float radius, float r, float g, float b, float a);

public:
    Ball(float x, float y);
    ~Ball() override = default;
    
    // GameObject interface implementation
    void Update(float deltaTime) override;
    void Draw() override;
    bool CheckCollision(const GameObject& other) override;
    
    // Ball-specific methods
    void ApplyForce(float power, float angle);
    void SetVelocity(float vx, float vy);
    void Stop();
    
    void GetInterpolatedPosition(float alpha, float& x, float& y) const;
    
    // Velocity getters
    float GetVelocityX() const { return m_velocityX; }
    float GetVelocityY() const { return m_velocityY; }
    
    // Utility methods
    bool IsPointInside(float px, float py);
    bool IsMoving() const { return m_isMoving; }
    float GetRadius() const { return m_radius; }
    
    void HandleCollision(Ball& other);
    void HandleBoundaryCollisions();
    
    // Add these declarations
    void GetPosition(float& x, float& y) const override;
    void SetPosition(float x, float y) override;
};