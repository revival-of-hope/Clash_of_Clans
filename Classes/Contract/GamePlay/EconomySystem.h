// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Singleton for managing global resources (Gold, Elixir, Population).
// Contract header for external modules.
//
// Path: Classes/Contract/Gameplay/EconomySystem.h

#ifndef CONTRACT_GAMEPLAY_ECONOMY_SYSTEM_H_
#define CONTRACT_GAMEPLAY_ECONOMY_SYSTEM_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include <vector>

class Building;
struct ResourceCost;

/**
 * @brief Economy System (Singleton)
 *
 * Responsibilities:
 * 1. Maintain global resources (Gold, Elixir) current values and max limits.
 * 2. Maintain Population (Housing Space) current usage and max limits.
 * 3. Handle resource Collection and Spending.
 * 4. Dynamically calculate limits (based on buildings on the field).
 * 5. Provide convenient affordability check methods.
 */
class EconomySystem {
public:
    static EconomySystem* GetInstance();

    void Reset();

    // =========================================================================
    // Resource Operations
    // =========================================================================

    void AddGold(int amount);
    void AddElixir(int amount);
    bool SpendGold(int amount);
    bool SpendElixir(int amount);
    bool SpendCost(const ResourceCost& cost);

    // =========================================================================
    // Affordability Check - Contract Layer
    // =========================================================================

    bool CanAfford(int gold_cost, int elixir_cost) const;
    bool CanAffordCost(const ResourceCost& cost, bool check_population = true) const;
    // Matchmaking affordability: CostQuery::GetMatchmakingCost() -> CanAffordCost(...) + SpendGold(...).
    bool CanAffordBuilding(Core::BuildingType type, int level = 1) const;
    bool CanAffordBuildingUpgrade(Core::BuildingType type, int current_level) const;
    bool CanAffordTroop(Core::TroopType type, int level = 1) const;
    bool HasPopulationSpace(int housing_space) const;

    // =========================================================================
    // Collection Logic
    // =========================================================================

    int TryCollectResource(Building* building);

    // =========================================================================
    // Population Management
    // =========================================================================

    bool AddTroopPopulation(int housing_space);
    void FreeTroopPopulation(int housing_space);
    int GetRemainingPopulation() const {
        return max_population_ - current_population_;
    }

    // =========================================================================
    // Limit Calculation
    // =========================================================================

    void RecalculateLimits(const cocos2d::Vector<Building*>& buildings);

    // =========================================================================
    // Read-only Accessors - Contract Layer
    // =========================================================================

    int GetCurrentGold() const { return current_gold_; }
    int GetMaxGold() const { return max_gold_; }
    int GetCurrentElixir() const { return current_elixir_; }
    int GetMaxElixir() const { return max_elixir_; }
    int GetCurrentPopulation() const { return current_population_; }
    int GetMaxPopulation() const { return max_population_; }

    float GetGoldPercentage() const {
        if (max_gold_ <= 0) return 0.0f;
        return static_cast<float>(current_gold_) / static_cast<float>(max_gold_);
    }

    float GetElixirPercentage() const {
        if (max_elixir_ <= 0) return 0.0f;
        return static_cast<float>(current_elixir_) / static_cast<float>(max_elixir_);
    }

    float GetPopulationPercentage() const {
        if (max_population_ <= 0) return 0.0f;
        return static_cast<float>(current_population_) / static_cast<float>(max_population_);
    }

private:
    EconomySystem();

    int current_gold_;
    int max_gold_;
    int current_elixir_;
    int max_elixir_;
    int current_population_;
    int max_population_;
};

#endif // CONTRACT_GAMEPLAY_ECONOMY_SYSTEM_H_
