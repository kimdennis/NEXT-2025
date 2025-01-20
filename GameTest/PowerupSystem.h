#pragma once
#include "stdafx.h"
#include <vector>
#include <string>
#include <functional>
#include "Ball.h"

struct Powerup {
    std::string name;
    std::string description;
    int shotCost;  // Negative means gaining shots
    std::function<void(Ball*)> effect;

    // Add constructor for proper initialization
    Powerup(const std::string& n, const std::string& desc, int cost, std::function<void(Ball*)> fx)
        : name(n), description(desc), shotCost(cost), effect(fx) {}
};

class PowerupSystem {
private:
    std::vector<Powerup> m_powerupPool;
    std::vector<Powerup> m_activePowerups;

public:
    PowerupSystem();
    std::vector<Powerup> GetRandomPowerups(int count = 3);
    void ApplyPowerup(const Powerup& powerup, Ball* ball);
    void ClearPowerups();
    void InitializePowerupPool();
};
