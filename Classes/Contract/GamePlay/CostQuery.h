// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// CostQuery - Read-only cost query API for UI integration.
// Contract header for external modules.
//
// Source: Classes/Integration/Docs/Gameplay Cost Query API.md :: 1.1 ResourceCost, 1.2 Interface
//
// Path: Classes/Contract/Gameplay/CostQuery.h

#ifndef CONTRACT_GAMEPLAY_COST_QUERY_H_
#define CONTRACT_GAMEPLAY_COST_QUERY_H_

#include "Core/GameConstants.h"

/**
 * @brief Resource Cost Structure
 */
struct ResourceCost {
    int gold = 0;
    int elixir = 0;
    int population = 0;
    float time_seconds = 0;
};

/**
 * @brief Cost Query Service (Singleton)
 *
 * Responsibilities:
 * 1. Provide building placement cost query.
 * 2. Provide building upgrade cost query.
 * 3. Provide troop training cost query.
 * 4. Provide construction/training time query.
 *
 * Note: Max level is unified at Level 3.
 */
class CostQuery {
public:
    static CostQuery* GetInstance();

    // =========================================================================
    // Building Cost Query
    // =========================================================================

    ResourceCost GetBuildingPlacementCost(Core::BuildingType type, int level = 1) const;
    ResourceCost GetBuildingUpgradeCost(Core::BuildingType type, int current_level) const;
    float GetBuildingConstructionTime(Core::BuildingType type, int level) const;
    int GetBuildingMaxLevel(Core::BuildingType type) const;

    // =========================================================================
    // Troop Cost Query
    // =========================================================================

    ResourceCost GetTroopTrainingCost(Core::TroopType type, int level = 1) const;
    float GetTroopTrainingTime(Core::TroopType type, int level) const;
    int GetTroopHousingSpace(Core::TroopType type, int level) const;
    int GetTroopMaxLevel(Core::TroopType type) const;

    ResourceCost GetMatchmakingCost() const;

private:
    CostQuery() = default;

    int GetBaseBuildingCost(Core::BuildingType type) const;
    float GetBaseBuildingTime(Core::BuildingType type) const;
    int GetBaseTroopCost(Core::TroopType type) const;
    float GetBaseTroopTime(Core::TroopType type) const;
};

#endif // CONTRACT_GAMEPLAY_COST_QUERY_H_
