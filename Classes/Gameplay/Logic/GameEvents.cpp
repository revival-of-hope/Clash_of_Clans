// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// GameEvents.cpp - Event Manager Implementation
// [UPDATE] Added missing broadcast functions
//
// Path: Classes/Gameplay/Logic/GameEvents.cpp

#include "Contract/GamePlay/GameEvents.h"

#include <algorithm>

namespace Gameplay {

GameEventManager* GameEventManager::GetInstance() {
    static GameEventManager instance;
    return &instance;
}

void GameEventManager::AddListener(IGameEventListener* listener) {
    if (!listener) return;
    
    for (auto* existing : listeners_) {
        if (existing == listener) {
            return;  // 已存在，不重复添加
        }
    }
    listeners_.push_back(listener);
}

void GameEventManager::RemoveListener(IGameEventListener* listener) {
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), listener),
        listeners_.end());
}

void GameEventManager::BroadcastResourceChange(const ResourceUpdateEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnResourceChanged(evt);
    }
}

void GameEventManager::BroadcastTroopCountUpdated(const TroopCountUpdateEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnTroopCountUpdated(evt);
    }
}

void GameEventManager::BroadcastDeploymentSelectionChanged(const DeploymentSelectionEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnDeploymentSelectionChanged(evt);
    }
}

void GameEventManager::BroadcastEntitySpawned(const EntitySpawnEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnEntitySpawned(evt);
    }
}

void GameEventManager::BroadcastEntityDestroyed(const EntityDestroyEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnEntityDestroyed(evt);
    }
}

void GameEventManager::BroadcastBuildingStateChanged(const BuildingStateEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnBuildingStateChanged(evt);
    }
}

void GameEventManager::BroadcastEntityDamaged(const DamageEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnEntityDamaged(evt);
    }
}

void GameEventManager::BroadcastProjectileFired(const ProjectileEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnProjectileFired(evt);
    }
}

void GameEventManager::BroadcastProjectileHit(const ProjectileHitEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnProjectileHit(evt);
    }
}

void GameEventManager::BroadcastBattleStarted(const BattleStartEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnBattleStarted(evt);
    }
}

void GameEventManager::BroadcastBattleEnded(const BattleEndEvent& evt) {
    last_battle_end_ = evt;
    for (auto* listener : listeners_) {
        if (listener) listener->OnBattleEnded(evt);
    }
}

BattleEndEvent GameEventManager::GetLastBattleEnded() const {
    return last_battle_end_;
}

void GameEventManager::BroadcastTownHallLevelChanged(const TownHallLevelChangedEvent& evt) {
    for (auto* listener : listeners_) {
        if (listener) listener->OnTownHallLevelChanged(evt);
    }
}

void GameEventManager::BroadcastLootAvailabilityUpdated(const LootAvailabilityEvent& evt) {
    last_loot_availability_ = evt;  // 保存最后一次战利品信息
    for (auto* listener : listeners_) {
        if (listener) listener->OnLootAvailabilityUpdated(evt);
    }
}

LootAvailabilityEvent GameEventManager::GetLastLootAvailability() const {
    return last_loot_availability_;
}

}  // namespace Gameplay
