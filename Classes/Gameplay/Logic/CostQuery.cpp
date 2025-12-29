// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of CostQuery.
// [UPDATE] Added TownHall level restriction system
//
// Path: Classes/Gameplay/Logic/CostQuery.cpp

#include "Contract/GamePlay/CostQuery.h"
#include "cocos2d.h"

CostQuery* CostQuery::GetInstance() {
    static CostQuery instance;
    return &instance;
}

// =============================================================================
// 费用查询
// =============================================================================

ResourceCost CostQuery::GetBuildingPlacementCost(Core::BuildingType type, int level) const {
    ResourceCost cost;

    // 基础费用表 (level 1)
    switch (type) {
    case Core::BuildingType::kTownHall:
        cost.gold = 0;  // 初始免费
        break;
    case Core::BuildingType::kCannon:
        cost.gold = 250;
        break;
    case Core::BuildingType::kArcherTower:
        cost.gold = 1000;
        break;
    case Core::BuildingType::kWall:
        cost.gold = 50;
        break;
    case Core::BuildingType::kGoldMine:
        cost.elixir = 150;
        break;
    case Core::BuildingType::kElixirCollector:
        cost.gold = 150;
        break;
    case Core::BuildingType::kGoldStorage:
        cost.elixir = 300;
        break;
    case Core::BuildingType::kElixirStorage:
        cost.gold = 300;
        break;
    case Core::BuildingType::kBarracks:
        cost.elixir = 200;
        break;
    case Core::BuildingType::kArmyCamp:
        cost.elixir = 250;
        break;
    case Core::BuildingType::kAirDefense:
        cost.gold = 1500;
        break;
    default:
        cost.gold = 100;
        break;
    }

    return cost;
}

ResourceCost CostQuery::GetBuildingUpgradeCost(Core::BuildingType type, int current_level) const {
    ResourceCost cost = GetBuildingPlacementCost(type, 1);

    // 升级费用 = 基础费用 * 等级倍率
    float multiplier = 1.0f + (current_level * 0.8f);
    cost.gold = static_cast<int>(cost.gold * multiplier);
    cost.elixir = static_cast<int>(cost.elixir * multiplier);

    return cost;
}

ResourceCost CostQuery::GetTroopTrainingCost(Core::TroopType type, int level) const {
    ResourceCost cost;

    switch (type) {
    case Core::TroopType::kBarbarian:
        cost.elixir = 25;
        cost.population = 1;
        break;
    case Core::TroopType::kArcher:
        cost.elixir = 50;
        cost.population = 1;
        break;
    case Core::TroopType::kGiant:
        cost.elixir = 250;
        cost.population = 5;
        break;
    case Core::TroopType::kWallBreaker:
        cost.elixir = 50;
        cost.population = 2;
        break;
    case Core::TroopType::kBabyDragon:
        cost.elixir = 500;
        cost.population = 10;
        break;
    default:
        cost.elixir = 50;
        cost.population = 1;
        break;
    }

    // 等级加成
    cost.elixir = static_cast<int>(cost.elixir * (1.0f + (level - 1) * 0.1f));

    return cost;
}

// =============================================================================
// 大本营等级限制
// =============================================================================

/**
 * 大本营解锁表 (最高3级):
 *
 * TH1: TownHall, GoldMine, ElixirCollector, GoldStorage, ElixirStorage, Barracks, ArmyCamp, Cannon
 * TH2: +ArcherTower, +Wall
 * TH3: +AirDefense
 */

int CostQuery::GetRequiredTownHallLevel(Core::BuildingType building_type) const {
    switch (building_type) {
        // TH1 解锁
    case Core::BuildingType::kTownHall:
    case Core::BuildingType::kGoldMine:
    case Core::BuildingType::kElixirCollector:
    case Core::BuildingType::kGoldStorage:
    case Core::BuildingType::kElixirStorage:
    case Core::BuildingType::kBarracks:
    case Core::BuildingType::kArmyCamp:
    case Core::BuildingType::kCannon:
        return 1;

        // TH2 解锁
    case Core::BuildingType::kArcherTower:
    case Core::BuildingType::kWall:
        return 2;

        // TH3 解锁
    case Core::BuildingType::kAirDefense:
        return 3;

    default:
        return 1;
    }
}

