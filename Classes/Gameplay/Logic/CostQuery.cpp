// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of CostQuery.
// Max level for all buildings and troops is 3.
//
// Path: Classes/Gameplay/Logic/CostQuery.cpp

#include "Contract/GamePlay/CostQuery.h"
#include "Core/GameConfig.h"

CostQuery* CostQuery::GetInstance() {
    static CostQuery instance;
    return &instance;
}

static constexpr int kMaxLevel = 3;

// =============================================================================
// 建筑成本查询
// =============================================================================

ResourceCost CostQuery::GetBuildingPlacementCost(Core::BuildingType type, int level) const {
    ResourceCost cost;

    if (level < 1) level = 1;
    if (level > kMaxLevel) level = kMaxLevel;

    int base_cost = GetBaseBuildingCost(type);
    float base_time = GetBaseBuildingTime(type);

    float cost_multiplier = 1.0f;
    float time_multiplier = 1.0f;
    switch (level) {
    case 1: cost_multiplier = 1.0f;  time_multiplier = 1.0f;  break;
    case 2: cost_multiplier = 2.0f;  time_multiplier = 1.5f;  break;
    case 3: cost_multiplier = 3.5f;  time_multiplier = 2.0f;  break;
    }

    int final_cost = static_cast<int>(base_cost * cost_multiplier);
    float final_time = base_time * time_multiplier;

    switch (type) {
    case Core::BuildingType::kCannon:
    case Core::BuildingType::kArcherTower:
    case Core::BuildingType::kWall:
    case Core::BuildingType::kGoldStorage:
    case Core::BuildingType::kTownHall:
    case Core::BuildingType::kAirDefense:
        cost.gold = final_cost;
        cost.elixir = 0;
        break;

    case Core::BuildingType::kBarracks:
    case Core::BuildingType::kArmyCamp:
    case Core::BuildingType::kElixirStorage:
    case Core::BuildingType::kElixirCollector:
    case Core::BuildingType::kGoldMine:
        cost.gold = 0;
        cost.elixir = final_cost;
        break;

    default:
        cost.gold = final_cost;
        cost.elixir = 0;
        break;
    }

    cost.time_seconds = final_time;
    cost.population = 0;

    return cost;
}

ResourceCost CostQuery::GetBuildingUpgradeCost(Core::BuildingType type, int current_level) const {
    int next_level = current_level + 1;

    if (next_level > kMaxLevel) {
        return ResourceCost();
    }

    return GetBuildingPlacementCost(type, next_level);
}

float CostQuery::GetBuildingConstructionTime(Core::BuildingType type, int level) const {
    if (level < 1) level = 1;
    if (level > kMaxLevel) level = kMaxLevel;

    float base_time = GetBaseBuildingTime(type);

    float time_multiplier = 1.0f;
    switch (level) {
    case 1: time_multiplier = 1.0f;  break;
    case 2: time_multiplier = 1.5f;  break;
    case 3: time_multiplier = 2.0f;  break;
    }

    return base_time * time_multiplier;
}

int CostQuery::GetBuildingMaxLevel(Core::BuildingType /*type*/) const {
    return kMaxLevel;
}

// =============================================================================
// 兵种成本查询
// =============================================================================

ResourceCost CostQuery::GetTroopTrainingCost(Core::TroopType type, int level) const {
    ResourceCost cost;

    if (level < 1) level = 1;
    if (level > kMaxLevel) level = kMaxLevel;

    int base_cost = GetBaseTroopCost(type);
    float base_time = GetBaseTroopTime(type);

    auto stats = Core::GameConfig::GetInstance()->GetTroopStats(type, level);

    float cost_multiplier = 1.0f;
    float time_multiplier = 1.0f;
    switch (level) {
    case 1: cost_multiplier = 1.0f;  time_multiplier = 1.0f;  break;
    case 2: cost_multiplier = 1.5f;  time_multiplier = 1.2f;  break;
    case 3: cost_multiplier = 2.0f;  time_multiplier = 1.5f;  break;
    }

    cost.gold = 0;
    cost.elixir = static_cast<int>(base_cost * cost_multiplier);
    cost.population = stats.housing_space_;
    cost.time_seconds = base_time * time_multiplier;

    return cost;
}

float CostQuery::GetTroopTrainingTime(Core::TroopType type, int level) const {
    if (level < 1) level = 1;
    if (level > kMaxLevel) level = kMaxLevel;

    float base_time = GetBaseTroopTime(type);

    float time_multiplier = 1.0f;
    switch (level) {
    case 1: time_multiplier = 1.0f;  break;
    case 2: time_multiplier = 1.2f;  break;
    case 3: time_multiplier = 1.5f;  break;
    }

    return base_time * time_multiplier;
}

int CostQuery::GetTroopHousingSpace(Core::TroopType type, int level) const {
    auto stats = Core::GameConfig::GetInstance()->GetTroopStats(type, level);
    return stats.housing_space_;
}

int CostQuery::GetTroopMaxLevel(Core::TroopType /*type*/) const {
    return kMaxLevel;
}

// =============================================================================
// 内部辅助函数
// =============================================================================

int CostQuery::GetBaseBuildingCost(Core::BuildingType type) const {
    switch (type) {
    case Core::BuildingType::kCannon:       return 500;
    case Core::BuildingType::kArcherTower:  return 1000;
    case Core::BuildingType::kAirDefense:   return 2000;
    case Core::BuildingType::kWall:         return 50;
    case Core::BuildingType::kGoldMine:         return 300;
    case Core::BuildingType::kElixirCollector:  return 300;
    case Core::BuildingType::kGoldStorage:      return 500;
    case Core::BuildingType::kElixirStorage:    return 500;
    case Core::BuildingType::kBarracks:     return 400;
    case Core::BuildingType::kArmyCamp:     return 500;
    case Core::BuildingType::kTownHall:     return 5000;
    default: return 100;
    }
}

float CostQuery::GetBaseBuildingTime(Core::BuildingType type) const {
    switch (type) {
    case Core::BuildingType::kCannon:       return 60.0f;
    case Core::BuildingType::kArcherTower:  return 120.0f;
    case Core::BuildingType::kAirDefense:   return 180.0f;
    case Core::BuildingType::kWall:         return 10.0f;
    case Core::BuildingType::kGoldMine:         return 30.0f;
    case Core::BuildingType::kElixirCollector:  return 30.0f;
    case Core::BuildingType::kGoldStorage:      return 60.0f;
    case Core::BuildingType::kElixirStorage:    return 60.0f;
    case Core::BuildingType::kBarracks:     return 90.0f;
    case Core::BuildingType::kArmyCamp:     return 120.0f;
    case Core::BuildingType::kTownHall:     return 300.0f;
    default: return 30.0f;
    }
}

int CostQuery::GetBaseTroopCost(Core::TroopType type) const {
    switch (type) {
    case Core::TroopType::kBarbarian:   return 50;
    case Core::TroopType::kArcher:      return 100;
    case Core::TroopType::kGiant:       return 500;
    case Core::TroopType::kWallBreaker: return 1000;
    case Core::TroopType::kBabyDragon:  return 2000;
    default: return 50;
    }
}

float CostQuery::GetBaseTroopTime(Core::TroopType type) const {
    switch (type) {
    case Core::TroopType::kBarbarian:   return 5.0f;
    case Core::TroopType::kArcher:      return 6.0f;
    case Core::TroopType::kGiant:       return 30.0f;
    case Core::TroopType::kWallBreaker: return 15.0f;
    case Core::TroopType::kBabyDragon:  return 60.0f;
    default: return 10.0f;
    }
}