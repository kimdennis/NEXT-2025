#pragma once
#include "GameObject.h"
#include "App/app.h"
#include "Wall.h"
#include "Enemy.h"
#include "Collectible.h"
#include <vector>

// Add screen constants
extern const float SCREEN_WIDTH;
extern const float SCREEN_HEIGHT;

class Ball : public GameObject {
private:
    static constexpr float BOUNCE_DAMPENING = 0.8f;
    
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
    float m_speedMultiplier = 1.0f;
    float m_sizeMultiplier = 1.0f;
    bool m_phaseMode = false;
    bool m_enemyImmune = false;

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
    void HandleWallCollision(const Wall& wall);
    
    // Add these declarations
    void GetPosition(float& x, float& y) const override;
    void SetPosition(float x, float y) override;
    
    void SetSpeedMultiplier(float multiplier) { m_speedMultiplier = multiplier; }
    void SetSizeMultiplier(float multiplier) { m_sizeMultiplier = multiplier; }
    void SetPhaseMode(bool enabled) { m_phaseMode = enabled; }
    void SetEnemyImmune(bool enabled) { m_enemyImmune = enabled; }
    void ResetPowerups();
};