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
#include "PowerupSystem.h"

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
    POWERUP_SELECT,
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

// Add these variables with other game state variables
int remainingStrokes = 3;  // Starting strokes
const int STROKES_PER_LEVEL = 3;  // Strokes gained per level

// Add these function declarations at the top after the includes
void HandleDragging(float mouseX, float mouseY, float ballX, float ballY);
void RenderMenu();
void RenderHUD();
void RenderAimingLine();
void RenderGameComplete();
void ResetGame();
void DrawProjectionLine(float startX, float startY, float angle, float power);
void DrawArrow(float x, float y, float angle, float size);
void RenderPowerupSelect();

// Add as global variables
PowerupSystem powerupSystem;
std::vector<Powerup> currentPowerupChoices;

// Constants for powerup selection UI
const float POWERUP_BUTTON_WIDTH = 200.0f;
const float POWERUP_BUTTON_HEIGHT = 150.0f;

void CreateCourse() {
    LevelGenerator generator;
    
    // Generate initial set of levels
    for (int i = 0; i < 3; i++) {  // Start with 3 levels
        levels.push_back(generator.GenerateLevel(i));
    }
}

void Init() {
    CreateCourse();
    currentLevel = std::move(levels[0]);
    remainingStrokes = 3;  // Initialize starting strokes
    
    // Subscribe to game events
    auto& eventManager = GameEventManager::GetInstance();
    eventManager.Subscribe(GameEventManager::EventType::HoleIn, 
        [](const GameEventManager::EventType&, void*) {
            currentPowerupChoices = powerupSystem.GetRandomPowerups(3);
            gameState = POWERUP_SELECT;
        });

    eventManager.Subscribe(GameEventManager::EventType::CollectibleCollected, 
        [](const GameEventManager::EventType&, void*) {
            remainingStrokes++; // Add one stroke when collecting
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
            if (!currentLevel) {
                return;
            }

            Ball* ball = currentLevel->GetBall();
            if (!ball) return;

            // Update level and check if we need to deduct a stroke
            currentLevel->Update(deltaTime);
            
            // Check for stroke deduction when ball stops moving
            static bool wasMoving = false;
            if (wasMoving && !ball->IsMoving()) {
                remainingStrokes--;
                totalStrokes++;
                
                // Get ball position
                float ballX, ballY;
                ball->GetPosition(ballX, ballY);
                
                // Only check for game over if we're out of strokes AND not in the hole
                if (remainingStrokes <= 0 && !currentLevel->GetHole()->IsInHole(ballX, ballY)) {
                    gameState = COMPLETE;
                    return;
                }
            }
            wasMoving = ball->IsMoving();
            
            // Mouse drag controls
            if (!ball->IsMoving()) {
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
                    if (remainingStrokes > 0) {
                        ball->ApplyForce(power, aimAngle);
                    }
                    isDragging = false;
                    power = 0.0f;
                }
            }
        } break;
            
        case POWERUP_SELECT: {
            if (App::IsKeyPressed(VK_LBUTTON)) {
                float mouseX, mouseY;
                App::GetMousePos(mouseX, mouseY);
                
                // Calculate button positions and check clicks
                for (int i = 0; i < currentPowerupChoices.size(); i++) {
                    float buttonX = (SCREEN_WIDTH / 4) * (i + 1) - POWERUP_BUTTON_WIDTH/2;
                    float buttonY = SCREEN_HEIGHT/2 - POWERUP_BUTTON_HEIGHT/2;
                    
                    // Check if mouse is within button bounds
                    if (mouseX >= buttonX && mouseX <= buttonX + POWERUP_BUTTON_WIDTH &&
                        mouseY >= buttonY && mouseY <= buttonY + POWERUP_BUTTON_HEIGHT) {
                        
                        const Powerup& selected = currentPowerupChoices[i];
                        
                        // First add the 3 free shots for completing the level
                        remainingStrokes += STROKES_PER_LEVEL;
                        
                        // Then apply the powerup cost
                        remainingStrokes -= selected.shotCost;
                        
                        // Generate and move to next level
                        LevelGenerator generator;
                        levels.push_back(generator.GenerateLevel(currentHoleIndex + 1));
                        currentHoleIndex++;
                        currentLevel = std::move(levels[currentHoleIndex]);
                        
                        // Apply the powerup to the new level's ball
                        powerupSystem.ApplyPowerup(selected, currentLevel->GetBall());
                        
                        gameState = PLAYING;
                        break;
                    }
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
            
        case POWERUP_SELECT:
            RenderPowerupSelect();
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
    remainingStrokes = 3;  // Reset to starting strokes
    CreateCourse();
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
    sprintf_s(buffer, "Level: %d", currentHoleIndex + 1);
    App::Print(10, 10, buffer);
    
    sprintf_s(buffer, "Total Strokes: %d", totalStrokes);
    App::Print(10, 30, buffer);
    
    sprintf_s(buffer, "Strokes Left: %d", remainingStrokes);
    App::Print(10, 50, buffer);
    
    if (currentLevel) {
        sprintf_s(buffer, "Par: %d", currentLevel->GetPar());
        App::Print(10, 70, buffer);
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
    App::Print(300, 350, "Game Over!");
    sprintf_s(buffer, "Final Level: %d", currentHoleIndex + 1);
    App::Print(300, 300, buffer);
    sprintf_s(buffer, "Total Strokes Used: %d", totalStrokes);
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

void RenderPowerupSelect() {
    // Draw title
    App::Print(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT - 100, "Choose Your Powerup!");
    
    // Draw each powerup option
    for (int i = 0; i < currentPowerupChoices.size(); i++) {
        const Powerup& powerup = currentPowerupChoices[i];
        float buttonX = (SCREEN_WIDTH / 4) * (i + 1) - POWERUP_BUTTON_WIDTH/2;
        float buttonY = SCREEN_HEIGHT/2 - POWERUP_BUTTON_HEIGHT/2;
        
        // Draw button background (filled rectangle)
        for (int x = 0; x < POWERUP_BUTTON_WIDTH; x += 2) {
            App::DrawLine(
                buttonX + x, buttonY,
                buttonX + x, buttonY + POWERUP_BUTTON_HEIGHT,
                0.2f, 0.2f, 0.2f
            );
        }
        
        // Draw button border (white square)
        App::DrawLine(buttonX, buttonY, buttonX + POWERUP_BUTTON_WIDTH, buttonY, 1.0f, 1.0f, 1.0f);  // Top
        App::DrawLine(buttonX + POWERUP_BUTTON_WIDTH, buttonY, buttonX + POWERUP_BUTTON_WIDTH, buttonY + POWERUP_BUTTON_HEIGHT, 1.0f, 1.0f, 1.0f);  // Right
        App::DrawLine(buttonX + POWERUP_BUTTON_WIDTH, buttonY + POWERUP_BUTTON_HEIGHT, buttonX, buttonY + POWERUP_BUTTON_HEIGHT, 1.0f, 1.0f, 1.0f);  // Bottom
        App::DrawLine(buttonX, buttonY + POWERUP_BUTTON_HEIGHT, buttonX, buttonY, 1.0f, 1.0f, 1.0f);  // Left
        
        // Draw powerup info
        App::Print(buttonX + 10, buttonY + POWERUP_BUTTON_HEIGHT - 30, 
                  powerup.name.c_str(), 1.0f, 1.0f, 0.0f);
        
        App::Print(buttonX + 10, buttonY + POWERUP_BUTTON_HEIGHT - 60, 
                  powerup.description.c_str());
        
        char costText[32];
        if (powerup.shotCost > 0) {
            sprintf_s(costText, "Cost: %d shots", powerup.shotCost);
        } else if (powerup.shotCost < 0) {
            sprintf_s(costText, "Gain: %d shots", -powerup.shotCost);
        } else {
            sprintf_s(costText, "Free!");
        }
        App::Print(buttonX + 10, buttonY + POWERUP_BUTTON_HEIGHT - 90, costText);
    }
    
    // Draw current shots
    char shotsText[32];
    sprintf_s(shotsText, "Current Shots: %d", remainingStrokes);
    App::Print(10, SCREEN_HEIGHT - 30, shotsText);
}