bool CostQuery::CanUnlockBuilding(int townhall_level, Core::BuildingType building_type) const {
    int required = GetRequiredTownHallLevel(building_type);
    return townhall_level >= required;
}

int CostQuery::GetMaxBuildingLevel(int townhall_level, Core::BuildingType building_type) const {
    // 大本营自身等级限制 (最高3级)
    if (building_type == Core::BuildingType::kTownHall) {
        return 3;
    }

    // 如果该建筑未解锁，返回0
    if (!CanUnlockBuilding(townhall_level, building_type)) {
        return 0;
    }

    // 所有建筑最高3级
    // 建筑最大等级 = min(大本营等级, 3)
    switch (building_type) {
    case Core::BuildingType::kCannon:
    case Core::BuildingType::kArcherTower:
    case Core::BuildingType::kAirDefense:
        // 防御建筑: 最大等级 = 大本营等级，上限3
        return std::min(townhall_level, 3);

    case Core::BuildingType::kWall:
        // 城墙: 最大等级 = 大本营等级，上限3
        return std::min(townhall_level, 3);

    case Core::BuildingType::kGoldMine:
    case Core::BuildingType::kElixirCollector:
        // 资源建筑: 最大等级 = 大本营等级，上限3
        return std::min(townhall_level, 3);

    case Core::BuildingType::kGoldStorage:
    case Core::BuildingType::kElixirStorage:
        // 存储建筑: 最大等级 = 大本营等级，上限3
        return std::min(townhall_level, 3);

    case Core::BuildingType::kBarracks:
    case Core::BuildingType::kArmyCamp:
        // 军事建筑: 最大等级 = 大本营等级，上限3
        return std::min(townhall_level, 3);

    default:
        return std::min(townhall_level, 3);
    }
}

int CostQuery::GetMaxBuildingCount(int townhall_level, Core::BuildingType building_type) const {
    // 如果该建筑未解锁，返回0
    if (!CanUnlockBuilding(townhall_level, building_type)) {
        return 0;
    }

    // 大本营最高3级，调整数量限制
    switch (building_type) {
    case Core::BuildingType::kTownHall:
        return 1;  // 只能有一个大本营

    case Core::BuildingType::kCannon:
        // TH1:2, TH2:3, TH3:4
        return townhall_level + 1;

    case Core::BuildingType::kArcherTower:
        // TH2:1, TH3:2
        return townhall_level - 1;

    case Core::BuildingType::kWall:
        // TH2:25, TH3:50
        return (townhall_level - 1) * 25;

    case Core::BuildingType::kGoldMine:
    case Core::BuildingType::kElixirCollector:
        // TH1:1, TH2:2, TH3:3
        return townhall_level;

    case Core::BuildingType::kGoldStorage:
    case Core::BuildingType::kElixirStorage:
        // TH1:1, TH2:1, TH3:2
        return (townhall_level + 1) / 2;

    case Core::BuildingType::kBarracks:
        // TH1:1, TH2:1, TH3:2
        return (townhall_level + 1) / 2;

    case Core::BuildingType::kArmyCamp:
        // TH1:1, TH2:2, TH3:2
        return std::min(townhall_level, 2);

    case Core::BuildingType::kAirDefense:
        // TH3 解锁后只有1个 (调整为TH3解锁)
        return 1;

    default:
        return 1;
    }
}

std::vector<Core::BuildingType> CostQuery::GetUnlockedBuildings(int townhall_level) const {
    std::vector<Core::BuildingType> result;

    // 遍历所有建筑类型
    const Core::BuildingType all_types[] = {
        Core::BuildingType::kTownHall,
        Core::BuildingType::kCannon,
        Core::BuildingType::kArcherTower,
        Core::BuildingType::kWall,
        Core::BuildingType::kGoldMine,
        Core::BuildingType::kElixirCollector,
        Core::BuildingType::kGoldStorage,
        Core::BuildingType::kElixirStorage,
        Core::BuildingType::kBarracks,
        Core::BuildingType::kArmyCamp,
        Core::BuildingType::kAirDefense
    };

    for (auto type : all_types) {
        if (CanUnlockBuilding(townhall_level, type)) {
            result.push_back(type);
        }
    }

    return result;
}
