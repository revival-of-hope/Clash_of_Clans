#include "Classes/Contract/Gameplay/CostQuery.h"

#include <algorithm>

namespace {

int ClampLevel(int level) {
    return level > 0 ? level : 1;
}

ResourceCost MakeCost(int gold, int elixir, int population, float time_seconds) {
    ResourceCost cost{};
    cost.gold = gold;
    cost.elixir = elixir;
    cost.population = population;
    cost.time_seconds = time_seconds;
    return cost;
}

}  // namespace

CostQuery* CostQuery::GetInstance() {
    static CostQuery instance;
    return &instance;
}

ResourceCost CostQuery::GetBuildingPlacementCost(Core::BuildingType type, int level) const {
    int clamped_level = ClampLevel(level);
    int gold = 100 * clamped_level;
    if (type == Core::BuildingType::kGoldMine) {
        return MakeCost(gold, 0, 0, 10.0f * clamped_level);
    }
    return MakeCost(gold, gold / 2, 0, 15.0f * clamped_level);
}

ResourceCost CostQuery::GetBuildingUpgradeCost(Core::BuildingType type, int current_level) const {
    int next_level = ClampLevel(current_level + 1);
    int gold = 150 * next_level;
    if (type == Core::BuildingType::kCannon) {
        gold += 50;
    }
    return MakeCost(gold, gold / 2, 0, 20.0f * next_level);
}

float CostQuery::GetBuildingConstructionTime(Core::BuildingType /*type*/, int level) const {
    return 20.0f * ClampLevel(level);
}

int CostQuery::GetBuildingMaxLevel(Core::BuildingType /*type*/) const {
    return 3;
}

ResourceCost CostQuery::GetTroopTrainingCost(Core::TroopType /*type*/, int level) const {
    int clamped_level = ClampLevel(level);
    return MakeCost(0, 25 * clamped_level, 1, 5.0f * clamped_level);
}

float CostQuery::GetTroopTrainingTime(Core::TroopType /*type*/, int level) const {
    return 5.0f * ClampLevel(level);
}

int CostQuery::GetTroopHousingSpace(Core::TroopType /*type*/, int level) const {
    return std::max(1, ClampLevel(level));
}

int CostQuery::GetTroopMaxLevel(Core::TroopType /*type*/) const {
    return 3;
}

ResourceCost CostQuery::GetMatchmakingCost() const {
    return MakeCost(50, 0, 0, 0.0f);
}

int CostQuery::GetBaseBuildingCost(Core::BuildingType /*type*/) const {
    return 100;
}

float CostQuery::GetBaseBuildingTime(Core::BuildingType /*type*/) const {
    return 10.0f;
}

int CostQuery::GetBaseTroopCost(Core::TroopType /*type*/) const {
    return 25;
}

float CostQuery::GetBaseTroopTime(Core::TroopType /*type*/) const {
    return 5.0f;
}
