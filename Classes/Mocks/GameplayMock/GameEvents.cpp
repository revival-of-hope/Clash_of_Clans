#include "Classes/Contract/Gameplay/GameEvents.h"

#include <algorithm>

namespace Gameplay {

GameEventManager* GameEventManager::GetInstance() {
    static GameEventManager instance;
    return &instance;
}

void GameEventManager::AddListener(IGameEventListener* listener) {
    for (auto* existing : listeners_) {
        if (existing == listener) {
            return;
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
        listener->OnResourceChanged(evt);
    }
}

void GameEventManager::BroadcastTroopCountUpdated(const TroopCountUpdateEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnTroopCountUpdated(evt);
    }
}

void GameEventManager::BroadcastDeploymentSelectionChanged(const DeploymentSelectionEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnDeploymentSelectionChanged(evt);
    }
}

void GameEventManager::BroadcastEntitySpawned(const EntitySpawnEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnEntitySpawned(evt);
    }
}

void GameEventManager::BroadcastEntityDestroyed(const EntityDestroyEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnEntityDestroyed(evt);
    }
}

void GameEventManager::BroadcastBuildingStateChanged(const BuildingStateEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnBuildingStateChanged(evt);
    }
}

void GameEventManager::BroadcastEntityDamaged(const DamageEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnEntityDamaged(evt);
    }
}

void GameEventManager::BroadcastProjectileFired(const ProjectileEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnProjectileFired(evt);
    }
}

void GameEventManager::BroadcastProjectileHit(const ProjectileHitEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnProjectileHit(evt);
    }
}

void GameEventManager::BroadcastBattleStarted(const BattleStartEvent& evt) {
    for (auto* listener : listeners_) {
        listener->OnBattleStarted(evt);
    }
}

void GameEventManager::BroadcastBattleEnded(const BattleEndEvent& evt) {
    last_battle_end_ = evt;
    for (auto* listener : listeners_) {
        listener->OnBattleEnded(evt);
    }
}

void GameEventManager::BroadcastLootAvailabilityUpdated(const LootAvailabilityEvent& evt) {
    last_loot_availability_ = evt;
    for (auto* listener : listeners_) {
        listener->OnLootAvailabilityUpdated(evt);
    }
}

BattleEndEvent GameEventManager::GetLastBattleEnded() const {
    return last_battle_end_;
}

LootAvailabilityEvent GameEventManager::GetLastLootAvailability() const {
    return last_loot_availability_;
}

}  // namespace Gameplay
