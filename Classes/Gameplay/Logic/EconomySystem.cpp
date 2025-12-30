// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of EconomySystem.
// [UPDATE] Added GameEvents broadcasting for resource changes
//
// Path: Classes/Gameplay/Logic/EconomySystem.cpp

#include "Contract/GamePlay/EconomySystem.h"
#include "Contract/GamePlay/Building.h"
#include "Contract/GamePlay/CostQuery.h"
#include "Contract/GamePlay/GameEvents.h"  // [NEW] 事件系统
#include "Core/GameConfig.h"
#include <algorithm>

EconomySystem::EconomySystem() {
    Reset();
}

EconomySystem* EconomySystem::GetInstance() {
    static EconomySystem instance;
    return &instance;
}

void EconomySystem::Reset() {
    current_gold_ = 100;
    current_elixir_ = 100;
    max_gold_ = 1000;
    max_elixir_ = 1000;
    current_population_ = 0;
    max_population_ = 0;
}

// =============================================================================
// 资源操作 - 带事件广播
// =============================================================================

void EconomySystem::AddGold(int amount) {
    int old_val = current_gold_;
    current_gold_ += amount;
    if (current_gold_ > max_gold_) {
        current_gold_ = max_gold_;
    }
    
    int actual_change = current_gold_ - old_val;
    cocos2d::log("Economy: Gold +%d -> %d/%d", amount, current_gold_, max_gold_);
    
    // [EVENT] 广播资源变化
    Gameplay::ResourceUpdateEvent evt;
    evt.resource_type = "Gold";
    evt.current_amount = current_gold_;
    evt.max_capacity = max_gold_;
    evt.change_amount = actual_change;
    Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
}

void EconomySystem::AddElixir(int amount) {
    int old_val = current_elixir_;
    current_elixir_ += amount;
    if (current_elixir_ > max_elixir_) {
        current_elixir_ = max_elixir_;
    }
    
    int actual_change = current_elixir_ - old_val;
    cocos2d::log("Economy: Elixir +%d -> %d/%d", amount, current_elixir_, max_elixir_);
    
    // [EVENT] 广播资源变化
    Gameplay::ResourceUpdateEvent evt;
    evt.resource_type = "Elixir";
    evt.current_amount = current_elixir_;
    evt.max_capacity = max_elixir_;
    evt.change_amount = actual_change;
    Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
}

bool EconomySystem::SpendGold(int amount) {
    if (current_gold_ >= amount) {
        current_gold_ -= amount;
        cocos2d::log("Economy: Gold -%d -> %d/%d", amount, current_gold_, max_gold_);
        
        // [EVENT] 广播资源变化
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Gold";
        evt.current_amount = current_gold_;
        evt.max_capacity = max_gold_;
        evt.change_amount = -amount;  // 负数表示消耗
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
        
        return true;
    }
    cocos2d::log("Economy: Not enough Gold! Need %d, have %d", amount, current_gold_);
    return false;
}

bool EconomySystem::SpendElixir(int amount) {
    if (current_elixir_ >= amount) {
        current_elixir_ -= amount;
        cocos2d::log("Economy: Elixir -%d -> %d/%d", amount, current_elixir_, max_elixir_);
        
        // [EVENT] 广播资源变化
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Elixir";
        evt.current_amount = current_elixir_;
        evt.max_capacity = max_elixir_;
        evt.change_amount = -amount;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
        
        return true;
    }
    cocos2d::log("Economy: Not enough Elixir! Need %d, have %d", amount, current_elixir_);
    return false;
}

bool EconomySystem::SpendCost(const ResourceCost& cost) {
    if (!CanAffordCost(cost, false)) {
        return false;
    }
    
    // 分别扣除，各自会触发事件
    if (cost.gold > 0) {
        current_gold_ -= cost.gold;
        
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Gold";
        evt.current_amount = current_gold_;
        evt.max_capacity = max_gold_;
        evt.change_amount = -cost.gold;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
        
        cocos2d::log("Economy: Gold -%d -> %d/%d", cost.gold, current_gold_, max_gold_);
    }
    if (cost.elixir > 0) {
        current_elixir_ -= cost.elixir;
        
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Elixir";
        evt.current_amount = current_elixir_;
        evt.max_capacity = max_elixir_;
        evt.change_amount = -cost.elixir;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
        
        cocos2d::log("Economy: Elixir -%d -> %d/%d", cost.elixir, current_elixir_, max_elixir_);
    }
    
    return true;
}

// =============================================================================
// 支付能力检查 (不触发事件)
// =============================================================================

bool EconomySystem::CanAfford(int gold_cost, int elixir_cost) const {
    return (current_gold_ >= gold_cost) && (current_elixir_ >= elixir_cost);
}

bool EconomySystem::CanAffordCost(const ResourceCost& cost, bool check_population) const {
    if (current_gold_ < cost.gold) return false;
    if (current_elixir_ < cost.elixir) return false;
    
    if (check_population && cost.population > 0) {
        if (!HasPopulationSpace(cost.population)) return false;
    }
    
    return true;
}

