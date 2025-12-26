// Source: Classes/Integration/Docs/Gameplay Cost Query API.md (CostQuery)
#ifndef CONTRACTS_GAMEPLAY_COSTQUERY_H_
#define CONTRACTS_GAMEPLAY_COSTQUERY_H_

#include "Core/GameConstants.h"

struct ResourceCost {
    int gold = 0;
    int elixir = 0;
    int population = 0;
};

class CostQuery {
public:
    static CostQuery* GetInstance();

    ResourceCost GetBuildingPlacementCost(Core::BuildingType type, int level) const;
    ResourceCost GetTroopTrainingCost(Core::TroopType type, int level) const;
};

#endif  // CONTRACTS_GAMEPLAY_COSTQUERY_H_
