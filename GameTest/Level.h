#pragma once
#include <memory>
#include <vector>
#include "GameObject.h"
#include "Ball.h"
#include "Hole.h"

class Level {
private:
    std::vector<std::unique_ptr<GameObject>> m_objects;
    std::unique_ptr<Ball> m_ball;
    std::unique_ptr<Hole> m_hole;
    int m_par;
    int m_strokes;

public:
    Level(int par);
    ~Level() = default;

    void Update(float deltaTime);
    void Draw();
    void Reset();

    void AddObject(std::unique_ptr<GameObject> obj);
    void SetBall(std::unique_ptr<Ball> ball);
    void SetHole(std::unique_ptr<Hole> hole);

    Ball* GetBall() const { return m_ball.get(); }
    Hole* GetHole() const { return m_hole.get(); }
    int GetPar() const { return m_par; }
    int GetStrokes() const { return m_strokes; }
    void AddStroke();
};
