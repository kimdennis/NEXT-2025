#pragma once
#include "App/app.h"
#include <vector>

class Ball {
private:
    float m_posX;
    float m_posY;
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

public:
    Ball(float x, float y);
    ~Ball() {}
    
    void Update(float deltaTime);
    void Draw();
    void ApplyForce(float power, float angle);
    void SetVelocity(float vx, float vy);
    void Stop();
    
    // Position methods
    void GetPosition(float& x, float& y) const;
    void SetPosition(float x, float y);
    void GetInterpolatedPosition(float alpha, float& x, float& y) const;
    
    // Velocity getters
    float GetVelocityX() const { return m_velocityX; }
    float GetVelocityY() const { return m_velocityY; }
    
    // Utility methods
    bool IsPointInside(float px, float py);
    bool IsMoving() const { return m_isMoving; }
    float GetRadius() const { return m_radius; }
    
    void HandleCollision(Ball& other);

protected:
    void DrawCircle(float x, float y, float radius, float r, float g, float b, float a);
};