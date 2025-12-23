// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Component to handle attack cooldowns and range checks.
// Delegates actual damage dealing to CombatResolver.

#ifndef GAMEPLAY_COMPONENTS_ATTACK_COMP_H_
#define GAMEPLAY_COMPONENTS_ATTACK_COMP_H_

#include "cocos2d.h"
#include "Contract/Gameplay/BaseEntity.h" // 引用 BaseEntity 来定义目标类型
#include "Core/GameConstants.h"

/**
 * @brief 攻击组件
 * 负责管理攻击冷却时间、攻击范围判定以及发起攻击请求。
 */
class AttackComp : public cocos2d::Node {
public:
    CREATE_FUNC(AttackComp);

    virtual bool init() override;
    virtual void update(float dt) override;

    /**
     * @brief 初始化攻击属性
     * @param damage 单次伤害
     * @param range_pixels 攻击范围 (像素)
     * @param attack_speed 攻击间隔 (秒)
     * @param projectile_type 投射物类型 (kNone表示近战)
     */
    void InitStats(int damage, float range_pixels, float attack_speed, Core::ProjectileType projectile_type);

    /**
     * @brief 尝试对目标发起攻击
     * 内部会检查冷却和距离。如果成功，会重置冷却并造成伤害。
     * @param target 目标实体
     * @return true 攻击成功触发
     */
    bool TryAttack(BaseEntity* target);

    // 检查目标是否在射程内 (不考虑冷却)
    bool IsTargetInRange(BaseEntity* target) const;

    // 获取当前剩余冷却时间 (用于 UI 显示或调试)
    float GetCooldownTimer() const { return attack_timer_; }

    // 强制重置冷却 (刚部署时需要等待)
    void ResetCooldown() { attack_timer_ = attack_speed_; }

private:
    int damage_;
    float range_sq_; // 范围的平方，用于高效计算
    float attack_speed_;
    Core::ProjectileType projectile_type_;

    float attack_timer_; // 当前倒计时

    // 实际执行扣血 (调用 CombatResolver)
    void DealDamage(BaseEntity* target);
};

#endif // GAMEPLAY_COMPONENTS_ATTACK_COMP_H_