bool EconomySystem::CanAffordBuilding(Core::BuildingType type, int level) const {
    ResourceCost cost = CostQuery::GetInstance()->GetBuildingPlacementCost(type, level);
    return CanAffordCost(cost, false);
}

bool EconomySystem::CanAffordBuildingUpgrade(Core::BuildingType type, int current_level) const {
    ResourceCost cost = CostQuery::GetInstance()->GetBuildingUpgradeCost(type, current_level);
    return CanAffordCost(cost, false);
}

bool EconomySystem::CanAffordTroop(Core::TroopType type, int level) const {
    ResourceCost cost = CostQuery::GetInstance()->GetTroopTrainingCost(type, level);
    return CanAffordCost(cost, true);
}

bool EconomySystem::HasPopulationSpace(int housing_space) const {
    return (current_population_ + housing_space) <= max_population_;
}

// =============================================================================
// 收集逻辑
// =============================================================================

int EconomySystem::TryCollectResource(Building* building) {
    if (!building) return 0;

    Core::BuildingType type = building->GetBuildingType();
    int stored = building->GetStoredResource();

    if (stored <= 0) return 0;

    int collected = 0;

    if (type == Core::BuildingType::kGoldMine) {
        int space = max_gold_ - current_gold_;
        if (space <= 0) {
            cocos2d::log("Economy: Gold Storage Full!");
            return 0;
        }
        collected = building->CollectResource(space);
        AddGold(collected);  // 会触发事件
    }
    else if (type == Core::BuildingType::kElixirCollector) {
        int space = max_elixir_ - current_elixir_;
        if (space <= 0) {
            cocos2d::log("Economy: Elixir Storage Full!");
            return 0;
        }
        collected = building->CollectResource(space);
        AddElixir(collected);  // 会触发事件
    }

    return collected;
}

// =============================================================================
// 人口管理 - 带事件广播
// =============================================================================

bool EconomySystem::AddTroopPopulation(int housing_space) {
    if (HasPopulationSpace(housing_space)) {
        current_population_ += housing_space;
        cocos2d::log("Economy: Population +%d -> %d/%d", 
                     housing_space, current_population_, max_population_);
        
        // [EVENT] 广播人口变化
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Population";
        evt.current_amount = current_population_;
        evt.max_capacity = max_population_;
        evt.change_amount = housing_space;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
        
        return true;
    }
    cocos2d::log("Economy: Not enough Housing Space! Need %d, have %d free", 
                 housing_space, GetRemainingPopulation());
    return false;
}

void EconomySystem::FreeTroopPopulation(int housing_space) {
    current_population_ -= housing_space;
    if (current_population_ < 0) current_population_ = 0;
    cocos2d::log("Economy: Population Freed -%d -> %d/%d", 
                 housing_space, current_population_, max_population_);
    
    // [EVENT] 广播人口变化
    Gameplay::ResourceUpdateEvent evt;
    evt.resource_type = "Population";
    evt.current_amount = current_population_;
    evt.max_capacity = max_population_;
    evt.change_amount = -housing_space;
    Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
}

// =============================================================================
// 上限动态计算
// =============================================================================

void EconomySystem::RecalculateLimits(const cocos2d::Vector<Building*>& /*ignored_arg*/) {
    int total_gold_cap = 1000;
    int total_elixir_cap = 1000;
    int total_pop_cap = 0;

    auto& all_entities = BaseEntity::GetAllEntities();

    for (auto node : all_entities) {
        Building* b = dynamic_cast<Building*>(node);
        if (!b) continue;

        if (b->get_owner_id() != 0) continue;
        if (b->IsConstructing()) continue;

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

    bool gold_changed = (max_gold_ != total_gold_cap);
    bool elixir_changed = (max_elixir_ != total_elixir_cap);
    bool pop_changed = (max_population_ != total_pop_cap);

    max_gold_ = total_gold_cap;
    max_elixir_ = total_elixir_cap;
    max_population_ = total_pop_cap;

    if (current_gold_ > max_gold_) current_gold_ = max_gold_;
    if (current_elixir_ > max_elixir_) current_elixir_ = max_elixir_;

    cocos2d::log("Economy: Limits Updated. GoldCap: %d, ElixirCap: %d, PopCap: %d",
                 max_gold_, max_elixir_, max_population_);

    // [EVENT] 广播容量变化 (change_amount = 0 表示仅容量变化)
    if (gold_changed) {
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Gold";
        evt.current_amount = current_gold_;
        evt.max_capacity = max_gold_;
        evt.change_amount = 0;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
    }
    if (elixir_changed) {
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Elixir";
        evt.current_amount = current_elixir_;
        evt.max_capacity = max_elixir_;
        evt.change_amount = 0;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
    }
    if (pop_changed) {
        Gameplay::ResourceUpdateEvent evt;
        evt.resource_type = "Population";
        evt.current_amount = current_population_;
        evt.max_capacity = max_population_;
        evt.change_amount = 0;
        Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
    }
}
