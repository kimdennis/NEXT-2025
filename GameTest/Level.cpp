#include "stdafx.h"
#include "Level.h"
#include "LevelGenerator.h"
#include "GameEventManager.h"
#include "Enemy.h"
#include "Collectible.h"
#include <cmath>

Level::Level(int par) : m_par(par), m_strokes(0) {}

void Level::Update(float deltaTime) {
    if (m_ball) {
        m_ball->Update(deltaTime);
        
        // Check collisions with all objects
        for (auto& obj : m_objects) {
            if (obj) {
                m_ball->CheckCollision(*obj);
                obj->CheckCollision(*m_ball);
            }
        }
        
        // Check if ball is in hole
        if (m_hole && m_ball->IsMoving()) {
            float ballX, ballY;
            m_ball->GetPosition(ballX, ballY);
            if (m_hole->IsInHole(ballX, ballY, m_ball->GetVelocityX(), m_ball->GetVelocityY())) {
                m_ball->Stop();
                GameEventManager::GetInstance().Emit(GameEventManager::EventType::HoleIn);
            }
        }
    }
    
    for (auto& obj : m_objects) {
        obj->Update(deltaTime);
    }
}

void Level::Draw() {
    if (m_hole) m_hole->Draw();
    if (m_ball) m_ball->Draw();
    
    for (auto& obj : m_objects) {
        obj->Draw();
    }
}

void Level::Reset() {
    m_strokes = 0;
    if (m_ball && m_hole) {
        float startX, startY;
        m_hole->GetStartPosition(startX, startY);
        m_ball->SetPosition(startX, startY);
        m_ball->Stop();
        m_ball->ResetPowerups();  // Reset powerups when resetting level
    }
}

void Level::AddObject(std::unique_ptr<GameObject> obj) {
    m_objects.push_back(std::move(obj));
}

void Level::SetBall(std::unique_ptr<Ball> ball) {
    m_ball = std::move(ball);
}

void Level::SetHole(std::unique_ptr<Hole> hole) {
    m_hole = std::move(hole);
}

void Level::AddStroke() {
    m_strokes++;
    GameEventManager::GetInstance().Emit(GameEventManager::EventType::StrokeAdded);
}

void Level::RandomizeObjects() {
    // Keep track of existing objects' types and counts
    int enemyCount = 0;
    int collectibleCount = 0;
    int wallCount = 0;
    
    // Count existing objects
    for (const auto& obj : m_objects) {
        if (dynamic_cast<Enemy*>(obj.get())) enemyCount++;
        else if (dynamic_cast<Collectible*>(obj.get())) collectibleCount++;
        else if (dynamic_cast<Wall*>(obj.get())) wallCount++;
    }
    
    // Clear existing objects
    m_objects.clear();
    
    // Create new LevelGenerator instance for helper functions
    LevelGenerator generator;
    
    // Regenerate walls
    for (int i = 0; i < wallCount; i++) {
        float x = generator.GetRandomFloat(100.0f, SCREEN_WIDTH - 100.0f);
        float y = generator.GetRandomFloat(100.0f, SCREEN_HEIGHT - 100.0f);
        float width = generator.GetRandomFloat(50.0f, 150.0f);
        float height = generator.GetRandomFloat(20.0f, 100.0f);
        
        auto wall = std::make_unique<Wall>(x, y, width, height);
        if (generator.IsPositionValid(x, y, std::sqrt(width*width + height*height) * 0.5f, m_objects)) {
            m_objects.push_back(std::move(wall));
        }
    }
    
    // Regenerate enemies
    for (int i = 0; i < enemyCount; i++) {
        float x = generator.GetRandomFloat(100.0f, SCREEN_WIDTH - 100.0f);
        float y = generator.GetRandomFloat(100.0f, SCREEN_HEIGHT - 100.0f);
        
        auto enemy = std::make_unique<Enemy>(x, y);
        if (generator.IsPositionValid(x, y, 15.0f, m_objects)) {
            m_objects.push_back(std::move(enemy));
        }
    }
    
    // Regenerate collectibles
    for (int i = 0; i < collectibleCount; i++) {
        float x = generator.GetRandomFloat(100.0f, SCREEN_WIDTH - 100.0f);
        float y = generator.GetRandomFloat(100.0f, SCREEN_HEIGHT - 100.0f);
        
        auto collectible = std::make_unique<Collectible>(x, y);
        if (generator.IsPositionValid(x, y, 8.0f, m_objects)) {
            m_objects.push_back(std::move(collectible));
        }
    }
}
