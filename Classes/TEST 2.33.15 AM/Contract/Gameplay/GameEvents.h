// Source: Classes/Integration/Docs/Gameplay_Public_GameEvents_Documentation.md (GameEvents)
#ifndef Contract_GAMEPLAY_GAMEEVENTS_H_
#define Contract_GAMEPLAY_GAMEEVENTS_H_

#include <string>
#include <vector>

#include "Core/GameConstants.h"

namespace Gameplay {

struct ResourceUpdateEvent {
    std::string resource_type;
    int current_amount = 0;
    int max_capacity = 0;
    int change_amount = 0;
};

struct TroopCountUpdateEvent {
    int owner_id = 0;
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    int remaining_count = 0;
};

struct DeploymentSelectionEvent {
    int owner_id = 0;
    bool has_selection = false;
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
};

struct EntitySpawnEvent {
    int instance_id = 0;
    int owner_id = 0;
    float x = 0.0f;
    float y = 0.0f;
    int level = 0;
    int current_hp = 0;
    int max_hp = 0;
    bool is_building = false;
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    Core::BuildingType building_type = Core::BuildingType::kTownHall;
};

struct EntityDestroyEvent {
    int instance_id = 0;
    bool is_building = false;
};

enum class BuildingState {
    kConstructing,
    kIdle,
    kDestroyed
};

struct BuildingStateEvent {
    int instance_id = 0;
    Core::BuildingType type = Core::BuildingType::kTownHall;
    BuildingState new_state = BuildingState::kIdle;
    float time_remaining = 0.0f;
    float total_build_time = 0.0f;
};

struct DamageEvent {
    int target_instance_id = 0;
    int damage_amount = 0;
    int current_hp = 0;
    int max_hp = 0;
    bool is_critical = false;
};

struct ProjectileEvent {
    int source_id = 0;
    float target_x = 0.0f;
    float target_y = 0.0f;
    Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
};

struct ProjectileHitEvent {
    float x = 0.0f;
    float y = 0.0f;
    Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
};

struct BattleStartEvent {
    int time_limit_seconds = 0;
};

enum class BattleResult {
    kVictory,
    kDefeat,
    kTimeOut
};

struct BattleEndEvent {
    BattleResult result = BattleResult::kVictory;
    int stars_earned = 0;
    int destruction_percent = 0;
    int gold_stolen = 0;
    int elixir_stolen = 0;
    int battle_duration_seconds = 0;
    int troops_deployed = 0;
    int troops_remaining = 0;
    int spells_used = 0;
};

class IGameEventListener {
public:
    virtual ~IGameEventListener() = default;

    virtual void OnResourceChanged(const ResourceUpdateEvent& evt) {}
    virtual void OnTroopCountUpdated(const TroopCountUpdateEvent& evt) {}
    virtual void OnDeploymentSelectionChanged(const DeploymentSelectionEvent& evt) {}
    virtual void OnEntitySpawned(const EntitySpawnEvent& evt) {}
    virtual void OnEntityDestroyed(const EntityDestroyEvent& evt) {}
    virtual void OnBuildingStateChanged(const BuildingStateEvent& evt) {}
    virtual void OnEntityDamaged(const DamageEvent& evt) {}
    virtual void OnProjectileFired(const ProjectileEvent& evt) {}
    virtual void OnProjectileHit(const ProjectileHitEvent& evt) {}
    virtual void OnBattleStarted(const BattleStartEvent& evt) {}
    virtual void OnBattleEnded(const BattleEndEvent& evt) {}
};

class GameEventManager {
public:
    static GameEventManager* GetInstance();

    void AddListener(IGameEventListener* listener);
    void RemoveListener(IGameEventListener* listener);

    void BroadcastResourceChange(const ResourceUpdateEvent& evt);
    void BroadcastTroopCountUpdated(const TroopCountUpdateEvent& evt);
    void BroadcastDeploymentSelectionChanged(const DeploymentSelectionEvent& evt);
    void BroadcastEntitySpawned(const EntitySpawnEvent& evt);
    void BroadcastEntityDestroyed(const EntityDestroyEvent& evt);
    void BroadcastBuildingStateChanged(const BuildingStateEvent& evt);
    void BroadcastEntityDamaged(const DamageEvent& evt);
    void BroadcastProjectileFired(const ProjectileEvent& evt);
    void BroadcastProjectileHit(const ProjectileHitEvent& evt);
    void BroadcastBattleStarted(const BattleStartEvent& evt);
    void BroadcastBattleEnded(const BattleEndEvent& evt);

    BattleEndEvent GetLastBattleEnded() const;

private:
    GameEventManager() = default;

    std::vector<IGameEventListener*> listeners_;
    BattleEndEvent last_battle_end_{};
};

}  // namespace Gameplay

#endif  // Contract_GAMEPLAY_GAMEEVENTS_H_
