// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// CostQuery - Read-only cost query API for UI integration.
// Contract header for external modules.
//
// Path: Classes/Contract/GamePlay/CostQuery.h

#ifndef CONTRACT_GAMEPLAY_COST_QUERY_H_
#define CONTRACT_GAMEPLAY_COST_QUERY_H_

#include "Core/GameConstants.h"

/**
 * @brief 资源成本结构体
 */
struct ResourceCost {
    int gold = 0;
    int elixir = 0;
    int population = 0;
    float time_seconds = 0;
};

/**
 * @brief 成本查询服务 (单例)
 *
 * 职责:
 * 1. 提供建筑放置成本查询
 * 2. 提供建筑升级成本查询
 * 3. 提供兵种训练成本查询
 * 4. 提供建造/训练时间查询
 *
 * 注意: 最大等级统一为 3 级
 */
class CostQuery {
public:
    static CostQuery* GetInstance();

    // =========================================================================
    // 建筑成本查询
    // =========================================================================

    ResourceCost GetBuildingPlacementCost(Core::BuildingType type, int level = 1) const;
    ResourceCost GetBuildingUpgradeCost(Core::BuildingType type, int current_level) const;
    float GetBuildingConstructionTime(Core::BuildingType type, int level) const;
    int GetBuildingMaxLevel(Core::BuildingType type) const;

    // =========================================================================
    // 兵种成本查询
    // =========================================================================

    ResourceCost GetTroopTrainingCost(Core::TroopType type, int level = 1) const;
    float GetTroopTrainingTime(Core::TroopType type, int level) const;
    int GetTroopHousingSpace(Core::TroopType type, int level) const;
    int GetTroopMaxLevel(Core::TroopType type) const;

private:
    CostQuery() = default;

    int GetBaseBuildingCost(Core::BuildingType type) const;
    float GetBaseBuildingTime(Core::BuildingType type) const;
    int GetBaseTroopCost(Core::TroopType type) const;
    float GetBaseTroopTime(Core::TroopType type) const;
};

#endif // CONTRACT_GAMEPLAY_COST_QUERY_H_