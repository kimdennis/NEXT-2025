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

// Add these constants at the top with other constants
const int INITIAL_STROKES = 3;  // Starting strokes for new games

// Add these function declarations at the top after the includes
void HandleDragging(float mouseX, float mouseY, float ballX, float ballY);
void RenderMenu();
void RenderHUD();
void RenderAimingLine();
void RenderGameComplete();
void ResetGame();
void DrawProjectionLine(float startX, float startY, float angle, float power);
void DrawArrow(float x, float y, float angle, float size, float r, float g, float b);
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
    remainingStrokes = INITIAL_STROKES;
    totalStrokes = 0;
    
    auto& eventManager = GameEventManager::GetInstance();
    
    static bool eventsInitialized = false;
    if (!eventsInitialized) {
        eventManager.Subscribe(GameEventManager::EventType::HoleIn, 
            [](const GameEventManager::EventType&, void*) {
                currentPowerupChoices = powerupSystem.GetRandomPowerups(3);
                gameState = POWERUP_SELECT;
            });

        eventManager.Subscribe(GameEventManager::EventType::StrokeAdded, 
            [](const GameEventManager::EventType&, void*) {
                if (gameState != MENU) {
                    totalStrokes++;
                    remainingStrokes--;
                }
            });
            
        // Add subscription for collectible
        eventManager.Subscribe(GameEventManager::EventType::CollectibleCollected,
            [](const GameEventManager::EventType&, void*) {
                remainingStrokes++;  // Grant an extra stroke
            });
            
        eventsInitialized = true;
    }
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
    // Clear existing levels and powerups
    levels.clear();
    powerupSystem.ClearPowerups();
    currentPowerupChoices.clear();
    
    // Reset ALL stroke-related variables to zero first
    currentHoleIndex = 0;
    totalStrokes = 0;
    remainingStrokes = INITIAL_STROKES;
    
    // Generate new course
    CreateCourse();
    currentLevel = std::move(levels[0]);
    if (currentLevel) {
        currentLevel->Reset();
    }
    
    // Reset game state
    gameState = MENU;
    isDragging = false;
    power = 0.0f;
    aimAngle = 0.0f;
}


