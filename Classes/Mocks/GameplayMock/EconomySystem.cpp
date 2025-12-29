#include "Classes/Contract/Gameplay/EconomySystem.h"

#include "Classes/Contract/Gameplay/Building.h"
#include "Classes/Contract/Gameplay/CostQuery.h"

#include <algorithm>

EconomySystem* EconomySystem::GetInstance() {
    static EconomySystem instance;
    return &instance;
}

EconomySystem::EconomySystem()
    : current_gold_(0),
      max_gold_(0),
      current_elixir_(0),
      max_elixir_(0),
      current_population_(0),
      max_population_(0) {}

void EconomySystem::Reset() {
    current_gold_ = 0;
    current_elixir_ = 0;
    current_population_ = 0;
    max_gold_ = 0;
    max_elixir_ = 0;
    max_population_ = 0;
}

void EconomySystem::AddGold(int amount) {
    if (amount <= 0) {
        return;
    }
    current_gold_ += amount;
    if (max_gold_ > 0) {
        current_gold_ = std::min(current_gold_, max_gold_);
    }
}

void EconomySystem::AddElixir(int amount) {
    if (amount <= 0) {
        return;
    }
    current_elixir_ += amount;
    if (max_elixir_ > 0) {
        current_elixir_ = std::min(current_elixir_, max_elixir_);
    }
}

bool EconomySystem::SpendGold(int amount) {
    if (amount <= 0) {
        return true;
    }
    if (current_gold_ < amount) {
        return false;
    }
    current_gold_ -= amount;
    return true;
}

bool EconomySystem::SpendElixir(int amount) {
    if (amount <= 0) {
        return true;
    }
    if (current_elixir_ < amount) {
        return false;
    }
    current_elixir_ -= amount;
    return true;
}

bool EconomySystem::SpendCost(const ResourceCost& cost) {
    if (!CanAffordCost(cost)) {
        return false;
    }
    SpendGold(cost.gold);
    SpendElixir(cost.elixir);
    AddTroopPopulation(cost.population);
    return true;
}

bool EconomySystem::CanAfford(int gold_cost, int elixir_cost) const {
    return current_gold_ >= gold_cost && current_elixir_ >= elixir_cost;
}

bool EconomySystem::CanAffordCost(const ResourceCost& cost, bool check_population) const {
    if (!CanAfford(cost.gold, cost.elixir)) {
        return false;
    }
    if (check_population && !HasPopulationSpace(cost.population)) {
        return false;
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
    return current_population_ + housing_space <= max_population_;
}

int EconomySystem::TryCollectResource(Building* building) {
    if (!building) {
        return 0;
    }
    int collected = building->CollectResource(-1);
    if (building->GetBuildingType() == Core::BuildingType::kGoldMine) {
        AddGold(collected);
    } else if (building->GetBuildingType() == Core::BuildingType::kElixirCollector) {
        AddElixir(collected);
    }
    return collected;
}

bool EconomySystem::AddTroopPopulation(int housing_space) {
    if (housing_space <= 0) {
        return true;
    }
    if (!HasPopulationSpace(housing_space)) {
        return false;
    }
    current_population_ += housing_space;
    return true;
}

void EconomySystem::FreeTroopPopulation(int housing_space) {
    if (housing_space <= 0) {
        return;
    }
    current_population_ = std::max(0, current_population_ - housing_space);
}

void EconomySystem::RecalculateLimits(const cocos2d::Vector<Building*>& buildings) {
    int storages = 0;
    for (auto* building : buildings) {
        if (!building) {
            continue;
        }
        if (building->GetBuildingType() == Core::BuildingType::kGoldStorage) {
            ++storages;
        }
        if (building->GetBuildingType() == Core::BuildingType::kElixirStorage) {
            ++storages;
        }
    }
    max_gold_ = std::max(storages, 1) * 1000;
    max_elixir_ = std::max(storages, 1) * 1000;
    max_population_ = std::max(static_cast<int>(buildings.size()), 1) * 10;
    current_gold_ = std::min(current_gold_, max_gold_);
    current_elixir_ = std::min(current_elixir_, max_elixir_);
    current_population_ = std::min(current_population_, max_population_);
}

