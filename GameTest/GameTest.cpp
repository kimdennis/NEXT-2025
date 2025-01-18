#include "stdafx.h"
#include <windows.h> 
#include <math.h>  
#include <algorithm>
#include "app/app.h"
#include "ball.h"
#include "hole.h"

using namespace std;

//------------------------------------------------------------------------
// Game objects and state
//------------------------------------------------------------------------
Ball* golfBall = nullptr;
std::vector<Hole*> holes;
int currentHole = 0;
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


void CreateCourse() {
    // Hole 1: Simple straight shot
    Hole* hole1 = new Hole(100.0f, 300.0f, 700.0f, 300.0f, 3);
    //hole1->AddObstacle(400.0f, 200.0f, 50.0f, 200.0f);
    holes.push_back(hole1);
    
    // Hole 2: Diagonal with multiple obstacles
    Hole* hole2 = new Hole(100.0f, 500.0f, 700.0f, 100.0f, 4);
    //hole2->AddObstacle(300.0f, 300.0f, 200.0f, 50.0f);
    //hole2->AddObstacle(500.0f, 400.0f, 50.0f, 200.0f);
    holes.push_back(hole2);
    
    // Hole 3: Maze-like
    Hole* hole3 = new Hole(100.0f, 100.0f, 700.0f, 500.0f, 5);
    //hole3->AddObstacle(300.0f, 100.0f, 50.0f, 400.0f);
    //hole3->AddObstacle(500.0f, 200.0f, 50.0f, 400.0f);
    holes.push_back(hole3);
}

void Init() {
    CreateCourse();
    float startX, startY;
    holes[0]->GetStartPosition(startX, startY);
    golfBall = new Ball(startX, startY);
}

void Update(float deltaTime) {
    float mouseX, mouseY;
    App::GetMousePos(mouseX, mouseY);
    
    Hole* currentHoleObj = nullptr;
    if (currentHole < holes.size()) {
        currentHoleObj = holes[currentHole];
    }

    switch (gameState) {
        case MENU:
            if (App::IsKeyPressed(VK_LBUTTON)) {
                gameState = PLAYING;
            }
            break;
            
        case PLAYING:
            if (currentHole >= holes.size() || !currentHoleObj) {
                gameState = COMPLETE;
                return;
            }

            // Fixed timestep physics update
            accumulator += deltaTime;
            while (accumulator >= FIXED_TIMESTEP) {
                golfBall->Update(FIXED_TIMESTEP);
                accumulator -= FIXED_TIMESTEP;
            }
            
            float ballX, ballY;
            golfBall->GetPosition(ballX, ballY);
            
            // Check if ball is in hole
            if (currentHoleObj->IsInHole(ballX, ballY)) {
                currentHole++;
                if (currentHole < holes.size()) {
                    float nextX, nextY;
                    holes[currentHole]->GetStartPosition(nextX, nextY);
                    golfBall->SetPosition(nextX, nextY);
                }
                return;
            }
            
            // Check collision with obstacles
            if (currentHoleObj->CheckCollision(ballX, ballY)) {
                float vx = -golfBall->GetVelocityX() * 0.5f;
                float vy = -golfBall->GetVelocityY() * 0.5f;
                golfBall->SetVelocity(vx, vy);
            }
            
            // Mouse drag controls
            if (!golfBall->IsMoving()) {
                if (App::IsKeyPressed(VK_LBUTTON) && !isDragging) {
                    if (golfBall->IsPointInside(mouseX, mouseY)) {
                        isDragging = true;
                        dragStartX = ballX;
                        dragStartY = ballY;
                    }
                }
                
                if (isDragging) {
                    float dx = mouseX - dragStartX;
                    float dy = mouseY - dragStartY;
                    aimAngle = static_cast<float>(atan2(-dy, -dx));
                    
                    float dragDistance = static_cast<float>(sqrt(dx * dx + dy * dy));
                    power = (dragDistance / MAX_DRAG_DISTANCE) * MAX_POWER;
                    if (power > MAX_POWER) power = MAX_POWER;
                }
                
                if (!App::IsKeyPressed(VK_LBUTTON) && isDragging) {
                    golfBall->ApplyForce(power, aimAngle);
                    totalStrokes++;
                    isDragging = false;
                    power = 0.0f;
                }
            }
            break;
            
        case COMPLETE:
            if (App::IsKeyPressed(VK_LBUTTON)) {
                currentHole = 0;
                totalStrokes = 0;
                float startX, startY;
                holes[0]->GetStartPosition(startX, startY);
                golfBall->SetPosition(startX, startY);
                gameState = MENU;
            }
            break;
    }
}

void Render() {
    switch (gameState) {
        case MENU:
            App::Print(static_cast<float>(SCREEN_WIDTH/2.0f), static_cast<float>(SCREEN_HEIGHT/2.0f), "Mini Golf");
            App::Print(static_cast<float>(SCREEN_WIDTH/2.0f), static_cast<float>(SCREEN_HEIGHT/2.0f - 50.0f), "Click to Start");
            break;
            
        case PLAYING:
            if (currentHole < holes.size()) {
                holes[currentHole]->Draw();
                
                if (isDragging) {
                    float ballX, ballY;
                    golfBall->GetPosition(ballX, ballY);
                    
                    float mouseX, mouseY;
                    App::GetMousePos(mouseX, mouseY);
                    
                    App::DrawLine(ballX, ballY, mouseX, mouseY, 1.0f, 0.0f, 0.0f);
                    
                    const int NUM_SEGMENTS = 20;
                    float projectedX = ballX - (mouseX - ballX);
                    float projectedY = ballY - (mouseY - ballY);
                    
                    float dx = (projectedX - ballX) / NUM_SEGMENTS;
                    float dy = (projectedY - ballY) / NUM_SEGMENTS;
                    
                    for (int i = 0; i < NUM_SEGMENTS; i += 2) {
                        float startX = ballX + dx * i;
                        float startY = ballY + dy * i;
                        float endX = ballX + dx * (i + 1);
                        float endY = ballY + dy * (i + 1);
                        
                        App::DrawLine(startX, startY, endX, endY, 1.0f, 1.0f, 1.0f);
                    }
                }
                
                golfBall->Draw();
                
                App::Print(50.0f, 50.0f, "Hole: %d/%d", currentHole + 1, static_cast<int>(holes.size()));
                App::Print(50.0f, 30.0f, "Strokes: %d", totalStrokes);
                App::Print(50.0f, 10.0f, "Par: %d", holes[currentHole]->GetPar());
            }
            break;
            
        case COMPLETE:
            App::Print(static_cast<float>(SCREEN_WIDTH/2), static_cast<float>(SCREEN_HEIGHT/2), "Course Complete!");
            App::Print(static_cast<float>(SCREEN_WIDTH/2), static_cast<float>(SCREEN_HEIGHT/2 - 50), 
                      "Total Strokes: %d", totalStrokes);
            App::Print(static_cast<float>(SCREEN_WIDTH/2), static_cast<float>(SCREEN_HEIGHT/2 - 100), 
                      "Click to Play Again");
            break;
    }
}

void Shutdown() {
    delete golfBall;
    for (Hole* hole : holes) {
        delete hole;
    }
    holes.clear();
}