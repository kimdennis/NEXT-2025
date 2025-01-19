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

// Add at the top of the file with other includes
#define _USE_MATH_DEFINES
#include <cmath>

// If M_PI is still not defined, define it manually
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
const float MAX_POWER = 100.0f;
const float MAX_DRAG_DISTANCE = 600.0f;
bool isDragging = false;
float dragStartX = 0.0f;
float dragStartY = 0.0f;

// Screen boundaries
const float SCREEN_WIDTH = 1024.0f;
const float SCREEN_HEIGHT = 768.0f;
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

// Add these constants at the top with your other constants
const int PROJECTION_DOTS = 10;  // Number of dots to show in the projection
const float DOT_SPACING = 20.0f; // Space between projection dots
const float DOT_SIZE = 3.0f;     // Size of each projection dot

// Add these function declarations at the top after the includes
void HandleDragging(float mouseX, float mouseY, float ballX, float ballY);
void RenderMenu();
void RenderHUD();
void RenderAimingLine();
void RenderGameComplete();
void ResetGame();
void DrawProjectionLine(float startX, float startY, float angle, float power);
void DrawArrow(float x, float y, float angle, float size);

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
                // If we were moving before (isDragging was false) and now we're stopped,
                // randomize the objects
                static bool wasMoving = false;
                if (wasMoving) {
                    currentLevel->RandomizeObjects();
                    wasMoving = false;
                }
                
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
                    wasMoving = true;  // Set this flag when we start moving
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
                
                // Get ball position correctly
                Ball* ball = currentLevel->GetBall();
                if (ball) {
                    float ballX, ballY;
                    ball->GetPosition(ballX, ballY);
                    DrawProjectionLine(ballX, ballY, aimAngle, power);
                }
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
        
        float lineEndX = ballX - cos(aimAngle) * (power / MAX_POWER) * 150.0f;
        float lineEndY = ballY - sin(aimAngle) * (power / MAX_POWER) * 150.0f;
        
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

void DrawArrow(float x, float y, float angle, float size) {
    // Calculate arrow head points
    float arrowAngle = 150.0f * (M_PI / 180.0f); // 150 degrees for arrow head
    
    // Right side of arrow head
    float rightX = x + cos(angle + arrowAngle) * size;
    float rightY = y + sin(angle + arrowAngle) * size;
    
    // Left side of arrow head
    float leftX = x + cos(angle - arrowAngle) * size;
    float leftY = y + sin(angle - arrowAngle) * size;
    
    // Draw arrow head
    App::DrawLine(x, y, rightX, rightY, 1.0f, 1.0f, 1.0f);
    App::DrawLine(x, y, leftX, leftY, 1.0f, 1.0f, 1.0f);
}

