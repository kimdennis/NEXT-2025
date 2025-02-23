#include "stdafx.h"
#include "PowerupSystem.h"
#include "Ball.h"
#include <random>
#include <algorithm>

PowerupSystem::PowerupSystem() {
    InitializePowerupPool();
}

void PowerupSystem::InitializePowerupPool() {
    m_powerupPool = std::vector<Powerup>{
        Powerup(
            "Speed Demon",
            "Increase ball speed by 30%",
            0,  // Free
            [](Ball* ball) {
                ball->SetSpeedMultiplier(1.3f);
            }
        ),
        Powerup(
            "Slow & Steady",
            "Decrease speed by 20%",
            -2,  // Gain 2 shots
            [](Ball* ball) {
                ball->SetSpeedMultiplier(0.8f);
            }
        ),
        Powerup(
            "Golf Corpse",
            "Pass through walls for this level",
            2,  // Costs 2 shot
            [](Ball* ball) {
                ball->SetPhaseMode(true);
            }
        ),
        Powerup(
            "Oblivious",
            "Ignore Enemies for this level",
            1,  // Costs 1 shot
            [](Ball* ball) {
                ball->SetEnemyImmune(true);
            }
        ),
        Powerup(
            "Big Baller",
            "Grow in size by 50% for this level",
            -2,  // Gain 2 shots
            [](Ball* ball) {
                ball->SetSizeMultiplier(1.50f);
            }
        ),
        Powerup(
            "Blind Shot",
            "Disables projection line for this level",
            -2,  // Gain 2 shots
            [](Ball* ball) {
                ball->SetProjectionLineEnabled(false);
            }
        )
    };
}

std::vector<Powerup> PowerupSystem::GetRandomPowerups(int count) {
    std::vector<Powerup> available = m_powerupPool;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(available.begin(), available.end(), gen);
    
    // Use direct comparison instead of std::min
    int actualCount = count;
    if (actualCount > (int)available.size()) {
        actualCount = (int)available.size();
    }
    
    return std::vector<Powerup>(available.begin(), available.begin() + actualCount);
}

void PowerupSystem::ApplyPowerup(const Powerup& powerup, Ball* ball) {
    powerup.effect(ball);
    m_activePowerups.push_back(powerup);
}

void PowerupSystem::ClearPowerups() {
    m_activePowerups.clear();
}
