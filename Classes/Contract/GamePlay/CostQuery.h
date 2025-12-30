// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// CostQuery - Query costs and TownHall level restrictions.
// [UPDATE] Added TownHall level restriction system
//
// Path: Classes/Contract/GamePlay/CostQuery.h

#ifndef CONTRACT_GAMEPLAY_COSTQUERY_H_
#define CONTRACT_GAMEPLAY_COSTQUERY_H_

#include "Core/GameConstants.h"
#include <vector>

/**
 * @brief 资源消耗结构
 */
struct ResourceCost {
    int gold = 0;
    int elixir = 0;
    int population = 0;
    float time_seconds = 0.0f;  // [NEW] 时间消耗 (秒)
    
    // 默认构造函数
    ResourceCost() = default;
    
    // 从单个 int 构造 (用于简单的金币消耗)
    ResourceCost(int gold_cost) : gold(gold_cost), elixir(0), population(0), time_seconds(0.0f) {}
    
    // 完整构造函数
    ResourceCost(int gold_cost, int elixir_cost, int pop_cost = 0, float time = 0.0f) 
        : gold(gold_cost), elixir(elixir_cost), population(pop_cost), time_seconds(time) {}
    
    // 从 int 赋值 (用于简单的金币消耗)
    ResourceCost& operator=(int gold_cost) {
        gold = gold_cost;
        elixir = 0;
        population = 0;
        time_seconds = 0.0f;
        return *this;
    }
    
    // 判断是否为零消耗
    bool IsZero() const {
        return gold == 0 && elixir == 0 && population == 0;
    }
    
    // 加法运算
    ResourceCost operator+(const ResourceCost& other) const {
        return ResourceCost(gold + other.gold, elixir + other.elixir, 
                           population + other.population, time_seconds + other.time_seconds);
    }
    
    // 乘法运算 (用于等级倍率)
    ResourceCost operator*(float multiplier) const {
        return ResourceCost(
            static_cast<int>(gold * multiplier),
            static_cast<int>(elixir * multiplier),
            population,  // 人口不受倍率影响
            time_seconds * multiplier
        );
    }
};

/**
 * @brief 费用与限制查询系统 (单例)
 * 
 * 职责:
 * 1. 查询建造/训练费用
 * 2. 查询大本营等级限制 (可建造建筑类型、建筑等级上限)
 */
class CostQuery {
public:
    static CostQuery* GetInstance();

    // ==========================================================================
    // 费用查询
    // ==========================================================================
    
    /**
     * @brief 获取建筑建造费用
     */
    ResourceCost GetBuildingPlacementCost(Core::BuildingType type, int level) const;

    /**
     * @brief 获取建筑升级费用
     */
    ResourceCost GetBuildingUpgradeCost(Core::BuildingType type, int current_level) const;

    /**
     * @brief 获取兵种训练费用
     */
    ResourceCost GetTroopTrainingCost(Core::TroopType type, int level) const;

    /**
     * @brief 获取匹配战费用 (搜索对手)
     * @return 匹配所需资源 (主要是金币)
     */
    ResourceCost GetMatchmakingCost() const;

    /**
     * @brief 获取建筑建造时间
     * @param type 建筑类型
     * @param level 等级
     * @return 建造时间 (秒)
     */
    float GetBuildingConstructionTime(Core::BuildingType type, int level) const;

    /**
     * @brief 获取建筑升级时间
     * @param type 建筑类型
     * @param current_level 当前等级
     * @return 升级时间 (秒)
     */
    float GetBuildingUpgradeTime(Core::BuildingType type, int current_level) const;

    // ==========================================================================
    // 兵种查询
    // ==========================================================================
    
    /**
     * @brief 获取兵种训练时间
     * @param type 兵种类型
     * @param level 等级
     * @return 训练时间 (秒)
     */
    float GetTroopTrainingTime(Core::TroopType type, int level) const;

    /**
     * @brief 获取兵种人口占用
     * @param type 兵种类型
     * @return 人口占用
     */
    int GetTroopHousingSpace(Core::TroopType type) const;

    /**
     * @brief 获取兵种最大等级
     * @param type 兵种类型
     * @return 最大等级 (当前版本均为3)
     */
    int GetTroopMaxLevel(Core::TroopType type) const;

    // ==========================================================================
    // 基础数值查询 (UI 显示用)
    // ==========================================================================
    
    /**
     * @brief 获取建筑基础费用 (1级)
     */
    ResourceCost GetBaseBuildingCost(Core::BuildingType type) const;

    /**
     * @brief 获取建筑基础建造时间 (1级)
     */
    float GetBaseBuildingTime(Core::BuildingType type) const;

    /**
     * @brief 获取兵种基础费用 (1级)
     */
    ResourceCost GetBaseTroopCost(Core::TroopType type) const;

    /**
     * @brief 获取兵种基础训练时间 (1级)
     */
    float GetBaseTroopTime(Core::TroopType type) const;

    /**
     * @brief 获取建筑最大等级 (无需大本营参数版本)
     * @param type 建筑类型
     * @return 最大等级 (当前版本均为3)
     */
    int GetBuildingMaxLevel(Core::BuildingType type) const;

    // ==========================================================================
    // 大本营等级限制查询
    // ==========================================================================
    
    /**
     * @brief 检查在当前大本营等级下是否可以建造某类型建筑
     * @param townhall_level 大本营等级
     * @param building_type 要建造的建筑类型
     * @return true 可以建造
     */
    bool CanUnlockBuilding(int townhall_level, Core::BuildingType building_type) const;

    /**
     * @brief 获取某建筑在当前大本营等级下的最大允许等级
     * @param townhall_level 大本营等级
     * @param building_type 建筑类型
     * @return 最大等级 (0表示不能建造)
     */
    int GetMaxBuildingLevel(int townhall_level, Core::BuildingType building_type) const;

    /**
     * @brief 获取某建筑在当前大本营等级下的最大数量
     * @param townhall_level 大本营等级
     * @param building_type 建筑类型
     * @return 最大数量 (0表示不能建造)
     */
    int GetMaxBuildingCount(int townhall_level, Core::BuildingType building_type) const;

    /**
     * @brief 获取当前大本营等级下可建造的所有建筑类型
     * @param townhall_level 大本营等级
     * @return 可建造的建筑类型列表
     */
    std::vector<Core::BuildingType> GetUnlockedBuildings(int townhall_level) const;

    /**
     * @brief 获取解锁某建筑所需的大本营等级
     * @param building_type 建筑类型
     * @return 所需大本营等级
     */
    int GetRequiredTownHallLevel(Core::BuildingType building_type) const;

private:
    CostQuery() = default;
};

#endif  // CONTRACT_GAMEPLAY_COSTQUERY_H_
