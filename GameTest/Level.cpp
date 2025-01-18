#include "stdafx.h"
#include "Level.h"
#include "GameEventManager.h"

Level::Level(int par) : m_par(par), m_strokes(0) {}

void Level::Update(float deltaTime) {
    if (m_ball) {
        m_ball->Update(deltaTime);
        
        // Check wall collisions
        for (const auto& obj : m_objects) {
            if (auto wall = dynamic_cast<Wall*>(obj.get())) {
                if (wall->CheckCollision(*m_ball)) {
                    m_ball->HandleWallCollision(*wall);
                }
            }
        }
        
        // Check if ball is in hole
        if (m_hole && m_ball->IsMoving()) {
            float ballX, ballY;
            m_ball->GetPosition(ballX, ballY);
            if (m_hole->IsInHole(ballX, ballY)) {
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
