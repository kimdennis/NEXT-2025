#pragma once
#include <memory>
#include "GameObject.h"
#include "Ball.h"
#include "Hole.h"

class GameObjectFactory {
public:
    enum class ObjectType {
        Ball,
        Hole,
        Wall,
        Water,
        Sand,
        Obstacle
    };

    static std::unique_ptr<GameObject> Create(ObjectType type, float x, float y);
    static std::unique_ptr<Ball> CreateBall(float x, float y);
    static std::unique_ptr<Hole> CreateHole(float x, float y, int par);
};
