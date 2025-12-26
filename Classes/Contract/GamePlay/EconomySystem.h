// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Singleton for managing global resources (Gold, Elixir, Population).
// Contract header for external modules.
//
// Path: Classes/Contract/GamePlay/EconomySystem.h

#ifndef CONTRACT_GAMEPLAY_ECONOMY_SYSTEM_H_
#define CONTRACT_GAMEPLAY_ECONOMY_SYSTEM_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include <vector>

class Building;
struct ResourceCost;

/**
 * @brief 经济系统 (单例)
 *
 * 职责:
 * 1. 维护全局资源 (金币, 圣水) 的当前值与最大上限
 * 2. 维护人口 (Housing Space) 的当前占用与最大上限
 * 3. 处理资源的 收集(Collect) 和 消费(Spend)
 * 4. 动态计算上限 (基于场上建筑)
 * 5. 提供便捷的支付能力检查方法
 */
class EconomySystem {
public:
    static EconomySystem* GetInstance();

    void Reset();

    // =========================================================================
    // 资源操作
    // =========================================================================

    void AddGold(int amount);
    void AddElixir(int amount);
    bool SpendGold(int amount);
    bool SpendElixir(int amount);
    bool SpendCost(const ResourceCost& cost);

    // =========================================================================
    // 支付能力检查 - 契约层
    // =========================================================================

    bool CanAfford(int gold_cost, int elixir_cost) const;
    bool CanAffordCost(const ResourceCost& cost, bool check_population = true) const;
    bool CanAffordBuilding(Core::BuildingType type, int level = 1) const;
    bool CanAffordBuildingUpgrade(Core::BuildingType type, int current_level) const;
    bool CanAffordTroop(Core::TroopType type, int level = 1) const;
    bool HasPopulationSpace(int housing_space) const;

    // =========================================================================
    // 收集逻辑
    // =========================================================================

    int TryCollectResource(Building* building);

    // =========================================================================
    // 人口管理
    // =========================================================================

    bool AddTroopPopulation(int housing_space);
    void FreeTroopPopulation(int housing_space);
    int GetRemainingPopulation() const {
        return max_population_ - current_population_;
    }

    // =========================================================================
    // 上限计算
    // =========================================================================

    void RecalculateLimits(const cocos2d::Vector<Building*>& buildings);

    // =========================================================================
    // 只读访问器 - 契约层
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