// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// GameEvents.h - Event System for Gameplay-UI decoupling
// Uses Observer Pattern for push-based notifications
// [UPDATE] Added StarUpdateEvent for battle star tracking
//
// Path: Classes/Contract/GamePlay/GameEvents.h

#ifndef CONTRACT_GAMEPLAY_GAMEEVENTS_H_
#define CONTRACT_GAMEPLAY_GAMEEVENTS_H_

#include <string>
#include <vector>

#include "Core/GameConstants.h"

namespace Gameplay {

    // =============================================================================
    // 事件数据结构定义
    // =============================================================================

    /**
     * @brief 资源变化事件
     */
    struct ResourceUpdateEvent {
        std::string resource_type;   // "Gold", "Elixir", "Population"
        int current_amount = 0;
        int max_capacity = 0;
        int change_amount = 0;
    };

    /**
     * @brief 兵种数量变化事件
     */
    struct TroopCountUpdateEvent {
        int owner_id = 0;
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
        int remaining_count = 0;
    };

    /**
     * @brief 部署选择事件
     */
    struct DeploymentSelectionEvent {
        int owner_id = 0;
        bool has_selection = false;
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
    };

    /**
     * @brief 实体生成事件
     */
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

    /**
     * @brief 实体销毁事件
     */
    struct EntityDestroyEvent {
        int instance_id = 0;
        bool is_building = false;
    };

    /**
     * @brief 建筑状态枚举
     */
    enum class BuildingState {
        kConstructing,
        kIdle,
        kDestroyed
    };

    /**
     * @brief 建筑状态变化事件
     */
    struct BuildingStateEvent {
        int instance_id = 0;
        Core::BuildingType type = Core::BuildingType::kTownHall;
        BuildingState new_state = BuildingState::kIdle;
        float time_remaining = 0.0f;
        float total_build_time = 0.0f;
    };

    /**
     * @brief 伤害事件
     */
    struct DamageEvent {
        int target_instance_id = 0;
        int damage_amount = 0;
        int current_hp = 0;
        int max_hp = 0;
        bool is_critical = false;
    };

    /**
     * @brief 投射物发射事件
     */
    struct ProjectileEvent {
        int source_id = 0;
        float target_x = 0.0f;
        float target_y = 0.0f;
        Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
    };

    /**
     * @brief 投射物命中事件
     */
    struct ProjectileHitEvent {
        float x = 0.0f;
        float y = 0.0f;
        Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
    };

    /**
     * @brief 战斗开始事件
     */
    struct BattleStartEvent {
        int time_limit_seconds = 0;
        int total_buildings = 0;      // [NEW] 总建筑数
        bool has_town_hall = false;   // [NEW] 是否有大本营
    };

    /**
     * @brief 战斗结果枚举
     */
    enum class BattleResult {
        kVictory,
        kDefeat,
        kTimeOut
    };

    /**
     * @brief 战斗结束事件
     */
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

    /**
     * @brief 大本营等级变化事件
     * 触发时机: 大本营升级完成时
     */
    struct TownHallLevelChangedEvent {
        int new_level = 1;               // 新等级
        int old_level = 1;               // 旧等级
    };

    // =============================================================================
    // 事件监听器接口
    // =============================================================================

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
        virtual void OnTownHallLevelChanged(const TownHallLevelChangedEvent& evt) {}  // [NEW]
    };

    // =============================================================================
    // 事件管理器 (单例)
    // =============================================================================

    class GameEventManager {
    public:
        static GameEventManager* GetInstance();

        // 监听器管理
        void AddListener(IGameEventListener* listener);
        void RemoveListener(IGameEventListener* listener);

        // 事件广播
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
        void BroadcastTownHallLevelChanged(const TownHallLevelChangedEvent& evt);  // [NEW]

        // 获取上次战斗结果
        BattleEndEvent GetLastBattleEnded() const;

    private:
        GameEventManager() = default;

        std::vector<IGameEventListener*> listeners_;
        BattleEndEvent last_battle_end_{};
    };

}  // namespace Gameplay

#endif  // CONTRACT_GAMEPLAY_GAMEEVENTS_H_