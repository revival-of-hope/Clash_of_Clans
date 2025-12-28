// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// GameEvents.h - Event System for Gameplay-UI decoupling
// Uses Observer Pattern for push-based notifications
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
     * 触发时机: 金币/圣水/人口 增减时
     */
    struct ResourceUpdateEvent {
        std::string resource_type;   // "Gold", "Elixir", "Population"
        int current_amount = 0;      // 变化后的当前值
        int max_capacity = 0;        // 最大容量
        int change_amount = 0;       // 变化量 (+50 或 -200)
    };

    /**
     * @brief 兵种数量变化事件
     * 触发时机: 兵种训练完成、部署、死亡
     */
    struct TroopCountUpdateEvent {
        int owner_id = 0;
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
        int remaining_count = 0;
    };

    /**
     * @brief 部署选择事件
     * 触发时机: 玩家选择/取消选择要部署的兵种
     */
    struct DeploymentSelectionEvent {
        int owner_id = 0;
        bool has_selection = false;
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
    };

    /**
     * @brief 实体生成事件
     * 触发时机: 单位部署、建筑放置、地图初始化
     */
    struct EntitySpawnEvent {
        int instance_id = 0;         // 全局唯一ID
        int owner_id = 0;            // 阵营ID (0: 玩家, 1: 敌方)
        float x = 0.0f;              // 世界坐标
        float y = 0.0f;
        int level = 0;               // 等级
        int current_hp = 0;          // 当前血量
        int max_hp = 0;              // 最大血量
        bool is_building = false;    // 是否为建筑
        Core::TroopType troop_type = Core::TroopType::kBarbarian;
        Core::BuildingType building_type = Core::BuildingType::kTownHall;
    };

    /**
     * @brief 实体销毁事件
     * 触发时机: 单位死亡、建筑摧毁
     */
    struct EntityDestroyEvent {
        int instance_id = 0;
        bool is_building = false;
    };

    /**
     * @brief 建筑状态枚举
     */
    enum class BuildingState {
        kConstructing,   // 建造中
        kIdle,           // 正常状态
        kDestroyed       // 已摧毁
    };

    /**
     * @brief 建筑状态变化事件
     * 触发时机: 开始建造、建造完成、被摧毁
     */
    struct BuildingStateEvent {
        int instance_id = 0;
        Core::BuildingType type = Core::BuildingType::kTownHall;
        BuildingState new_state = BuildingState::kIdle;
        float time_remaining = 0.0f;    // 剩余建造时间
        float total_build_time = 0.0f;  // 总建造时间
    };

    /**
     * @brief 伤害事件
     * 触发时机: 实体受到攻击
     */
    struct DamageEvent {
        int target_instance_id = 0;
        int damage_amount = 0;
        int current_hp = 0;
        int max_hp = 0;
        bool is_critical = false;    // 预留: 暴击
    };

    /**
     * @brief 投射物发射事件
     * 触发时机: 远程攻击发射
     */
    struct ProjectileEvent {
        int source_id = 0;           // 发射者 ID
        float target_x = 0.0f;       // 目标位置
        float target_y = 0.0f;
        Core::ProjectileType projectile_type = Core::ProjectileType::kNone;
    };

    /**
     * @brief 投射物命中事件
     * 触发时机: 投射物到达目标
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

    // =============================================================================
    // 事件监听器接口
    // =============================================================================

    /**
     * @brief 游戏事件监听器接口
     * UI/Audio 层需要继承此接口并实现感兴趣的回调
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
    };

    // =============================================================================
    // 事件管理器 (单例)
    // =============================================================================

    /**
     * @brief 游戏事件管理器
     * 负责管理监听器和广播事件
     */
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

        // 获取上次战斗结果 (供结算界面使用)
        BattleEndEvent GetLastBattleEnded() const;

    private:
        GameEventManager() = default;

        std::vector<IGameEventListener*> listeners_;
        BattleEndEvent last_battle_end_{};
    };

}  // namespace Gameplay

#endif  // CONTRACT_GAMEPLAY_GAMEEVENTS_H_