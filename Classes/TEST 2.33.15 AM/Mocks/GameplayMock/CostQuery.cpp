#include "Classes/Contract/Gameplay/CostQuery.h"

namespace {

ResourceCost MakeCost(int gold, int elixir, int population) {
    ResourceCost cost;
    cost.gold = gold;
    cost.elixir = elixir;
    cost.population = population;
    return cost;
}

int ClampLevel(int level) {
    return level > 0 ? level : 1;
}

}  // namespace

CostQuery* CostQuery::GetInstance() {
    static CostQuery instance;
    return &instance;
}

ResourceCost CostQuery::GetBuildingPlacementCost(Core::BuildingType /*type*/, int level) const {
    int clamped = ClampLevel(level);
    return MakeCost(100 * clamped, 50 * clamped, 0);
}

ResourceCost CostQuery::GetTroopTrainingCost(Core::TroopType /*type*/, int level) const {
    int clamped = ClampLevel(level);
    return MakeCost(0, 25 * clamped, 1);
}
