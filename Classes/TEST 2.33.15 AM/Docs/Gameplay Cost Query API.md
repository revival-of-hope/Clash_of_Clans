# Gameplay Cost Query API (Dev C)

This document defines the canonical API for UI cost queries. It does **not** spend resources; it only exposes authoritative costs.

## 1. CostQuery

**Header:** `Classes/Contracts/Gameplay/CostQuery.h`

### 1.1 ResourceCost

```cpp
struct ResourceCost {
    int gold = 0;
    int elixir = 0;
    int population = 0;
};
```

**Semantics:**
* Values are **non-negative**.
* `population` represents housing space required (0 when not applicable).

### 1.2 Interface

```cpp
class CostQuery {
public:
    static CostQuery* GetInstance();

    ResourceCost GetBuildingPlacementCost(Core::BuildingType type, int level) const;
    ResourceCost GetTroopTrainingCost(Core::TroopType type, int level) const;
};
```

**Semantics:**
* `GetBuildingPlacementCost(...)` returns the resource cost to place/build a building at the specified level.
* `GetTroopTrainingCost(...)` returns the resource + population cost to train/deploy one troop of the specified level.
* Functions are **read-only** and do not mutate state.
* If an invalid `level` is provided, the implementation may clamp to level 1.

### 1.3 Usage Example

```cpp
auto cost_query = CostQuery::GetInstance();
auto cannon_cost = cost_query->GetBuildingPlacementCost(Core::BuildingType::kCannon, 1);
auto barb_cost = cost_query->GetTroopTrainingCost(Core::TroopType::kBarbarian, 1);

// UI shows: cannon_cost.gold / cannon_cost.elixir, barb_cost.population
```