void DrawProjectionLine(float startX, float startY, float angle, float power) {
    if (!isDragging || !currentLevel) return;

    const int MAX_BOUNCES = 5;  
    const int STEPS_PER_BOUNCE = 15;  
    const float BOUNCE_DAMPENING = 0.8f;
    const float BALL_RADIUS = 6.0f;
    const float ENEMY_FORCE_MULTIPLIER = 1.2f;
    const float DOT_LENGTH = 5.0f;  
    const float DOT_SPACING = 20.0f; 
    const float ARROW_SIZE = 15.0f;  
    
    float normalizedPower = power / MAX_POWER;
    float velocityX = cos(angle) * normalizedPower * MAX_POWER * 3.5f;  // Added 1.5x multiplier
    float velocityY = sin(angle) * normalizedPower * MAX_POWER * 3.5f;  // Added 1.5x multiplier
    
    float currentX = startX;
    float currentY = startY;
    float timeStep = 0.016f;
    
    float accumulatedDistance = 0.0f;  // Track distance for dot spacing
    float lastDotX = startX;  // Position of last drawn dot
    float lastDotY = startY;
    float lastValidX = startX;  // Last valid position for arrow
    float lastValidY = startY;
    float lastValidAngle = angle;  // Last valid angle for arrow
    
    const auto& objects = currentLevel->GetObjects();
    
    for (int bounce = 0; bounce <= MAX_BOUNCES; bounce++) {
        for (int step = 0; step < STEPS_PER_BOUNCE; step++) {
            float prevX = currentX;
            float prevY = currentY;
            
            currentX += velocityX * timeStep;
            currentY += velocityY * timeStep;
            
            // Calculate distance moved this step
            float dx = currentX - prevX;
            float dy = currentY - prevY;
            float stepDistance = sqrt(dx * dx + dy * dy);
            accumulatedDistance += stepDistance;
            
            // Draw dots when we've moved far enough
            while (accumulatedDistance >= DOT_SPACING) {
                float t = DOT_SPACING / accumulatedDistance;
                float dotX = prevX + dx * t;
                float dotY = prevY + dy * t;
                
                // Draw a small line segment for the dot
                float dotAngle = atan2(dy, dx);
                float dotEndX = dotX + cos(dotAngle) * DOT_LENGTH;
                float dotEndY = dotY + sin(dotAngle) * DOT_LENGTH;
                
                App::DrawLine(dotX, dotY, dotEndX, dotEndY, 1.0f, 1.0f, 1.0f);
                
                lastDotX = dotX;
                lastDotY = dotY;
                accumulatedDistance -= DOT_SPACING;
            }
            
            bool collision = false;
            
            // Check level boundaries
            if (currentX - BALL_RADIUS < 0) {
                currentX = BALL_RADIUS;
                velocityX = -velocityX * BOUNCE_DAMPENING;
                collision = true;
            }
            else if (currentX + BALL_RADIUS > SCREEN_WIDTH) {
                currentX = SCREEN_WIDTH - BALL_RADIUS;
                velocityX = -velocityX * BOUNCE_DAMPENING;
                collision = true;
            }
            
            if (currentY - BALL_RADIUS < 0) {
                currentY = BALL_RADIUS;
                velocityY = -velocityY * BOUNCE_DAMPENING;
                collision = true;
            }
            else if (currentY + BALL_RADIUS > SCREEN_HEIGHT) {
                currentY = SCREEN_HEIGHT - BALL_RADIUS;
                velocityY = -velocityY * BOUNCE_DAMPENING;
                collision = true;
            }
            
            // Check collisions with all objects
            for (const auto& obj : objects) {
                // Check walls
                if (const Wall* wall = dynamic_cast<const Wall*>(obj.get())) {
                    float wallLeft = wall->GetPosition().x - wall->GetWidth()/2;
                    float wallRight = wall->GetPosition().x + wall->GetWidth()/2;
                    float wallTop = wall->GetPosition().y - wall->GetHeight()/2;
                    float wallBottom = wall->GetPosition().y + wall->GetHeight()/2;
                    
                    // Check if current position (including ball radius) intersects with wall
                    if (currentX + BALL_RADIUS > wallLeft && currentX - BALL_RADIUS < wallRight &&
                        currentY + BALL_RADIUS > wallTop && currentY - BALL_RADIUS < wallBottom) {
                        
                        float rightPen = (currentX + BALL_RADIUS) - wallLeft;
                        float leftPen = wallRight - (currentX - BALL_RADIUS);
                        float bottomPen = (currentY + BALL_RADIUS) - wallTop;
                        float topPen = wallBottom - (currentY - BALL_RADIUS);
                        
                        float minPenX = (rightPen < leftPen) ? -rightPen : leftPen;
                        float minPenY = (bottomPen < topPen) ? -bottomPen : topPen;
                        
                        if (abs(minPenX) < abs(minPenY)) {
                            velocityX = -velocityX * BOUNCE_DAMPENING;
                            currentX = prevX;
                        } else {
                            velocityY = -velocityY * BOUNCE_DAMPENING;
                            currentY = prevY;
                        }
                        collision = true;
                        break;
                    }
                }
                // Check enemies
                else if (const Enemy* enemy = dynamic_cast<const Enemy*>(obj.get())) {
                    float enemyX, enemyY;
                    enemy->GetPosition(enemyX, enemyY);
                    
                    float dx = currentX - enemyX;
                    float dy = currentY - enemyY;
                    float distanceSquared = dx * dx + dy * dy;
                    float minDistance = BALL_RADIUS + enemy->GetSize();
                    
                    if (distanceSquared < minDistance * minDistance) {
                        // Calculate deflection angle (8 possible directions)
                        float angleToEnemy = atan2(dy, dx);
                        float deflectionAngle = round(angleToEnemy / (M_PI / 4)) * (M_PI / 4);
                        
                        // Calculate new velocity based on deflection angle
                        float currentSpeed = sqrt(velocityX * velocityX + velocityY * velocityY);
                        float newSpeed = currentSpeed * ENEMY_FORCE_MULTIPLIER;
                        
                        velocityX = cos(deflectionAngle) * newSpeed;
                        velocityY = sin(deflectionAngle) * newSpeed;
                        
                        // Move back to previous position to prevent sticking
                        currentX = prevX;
                        currentY = prevY;
                        
                        collision = true;
                        break;
                    }
                }
            }
            
            if (collision) {
                // Store last valid position and angle before collision
                lastValidX = prevX;
                lastValidY = prevY;
                lastValidAngle = atan2(velocityY, velocityX);
                break;
            }
            
            // Update last valid position and angle
            lastValidX = currentX;
            lastValidY = currentY;
            lastValidAngle = atan2(velocityY, velocityX);
        }
    }
    
    // Draw arrow at the end of the last valid position
    DrawArrow(lastValidX, lastValidY, lastValidAngle, ARROW_SIZE);
}