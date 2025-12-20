// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of EconomySystem.

#include "Gameplay/Public/EconomySystem.h"
#include "Gameplay/Public/Building.h"
#include <algorithm> // for std::min

EconomySystem::EconomySystem() {
    Reset();
}

EconomySystem* EconomySystem::GetInstance() {
    static EconomySystem instance;
    return &instance;
}

void EconomySystem::Reset() {
    // 初始赠送一点资源，防止开局卡死
    current_gold_ = 100;
    current_elixir_ = 100;

    // 基础上限 (大本营自带的容量)
    // 在 RecalculateLimits 中会被覆盖累加
    max_gold_ = 1000;
    max_elixir_ = 1000;

    current_population_ = 0;
    max_population_ = 0;
}

// -----------------------------------------------------------------------------
// 资源管理
// -----------------------------------------------------------------------------

void EconomySystem::AddGold(int amount) {
    current_gold_ += amount;
    if (current_gold_ > max_gold_) {
        current_gold_ = max_gold_; // 爆仓截断
    }
    cocos2d::log("Economy: Gold +%d -> %d/%d", amount, current_gold_, max_gold_);
}

void EconomySystem::AddElixir(int amount) {
    current_elixir_ += amount;
    if (current_elixir_ > max_elixir_) {
        current_elixir_ = max_elixir_; // 爆仓截断
    }
    cocos2d::log("Economy: Elixir +%d -> %d/%d", amount, current_elixir_, max_elixir_);
}

bool EconomySystem::SpendGold(int amount) {
    if (current_gold_ >= amount) {
        current_gold_ -= amount;
        cocos2d::log("Economy: Gold -%d -> %d/%d", amount, current_gold_, max_gold_);
        return true;
    }
    cocos2d::log("Economy: Not enough Gold! Need %d, have %d", amount, current_gold_);
    return false;
}

bool EconomySystem::SpendElixir(int amount) {
    if (current_elixir_ >= amount) {
        current_elixir_ -= amount;
        cocos2d::log("Economy: Elixir -%d -> %d/%d", amount, current_elixir_, max_elixir_);
        return true;
    }
    cocos2d::log("Economy: Not enough Elixir! Need %d, have %d", amount, current_elixir_);
    return false;
}

bool EconomySystem::CanAfford(int gold_cost, int elixir_cost) const {
    return (current_gold_ >= gold_cost) && (current_elixir_ >= elixir_cost);
}

// -----------------------------------------------------------------------------
// 收集逻辑
// -----------------------------------------------------------------------------

int EconomySystem::TryCollectResource(Building* building) {
    if (!building) return 0;

    Core::BuildingType type = building->GetBuildingType();
    int stored = building->GetStoredResource();

    // 如果没东西可收，直接跳过
    if (stored <= 0) return 0;

    int collected = 0;

    // 1. 金矿 -> 收金币
    if (type == Core::BuildingType::kGoldMine) {
        // 先计算剩余空间
        int space = max_gold_ - current_gold_;

        if (space <= 0) {
            cocos2d::log("Economy: Gold Storage Full!");
            return 0; // 满了，不收
        }

        // 智能收集：只收取 (库存量) 和 (剩余空间) 两者中的较小值
        // 传入 space 告诉建筑：我只能装下这么多
        collected = building->CollectResource(space);
        AddGold(collected);
    }
    // 2. 圣水收集器 -> 收圣水
    else if (type == Core::BuildingType::kElixirCollector) {
        // 先计算剩余空间
        int space = max_elixir_ - current_elixir_;

        if (space <= 0) {
            cocos2d::log("Economy: Elixir Storage Full!");
            return 0;
        }

        // 智能收集
        collected = building->CollectResource(space);
        AddElixir(collected);
    }

    return collected;
}

// -----------------------------------------------------------------------------
// 人口管理
// -----------------------------------------------------------------------------

bool EconomySystem::AddTroopPopulation(int housing_space) {
    if (current_population_ + housing_space <= max_population_) {
        current_population_ += housing_space;
        cocos2d::log("Economy: Population +%d -> %d/%d", housing_space, current_population_, max_population_);
        return true;
    }
    cocos2d::log("Economy: Not enough Housing Space!");
    return false;
}

void EconomySystem::FreeTroopPopulation(int housing_space) {
    current_population_ -= housing_space;
    if (current_population_ < 0) current_population_ = 0;
    cocos2d::log("Economy: Population Freed -%d -> %d/%d", housing_space, current_population_, max_population_);
}

// -----------------------------------------------------------------------------
// 上限动态计算
// -----------------------------------------------------------------------------

void EconomySystem::RecalculateLimits(const cocos2d::Vector<Building*>& /*ignored_arg*/) {
    // 1. 重置为大本营基础值
    // (假设大本营自带 1000 存储)
    int total_gold_cap = 1000;
    int total_elixir_cap = 1000;
    int total_pop_cap = 0;

    // [修复] 使用 BaseEntity::GetAllEntities() 代替传入的参数
    // 解决了所有权边界问题：不再依赖 UI/Engine 传递列表，而是 GamePlay 拥有权威列表。
    auto& all_entities = BaseEntity::GetAllEntities();

    // 2. 遍历所有建筑，累加 Capacity
    for (auto node : all_entities) {
        Building* b = dynamic_cast<Building*>(node);
        if (!b) continue;

        // 必须是玩家自己的建筑才算容量
        if (b->get_owner_id() != 0) continue;

        // 正在建造/升级的建筑不提供功能 (不增加容量/人口)
        if (b->IsConstructing()) continue;

        //  正确获取建筑等级
        int level = b->GetLevel();
        auto stats = Core::GameConfig::GetInstance()->GetBuildingStats(b->GetBuildingType(), level);

        if (b->GetBuildingType() == Core::BuildingType::kGoldStorage) {
            total_gold_cap += stats.resource_capacity_;
        }
        else if (b->GetBuildingType() == Core::BuildingType::kElixirStorage) {
            total_elixir_cap += stats.resource_capacity_;
        }
        else if (b->GetBuildingType() == Core::BuildingType::kArmyCamp) {
            total_pop_cap += stats.troop_capacity_;
        }
        else if (b->GetBuildingType() == Core::BuildingType::kTownHall) {
            total_gold_cap += stats.resource_capacity_;
            total_elixir_cap += stats.resource_capacity_;
        }
    }

    // 3. 应用新上限
    max_gold_ = total_gold_cap;
    max_elixir_ = total_elixir_cap;
    max_population_ = total_pop_cap;

    // 4. 再次截断当前资源 (防止拆了仓库后资源溢出)
    if (current_gold_ > max_gold_) current_gold_ = max_gold_;
    if (current_elixir_ > max_elixir_) current_elixir_ = max_elixir_;

    cocos2d::log("Economy: Limits Updated. GoldCap: %d, ElixirCap: %d, PopCap: %d",
        max_gold_, max_elixir_, max_population_);
}