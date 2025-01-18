#include "stdafx.h"
#include "GameObjectFactory.h"

std::unique_ptr<GameObject> GameObjectFactory::Create(ObjectType type, float x, float y) {
    switch (type) {
        case ObjectType::Ball:
            return std::make_unique<Ball>(x, y);
        case ObjectType::Hole:
            return std::make_unique<Hole>(x, y, x + 600, y, 3);
        default:
            return nullptr;
    }
}

std::unique_ptr<Ball> GameObjectFactory::CreateBall(float x, float y) {
    return std::make_unique<Ball>(x, y);
}

std::unique_ptr<Hole> GameObjectFactory::CreateHole(float targetX, float targetY, int par) {
    // Start position is passed as the hole's initial position
    return std::make_unique<Hole>(100.0f, targetY, targetX, targetY, par);
}
