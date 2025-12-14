// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Component to manage Health and display a Health Bar.

#ifndef GAMEPLAY_COMPONENTS_HEALTH_COMP_H_
#define GAMEPLAY_COMPONENTS_HEALTH_COMP_H_

#include "cocos2d.h"

/**
 * @brief 生命值组件
 * 挂载在 Unit 或 Building 上，负责管理血量、判断死亡和绘制简易血条。
 */
class HealthComp : public cocos2d::Node {
public:
    // 标准 Cocos 创建宏
    CREATE_FUNC(HealthComp);

    // 初始化并设定最大生命值
    bool init() override;

    /**
     * @brief 初始化数值
     * @param max_hp 最大血量
     */
    void InitStats(int max_hp);

    /**
     * @brief 承受伤害
     * @param amount 伤害数值
     * @return bool 如果本次伤害导致死亡，返回 true；否则返回 false
     */
    bool TakeDamage(int amount);

    // 治疗 (预留可能接口)
    void Heal(int amount);

    // 状态查询
    bool IsDead() const { return is_dead_; }
    float GetHealthPercentage() const;

    /**
     * @brief 设置血条位置
     * @param offset 相对于父节点中心的偏移量
     */
    void SetHealthBarOffset(cocos2d::Vec2 offset);

protected:
    // 内部函数：重绘血条
    void UpdateHealthBar();

private:
    float current_hp_;
    float max_hp_;
    bool is_dead_;

    // 使用 DrawNode 绘制简易血条，无需加载图片
    cocos2d::DrawNode* health_bar_node_;

    // 血条偏移量
    cocos2d::Vec2 bar_offset_;
};

#endif // GAMEPLAY_COMPONENTS_HEALTH_COMP_H_