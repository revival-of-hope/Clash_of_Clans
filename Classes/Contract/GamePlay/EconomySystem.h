// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Singleton for managing global resources (Gold, Elixir, Population).

#ifndef GAMEPLAY_LOGIC_ECONOMY_SYSTEM_H_
#define GAMEPLAY_LOGIC_ECONOMY_SYSTEM_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include <vector>

// 前向声明
class Building;

/**
 * @brief 经济系统 (单例)
 * 职责:
 * 1. 维护全局资源 (金币, 圣水) 的当前值与最大上限。
 * 2. 维护人口 (Housing Space) 的当前占用与最大上限。
 * 3. 处理资源的 收集(Collect) 和 消费(Spend)。
 * 4. 动态计算上限 (基于场上建筑)。
 */
class EconomySystem {
public:
    static EconomySystem* GetInstance();

    // 初始化数据
    void Reset();

    // 资源操作 (Resource Operations)

    /**
     * @brief 增加金币 (来源: 收集、任务奖励、调试)
     * 会自动执行上限截断 (Clamp to Max)
     */
    void AddGold(int amount);

    /**
     * @brief 增加圣水
     */
    void AddElixir(int amount);

    /**
     * @brief 尝试消费金币 (来源: 造兵、升级)
     * @return true 表示扣款成功; false 表示余额不足
     */
    bool SpendGold(int amount);

    /**
     * @brief 尝试消费圣水
     */
    bool SpendElixir(int amount);

    // 检查是否买得起 (仅检查，不扣款)
    bool CanAfford(int gold_cost, int elixir_cost) const;

    // 收集逻辑 (Collection Logic)

    /**
     * @brief 尝试从建筑收集资源
     * @param building 目标建筑 (金矿/收集器)
     * @return int 实际收集到的数量 (如果库满了则是 0)
     */
    int TryCollectResource(Building* building);

    // 人口管理 (Population / Housing Space)

    /**
     * @brief 增加当前人口占用 (造兵时调用)
     * @return true 成功, false 人口已满
     */
    bool AddTroopPopulation(int housing_space);

    /**
     * @brief 释放人口 (士兵死亡/捐兵时调用)
     */
    void FreeTroopPopulation(int housing_space);

    // 上限计算 (Capacity Calculation)

    /**
     * @brief 重新计算所有资源的上限
     * 触发时机: 游戏加载完毕、储金罐/兵营 建造或升级完成时
     * @param buildings 场上所有建筑的列表
     */
    void RecalculateLimits(const cocos2d::Vector<Building*>& buildings);

    // Getters (UI 用于显示)

    int GetCurrentGold() const { return current_gold_; }
    int GetMaxGold() const { return max_gold_; }

    int GetCurrentElixir() const { return current_elixir_; }
    int GetMaxElixir() const { return max_elixir_; }

    int GetCurrentPopulation() const { return current_population_; }
    int GetMaxPopulation() const { return max_population_; }

private:
    // 构造函数私有化
    EconomySystem();

    int current_gold_;
    int max_gold_;

    int current_elixir_;
    int max_elixir_;

    int current_population_; // 当前已造兵人口
    int max_population_;     // 兵营提供的总上限
};

#endif // GAMEPLAY_LOGIC_ECONOMY_SYSTEM_H_