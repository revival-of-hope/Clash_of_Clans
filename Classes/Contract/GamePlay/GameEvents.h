// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// GameEvents.h - Event System for Gameplay-UI decoupling
// Uses Observer Pattern for push-based notifications
//
// Path: Classes/Contract/Gameplay/GameEvents.h

#ifndef CONTRACT_GAMEPLAY_GAMEEVENTS_H_
#define CONTRACT_GAMEPLAY_GAMEEVENTS_H_

#include <string>
#include <vector>

#include "Core/GameConstants.h"

namespace Gameplay {

    // =============================================================================
    // Event Data Structure Definitions
    // =============================================================================

    /**
     * @brief Resource Update Event
     * Trigger timing: When Gold/Elixir/Population/Gems/Trophies increases or decreases
     */
    struct ResourceUpdateEvent {
        std::string resource_type;   // "Gold", "Elixir", "Population", "Gems", "Trophies"
        int current_amount = 0;      // Current value after change
        int max_capacity = 0;        // Max capacity
        int change_amount = 0;       // Amount changed (+50 or -200)
    };

    /**
     * @brief Troop Count Update Event
     * Trigger timing: Troop training complete, deployment, death
     */
    struct TroopCountUpdateEvent {
        int owner_id = 0;
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
        int remaining_count = 0;
    };

    /**
     * @brief Deployment Selection Event
     * Trigger timing: Player selects/deselects a troop to deploy
     */
    struct DeploymentSelectionEvent {
        int owner_id = 0;
        bool has_selection = false;
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
    };

    /**
     * @brief Entity Spawn Event
     * Trigger timing: Unit deployment, building placement, map initialization
     */
    struct EntitySpawnEvent {
        int instance_id = 0;         // Global unique ID
        int owner_id = 0;            // Faction ID (0: Player, 1: Enemy)
        float x = 0.0f;              // World coordinate
        float y = 0.0f;
        int level = 0;               // Level
        int current_hp = 0;          // Current HP
        int max_hp = 0;              // Max HP
        bool is_building = false;    // Is building
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
        Core::BuildingType building_type = Core::BuildingType::kTownHall;
    };

    /**
     * @brief Entity Destroy Event
     * Trigger timing: Unit death, building destruction
     */
    struct EntityDestroyEvent {
        int instance_id = 0;
        bool is_building = false;
    };

    /**
     * @brief Building State Enum
     */
    enum class BuildingState {
        kConstructing,   // Constructing
        kIdle,           // Normal/Idle
        kDestroyed       // Destroyed
    };

    /**
     * @brief Building State Change Event
     * Trigger timing: Start construction, construction complete, destroyed
     */
    struct BuildingStateEvent {
        int instance_id = 0;
        Core::BuildingType type = Core::BuildingType::kTownHall;
        BuildingState new_state = BuildingState::kIdle;
        float time_remaining = 0.0f;    // Remaining construction time
        float total_build_time = 0.0f;  // Total construction time
    };

    /**
     * @brief Damage Event
     * Trigger timing: Entity takes damage
     */
    struct DamageEvent {
        int target_instance_id = 0;
        int damage_amount = 0;
        int current_hp = 0;
        int max_hp = 0;
        bool is_critical = false;    // Reserved: Critical hit
    };

    /**
     * @brief Projectile Fired Event
     * Trigger timing: Ranged attack fired
     */
    struct ProjectileEvent {
        int source_id = 0;           // Shooter ID
        float target_x = 0.0f;       // Target position
        float target_y = 0.0f;
        Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
    };

    /**
     * @brief Projectile Hit Event
     * Trigger timing: Projectile reaches target
     */
    struct ProjectileHitEvent {
        float x = 0.0f;
        float y = 0.0f;
        Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
    };

    /**
     * @brief Battle Start Event
     */
    struct BattleStartEvent {
        int time_limit_seconds = 0;
    };

    /**
     * @brief Battle Result Enum
     */
    enum class BattleResult {
        kVictory,
        kDefeat,
        kTimeOut
    };

    /**
     * @brief Battle End Event
     */
    struct BattleEndEvent {
        BattleResult result = BattleResult::kVictory;
        int stars_earned = 0;
        int destruction_percent = 0;
        int gold_stolen = 0;
        int elixir_stolen = 0;
        int trophies_earned = 0;
        int trophies_total = 0;
        int battle_duration_seconds = 0;
        int troops_deployed = 0;
        int troops_remaining = 0;
        int spells_used = 0;
    };

    /**
     * @brief Loot Availability Event
     * Trigger timing: Match preview/scouting provides available loot/trophies.
     */
    struct LootAvailabilityEvent {
        int gold_available = 0;
        int elixir_available = 0;
        int trophies_available = 0;
    };

    // =============================================================================
    // Event Listener Interface
    // =============================================================================

    /**
     * @brief Game Event Listener Interface
     * UI/Audio layers need to inherit this interface and implement interested callbacks
     */
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
        virtual void OnLootAvailabilityUpdated(const LootAvailabilityEvent& evt) {}
    };

    // =============================================================================
    // Event Manager (Singleton)
    // =============================================================================

    /**
     * @brief Game Event Manager
     * Responsible for managing listeners and broadcasting events
     */
    class GameEventManager {
    public:
        static GameEventManager* GetInstance();

        // Listener Management
        void AddListener(IGameEventListener* listener);
        void RemoveListener(IGameEventListener* listener);

        // Event Broadcasting
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
        void BroadcastLootAvailabilityUpdated(const LootAvailabilityEvent& evt);

        // Get last battle result (for settlement screen)
        BattleEndEvent GetLastBattleEnded() const;
        LootAvailabilityEvent GetLastLootAvailability() const;

    private:
        GameEventManager() = default;

        std::vector<IGameEventListener*> listeners_;
        BattleEndEvent last_battle_end_{};
        LootAvailabilityEvent last_loot_availability_{};
    };

}  // namespace Gameplay

#endif  // CONTRACT_GAMEPLAY_GAMEEVENTS_H_
