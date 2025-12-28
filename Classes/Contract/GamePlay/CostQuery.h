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