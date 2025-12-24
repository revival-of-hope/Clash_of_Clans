#include "Contracts/Gameplay/EconomySystem.h"
#include "Contracts/Gameplay/Building.h"

EconomySystem* EconomySystem::GetInstance() {
    static EconomySystem instance;
    return &instance;
}

void EconomySystem::Reset() {
    current_gold_ = 0;
    current_elixir_ = 0;
    current_population_ = 0;
    max_gold_ = 0;
    max_elixir_ = 0;
    max_population_ = 0;
}

bool EconomySystem::SpendGold(int amount) {
    if (amount <= 0) {
        return true;
    }
    if (amount > current_gold_) {
        return false;
    }
    current_gold_ -= amount;
    return true;
}

bool EconomySystem::SpendElixir(int amount) {
    if (amount <= 0) {
        return true;
    }
    if (amount > current_elixir_) {
        return false;
    }
    current_elixir_ -= amount;
    return true;
}

int EconomySystem::TryCollectResource(Building* building) {
    if (!building) {
        return 0;
    }
    int capacity = 0;
    switch (building->GetBuildingType()) {
        case Core::BuildingType::kGoldMine:
            capacity = max_gold_ - current_gold_;
            break;
        case Core::BuildingType::kElixirCollector:
            capacity = max_elixir_ - current_elixir_;
            break;
        default:
            return 0;
    }
    if (capacity <= 0) {
        return 0;
    }
    int collected = building->CollectResource(capacity);
    if (collected < 0) {
        collected = 0;
    }
    if (building->GetBuildingType() == Core::BuildingType::kGoldMine) {
        current_gold_ += collected;
        if (max_gold_ > 0 && current_gold_ > max_gold_) {
            current_gold_ = max_gold_;
        }
    } else if (building->GetBuildingType() == Core::BuildingType::kElixirCollector) {
        current_elixir_ += collected;
        if (max_elixir_ > 0 && current_elixir_ > max_elixir_) {
            current_elixir_ = max_elixir_;
        }
    }
    return collected;
}

void EconomySystem::RecalculateLimits(const cocos2d::Vector<Building*>& buildings) {
    int building_count = static_cast<int>(buildings.size());
    max_gold_ = building_count * 1000;
    max_elixir_ = building_count * 1000;
    max_population_ = building_count * 10;
    if (current_gold_ > max_gold_) {
        current_gold_ = max_gold_;
    }
    if (current_elixir_ > max_elixir_) {
        current_elixir_ = max_elixir_;
    }
    if (current_population_ > max_population_) {
        current_population_ = max_population_;
    }
}

bool EconomySystem::AddTroopPopulation(int space) {
    if (space <= 0) {
        return true;
    }
    if (current_population_ + space > max_population_) {
        return false;
    }
    current_population_ += space;
    return true;
}

void EconomySystem::AddGold(int amount) {
    if (amount <= 0) {
        return;
    }
    current_gold_ += amount;
    if (max_gold_ > 0 && current_gold_ > max_gold_) {
        current_gold_ = max_gold_;
    }
}

void EconomySystem::AddElixir(int amount) {
    if (amount <= 0) {
        return;
    }
    current_elixir_ += amount;
    if (max_elixir_ > 0 && current_elixir_ > max_elixir_) {
        current_elixir_ = max_elixir_;
    }
}

int EconomySystem::GetCurrentGold() const {
    return current_gold_;
}

int EconomySystem::GetMaxGold() const {
    return max_gold_;
}

int EconomySystem::GetCurrentElixir() const {
    return current_elixir_;
}

int EconomySystem::GetMaxElixir() const {
    return max_elixir_;
}

int EconomySystem::GetCurrentPopulation() const {
    return current_population_;
}

int EconomySystem::GetMaxPopulation() const {
    return max_population_;
}
