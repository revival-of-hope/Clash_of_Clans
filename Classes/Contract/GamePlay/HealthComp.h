// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Component to manage Health and display a Health Bar.
//
// Path: Classes/Contract/GamePlay/HealthComp.h

#ifndef CONTRACT_GAMEPLAY_HEALTH_COMP_H_
#define CONTRACT_GAMEPLAY_HEALTH_COMP_H_

#include "cocos2d.h"

/**
 * @brief 生命值组件
 * 挂载在 Unit 或 Building 上，负责管理血量、判断死亡和绘制简易血条。
 */
class HealthComp : public cocos2d::Node {
public:
    CREATE_FUNC(HealthComp);

    bool init() override;

    /**
     * @brief 初始化数值
     * @param max_hp 最大血量
     */
    void InitStats(int max_hp);

    /**
     * @brief 承受伤害
     * @param amount 伤害数值
     * @return bool 如果本次伤害导致死亡，返回 true
     */
    bool TakeDamage(int amount);

    /**
     * @brief 治疗
     * @param amount 治疗量
     */
    void Heal(int amount);

    // 状态查询
    bool IsDead() const { return is_dead_; }
    float GetHealthPercentage() const;
    float GetCurrentHealth() const { return current_hp_; }
    float GetMaxHealth() const { return max_hp_; }

    /**
     * @brief 设置血条位置
     * @param offset 相对于父节点中心的偏移量
     */
    void SetHealthBarOffset(cocos2d::Vec2 offset);

protected:
    void UpdateHealthBar();

private:
    float current_hp_ = 100.0f;
    float max_hp_ = 100.0f;
    bool is_dead_ = false;

    cocos2d::DrawNode* health_bar_node_ = nullptr;
    cocos2d::Vec2 bar_offset_;
};

#endif // CONTRACT_GAMEPLAY_HEALTH_COMP_H_