void RenderMenu() {
    App::Print(300, 300, "Albatross");
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

void DrawArrow(float x, float y, float angle, float size, float r, float g, float b) {
    // Calculate arrow head points
    float arrowAngle = 150.0f * (M_PI / 180.0f); // 150 degrees for arrow head
    
    // Right side of arrow head
    float rightX = x + cos(angle + arrowAngle) * size;
    float rightY = y + sin(angle + arrowAngle) * size;
    
    // Left side of arrow head
    float leftX = x + cos(angle - arrowAngle) * size;
    float leftY = y + sin(angle - arrowAngle) * size;
    
    // Draw arrow head with specified color
    App::DrawLine(x, y, rightX, rightY, r, g, b);
    App::DrawLine(x, y, leftX, leftY, r, g, b);
}

void DrawProjectionLine(float startX, float startY, float angle, float power) {
    if (!isDragging || !currentLevel) return;
    
    Ball* ball = currentLevel->GetBall();
    if (!ball || !ball->IsProjectionLineEnabled()) return;
    
    const int MAX_BOUNCES = 5;  
    const int STEPS_PER_BOUNCE = 15;  
    const float BOUNCE_DAMPENING = 0.8f;
    const float BALL_RADIUS = 6.0f;
    const float ENEMY_FORCE_MULTIPLIER = 1.2f;
    const float DOT_LENGTH = 5.0f;  
    const float DOT_SPACING = 20.0f; 
    const float ARROW_SIZE = 15.0f;  
    
    float normalizedPower = power / MAX_POWER;
    float velocityX = cos(angle) * normalizedPower * MAX_POWER * 3.5f;
    float velocityY = sin(angle) * normalizedPower * MAX_POWER * 3.5f;
    
    float currentX = startX;
    float currentY = startY;
    float timeStep = 0.016f;
    
    float accumulatedDistance = 0.0f;
    float lastDotX = startX;
    float lastDotY = startY;
    float lastValidX = startX;
    float lastValidY = startY;
    float lastValidAngle = angle;
    
    const auto& objects = currentLevel->GetObjects();
    
    // Draw the full line if in ghost mode, otherwise check for wall collisions
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
                
                float dotAngle = atan2(dy, dx);
                float dotEndX = dotX + cos(dotAngle) * DOT_LENGTH;
                float dotEndY = dotY + sin(dotAngle) * DOT_LENGTH;
                
                // Use different colors for ghost mode
                if (ball->IsPhaseMode()) {
                    App::DrawLine(dotX, dotY, dotEndX, dotEndY, 0.5f, 0.5f, 1.0f);  // Light blue for ghost mode
                } else {
                    App::DrawLine(dotX, dotY, dotEndX, dotEndY, 1.0f, 1.0f, 1.0f);  // White for normal mode
                }
                
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
            
            // Only check wall collisions if NOT in ghost mode
            if (!ball->IsPhaseMode()) {
                for (const auto& obj : objects) {
                    if (const Wall* wall = dynamic_cast<const Wall*>(obj.get())) {
                        float wallLeft = wall->GetPosition().x - wall->GetWidth()/2;
                        float wallRight = wall->GetPosition().x + wall->GetWidth()/2;
                        float wallTop = wall->GetPosition().y - wall->GetHeight()/2;
                        float wallBottom = wall->GetPosition().y + wall->GetHeight()/2;
                        
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
                }
            }
            
            // Update last valid position and angle
            lastValidX = currentX;
            lastValidY = currentY;
            lastValidAngle = atan2(velocityY, velocityX);
            
            if (collision && !ball->IsPhaseMode()) {
                break;
            }
        }
    }
    
    // Draw final arrow with appropriate color
    if (ball->IsPhaseMode()) {
        DrawArrow(lastValidX, lastValidY, lastValidAngle, ARROW_SIZE, 0.5f, 0.5f, 1.0f);  // Light blue for ghost mode
    } else {
        DrawArrow(lastValidX, lastValidY, lastValidAngle, ARROW_SIZE, 1.0f, 1.0f, 1.0f);  // White for normal mode
    }
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
        
        // Draw button border
        App::DrawLine(buttonX, buttonY, buttonX + POWERUP_BUTTON_WIDTH, buttonY, 1.0f, 1.0f, 1.0f);
        App::DrawLine(buttonX + POWERUP_BUTTON_WIDTH, buttonY, buttonX + POWERUP_BUTTON_WIDTH, buttonY + POWERUP_BUTTON_HEIGHT, 1.0f, 1.0f, 1.0f);
        App::DrawLine(buttonX + POWERUP_BUTTON_WIDTH, buttonY + POWERUP_BUTTON_HEIGHT, buttonX, buttonY + POWERUP_BUTTON_HEIGHT, 1.0f, 1.0f, 1.0f);
        App::DrawLine(buttonX, buttonY + POWERUP_BUTTON_HEIGHT, buttonX, buttonY, 1.0f, 1.0f, 1.0f);
        
        const float MARGIN = 10.0f;
        const float LINE_HEIGHT = 20.0f;
        
        // Draw powerup name at top (in yellow)
        App::Print(buttonX + MARGIN, buttonY + POWERUP_BUTTON_HEIGHT - LINE_HEIGHT, powerup.name.c_str(), 1.0f, 1.0f, 0.0f);
        
        // Draw description in middle (with word wrap)
        std::string desc = powerup.description;
        size_t maxCharsPerLine = (POWERUP_BUTTON_WIDTH - 2 * MARGIN) / 8; // Approximate characters that fit
        
        // Split description into lines if too long
        if (desc.length() > maxCharsPerLine) {
            size_t lastSpace = desc.rfind(' ', maxCharsPerLine);
            if (lastSpace != std::string::npos) {
                std::string line1 = desc.substr(0, lastSpace);
                std::string line2 = desc.substr(lastSpace + 1);
                App::Print(buttonX + MARGIN, buttonY + POWERUP_BUTTON_HEIGHT/2, line1.c_str());
                App::Print(buttonX + MARGIN, buttonY + POWERUP_BUTTON_HEIGHT/2 - LINE_HEIGHT, line2.c_str());
            }
        } else {
            App::Print(buttonX + MARGIN, buttonY + POWERUP_BUTTON_HEIGHT/2, desc.c_str());
        }
        
        // Draw cost text at bottom
        char costText[32];
        if (powerup.shotCost > 0) {
            sprintf_s(costText, "Cost: %d shots", powerup.shotCost);
        } else if (powerup.shotCost < 0) {
            sprintf_s(costText, "Gain: %d shots", -powerup.shotCost);
        } else {
            sprintf_s(costText, "Free!");
        }
        App::Print(buttonX + MARGIN, buttonY + MARGIN, costText);
    }
    
    // Draw current shots
    char shotsText[32];
    sprintf_s(shotsText, "Current Shots: %d", remainingStrokes);
    App::Print(10, SCREEN_HEIGHT - 30, shotsText);
}

void DrawProjectionLine(Ball* ball, float startX, float startY, float endX, float endY) {
    if (!ball) return;
    
    float dx = endX - startX;
    float dy = endY - startY;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 0.1f) return;
    
    float normalizedX = dx / distance;
    float normalizedY = dy / distance;
    
    const int NUM_POINTS = 20;
    float lastX = startX;
    float lastY = startY;
    bool stopped = false;
    
    // Draw the full line if in ghost mode, otherwise check for wall collisions
    for (int i = 1; i <= NUM_POINTS && !stopped; i++) {
        float t = (float)i / NUM_POINTS;
        float currentX = startX + normalizedX * distance * t;
        float currentY = startY + normalizedY * distance * t;
        
        // Only check wall collisions if NOT in ghost mode
        if (!ball->IsPhaseMode()) {
            for (const auto& obj : currentLevel->GetObjects()) {
                if (const Wall* wall = dynamic_cast<const Wall*>(obj.get())) {
                    float wallLeft = wall->GetPosition().x - wall->GetWidth()/2;
                    float wallRight = wall->GetPosition().x + wall->GetWidth()/2;
                    float wallTop = wall->GetPosition().y - wall->GetHeight()/2;
                    float wallBottom = wall->GetPosition().y + wall->GetHeight()/2;
                    
                    if (currentX >= wallLeft && currentX <= wallRight &&
                        currentY >= wallTop && currentY <= wallBottom) {
                        stopped = true;
                        break;
                    }
                }
            }
        }
        
        // Draw line segment with different color for ghost mode
        if (ball->IsPhaseMode()) {
            App::DrawLine(lastX, lastY, currentX, currentY, 0.5f, 0.5f, 1.0f);  // Light blue for ghost mode
        } else {
            App::DrawLine(lastX, lastY, currentX, currentY, 1.0f, 1.0f, 1.0f);  // White for normal mode
        }
        
        lastX = currentX;
        lastY = currentY;
    }
}