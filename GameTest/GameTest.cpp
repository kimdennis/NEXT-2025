#include "stdafx.h"
#include <windows.h> 
#include <math.h>  
#include <algorithm>
#include "app/app.h"
#include "ball.h"
#include "hole.h"
#include "Level.h"
#include "GameObjectFactory.h"
#include "GameEventManager.h"
#include "Wall.h"
#include "Enemy.h"
#include "Collectible.h"
#include "LevelGenerator.h"

using namespace std;

//------------------------------------------------------------------------
// Game objects and state
//------------------------------------------------------------------------
std::unique_ptr<Level> currentLevel;
std::vector<std::unique_ptr<Level>> levels;
int currentHoleIndex = 0;
int totalStrokes = 0;

// Aiming variables
float aimAngle = 0.0f;
float power = 0.0f;
const float MAX_POWER = 80.0f;
const float MAX_DRAG_DISTANCE = 150.0f;
bool isDragging = false;
float dragStartX = 0.0f;
float dragStartY = 0.0f;

// Screen boundaries
const float SCREEN_WIDTH = 800.0f;
const float SCREEN_HEIGHT = 600.0f;
const float BALL_RADIUS = 6.0f;

// Game states
enum GameState {
    MENU,
    PLAYING,
    COMPLETE
};
GameState gameState = MENU;

// Physics timestep
const float FIXED_TIMESTEP = 1.0f / 240.0f;
float accumulator = 0.0f;

// Add these function declarations at the top after the includes
void HandleDragging(float mouseX, float mouseY, float ballX, float ballY);
void RenderMenu();
void RenderHUD();
void RenderAimingLine();
void RenderGameComplete();
void ResetGame();

void CreateCourse() {
    LevelGenerator generator;
    
    // Create multiple levels with increasing difficulty
    for (int i = 0; i < 5; i++) {
        levels.push_back(generator.GenerateLevel(i));
    }
}

void Init() {
    CreateCourse();
    currentLevel = std::move(levels[0]);
    
    // Subscribe to game events
    auto& eventManager = GameEventManager::GetInstance();
    eventManager.Subscribe(GameEventManager::EventType::HoleIn, 
        [](const GameEventManager::EventType&, void*) {
            if (currentHoleIndex + 1 < levels.size()) {
                currentHoleIndex++;
                currentLevel = std::move(levels[currentHoleIndex]);
            } else {
                gameState = COMPLETE;
            }
        });
}

void Update(float deltaTime) {
    float mouseX, mouseY;
    App::GetMousePos(mouseX, mouseY);
    
    switch (gameState) {
        case MENU:
            if (App::IsKeyPressed(VK_LBUTTON)) {
                gameState = PLAYING;
            }
            break;
            
        case PLAYING: {
            if (!currentLevel || currentHoleIndex >= levels.size()) {
                gameState = COMPLETE;
                return;
            }

            currentLevel->Update(deltaTime);
            
            // Mouse drag controls
            Ball* ball = currentLevel->GetBall();
            if (ball && !ball->IsMoving()) {
                float ballX, ballY;
                ball->GetPosition(ballX, ballY);
                
                if (App::IsKeyPressed(VK_LBUTTON) && !isDragging) {
                    if (ball->IsPointInside(mouseX, mouseY)) {
                        isDragging = true;
                        dragStartX = ballX;
                        dragStartY = ballY;
                    }
                }
                
                if (isDragging) {
                    HandleDragging(mouseX, mouseY, ballX, ballY);
                }
                
                if (!App::IsKeyPressed(VK_LBUTTON) && isDragging) {
                    ball->ApplyForce(power, aimAngle);
                    totalStrokes++;
                    isDragging = false;
                    power = 0.0f;
                }
            }
        } break;
            
        case COMPLETE:
            if (App::IsKeyPressed(VK_LBUTTON)) {
                ResetGame();
            }
            break;
    }
}

void Render() {
    switch (gameState) {
        case MENU:
            RenderMenu();
            break;
            
        case PLAYING:
            if (currentLevel) {
                currentLevel->Draw();
                RenderHUD();
                RenderAimingLine();
            }
            break;
            
        case COMPLETE:
            RenderGameComplete();
            break;
    }
}

void Shutdown() {
    currentLevel.reset();
    levels.clear();
}

// Helper functions
void HandleDragging(float mouseX, float mouseY, float ballX, float ballY) {
    float dx = mouseX - dragStartX;
    float dy = mouseY - dragStartY;
    aimAngle = atan2f(-dy, -dx);
    
    float dragDistance = sqrtf(dx * dx + dy * dy);
    power = (dragDistance / MAX_DRAG_DISTANCE) * MAX_POWER;
    power = power > MAX_POWER ? MAX_POWER : power;
}

void ResetGame() {
    currentHoleIndex = 0;
    totalStrokes = 0;
    CreateCourse();  // Recreate the course
    currentLevel = std::move(levels[0]);
    gameState = MENU;
}

// ... Add RenderMenu, RenderHUD, RenderAimingLine, and RenderGameComplete functions ...

void RenderMenu() {
    App::Print(300, 300, "Mini Golf");
    App::Print(300, 250, "Click to Start");
}

void RenderHUD() {
    char buffer[32];
    sprintf_s(buffer, "Strokes: %d", totalStrokes);
    App::Print(10, 10, buffer);
    
    if (currentLevel) {
        sprintf_s(buffer, "Par: %d", currentLevel->GetPar());
        App::Print(10, 30, buffer);
    }
}

void RenderAimingLine() {
    if (isDragging && currentLevel && currentLevel->GetBall()) {
        float ballX, ballY;
        currentLevel->GetBall()->GetPosition(ballX, ballY);
        
        float lineEndX = ballX - cos(aimAngle) * (power / MAX_POWER) * 50.0f;
        float lineEndY = ballY - sin(aimAngle) * (power / MAX_POWER) * 50.0f;
        
        App::DrawLine(ballX, ballY, lineEndX, lineEndY, 1.0f, 0.0f, 0.0f);
    }
}

void RenderGameComplete() {
    char buffer[32];
    App::Print(300, 300, "Course Complete!");
    sprintf_s(buffer, "Total Strokes: %d", totalStrokes);
    App::Print(300, 250, buffer);
    App::Print(300, 200, "Click to Play Again");
}