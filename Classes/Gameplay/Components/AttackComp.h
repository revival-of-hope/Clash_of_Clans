// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// AttackComp - Attack Component for Units and Buildings.
//
// Path: Classes/Gameplay/Components/AttackComp.h

#ifndef GAMEPLAY_COMPONENTS_ATTACK_COMP_H_
#define GAMEPLAY_COMPONENTS_ATTACK_COMP_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"

class BaseEntity;

/**
 * @brief 攻击组件
 */
class AttackComp : public cocos2d::Node {
public:
    CREATE_FUNC(AttackComp);

    virtual bool init() override;
    virtual void update(float dt) override;

    void InitStats(int damage, float range_pixels, float attack_speed,
        Core::ProjectileType projectile_type);

    bool TryAttack(BaseEntity* target);
    bool IsTargetInRange(BaseEntity* target) const;

    // =========================================================================
    // 契约层只读接口
    // =========================================================================

    float GetCooldownTimer() const { return attack_timer_; }
    float GetCooldownDuration() const { return attack_speed_; }

    float GetCooldownPercentage() const {
        if (attack_speed_ <= 0.0f) return 1.0f;
        return 1.0f - (attack_timer_ / attack_speed_);
    }

    bool IsOnCooldown() const { return attack_timer_ > 0.0f; }
    float GetRange() const { return std::sqrt(range_sq_); }
    float GetRangeSq() const { return range_sq_; }
    int GetDamage() const { return damage_; }
    Core::ProjectileType GetProjectileType() const { return projectile_type_; }
    bool IsRangedAttack() const { return projectile_type_ != Core::ProjectileType::kNone; }
    bool IsMeleeAttack() const { return projectile_type_ == Core::ProjectileType::kNone; }

private:
    void DealDamage(BaseEntity* target);

    int damage_;
    float range_sq_;
    float attack_speed_;
    float attack_timer_;
    Core::ProjectileType projectile_type_;
};

#endif // GAMEPLAY_COMPONENTS_ATTACK_COMP_H_