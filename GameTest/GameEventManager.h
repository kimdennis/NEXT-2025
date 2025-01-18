#pragma once
#include <functional>
#include <map>
#include <vector>

class GameEventManager {
public:
    enum class EventType {
        BallCollision,
        HoleIn,
        OutOfBounds,
        StrokeAdded,
        LevelComplete,
        GameComplete
    };

    using EventCallback = std::function<void(const EventType&, void*)>;

    static GameEventManager& GetInstance() {
        static GameEventManager instance;
        return instance;
    }

    void Subscribe(EventType type, EventCallback callback);
    void Emit(EventType type, void* data = nullptr);

private:
    GameEventManager() = default;
    std::map<EventType, std::vector<EventCallback>> m_observers;
};
