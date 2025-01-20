#include "stdafx.h"
#include "GameEventManager.h"

void GameEventManager::Subscribe(EventType type, EventCallback callback) {
    m_observers[type].push_back(callback);
}

void GameEventManager::Emit(EventType type, void* data) {
    if (m_observers.find(type) != m_observers.end()) {
        for (const auto& callback : m_observers[type]) {
            callback(type, data);
        }
    }
}

void GameEventManager::ClearAllSubscriptions() {
    m_observers.clear();
}
