// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of AttackComp.

#include "AttackComp.h"
#include "Gameplay/Public/HealthComp.h"
#include "Gameplay/Public/Building.h"
#include "Core/GameConstants.h"
#include "Gameplay/Public/CombatResolver.h"

bool AttackComp::init() {
    if (!cocos2d::Node::init()) {
        return false;
    }

    // 开启 Update 以便自动减少冷却时间
    this->scheduleUpdate();

    damage_ = 0;
    range_sq_ = 0.0f;
    attack_speed_ = 1.0f;
    attack_timer_ = 0.0f;
    projectile_type_ = Core::ProjectileType::kNone; // 默认近战

    return true;
}

void AttackComp::InitStats(int damage, float range_pixels, float attack_speed, Core::ProjectileType projectile_type) {
    damage_ = damage;
    range_sq_ = range_pixels * range_pixels;
    attack_speed_ = attack_speed;
    projectile_type_ = projectile_type;

    attack_timer_ = 0.0f;
}


void AttackComp::update(float dt) {
    // 冷却倒计时
    if (attack_timer_ > 0.0f) {
        attack_timer_ -= dt;
        if (attack_timer_ < 0.0f) {
            attack_timer_ = 0.0f;
        }
    }
}

bool AttackComp::IsTargetInRange(BaseEntity* target) const {
    if (!target) return false;

    // 1. 获取物理距离
    // getParent() 是挂载这个组件的攻击者 (Unit)
    auto attacker = this->getParent();
    if (!attacker) return false;

    cocos2d::Vec2 my_pos = attacker->getPosition();
    cocos2d::Vec2 target_pos = target->getPosition();

    // 尝试将目标转换为建筑
    auto target_building = dynamic_cast<Building*>(target);
    if (target_building) {
        // --- 针对建筑的边缘检测 (Edge-to-Edge) ---
        // 解决大建筑中心点距离过远的问题
        cocos2d::Rect rect = target_building->GetOccupiedRect();
        float dx = 0.0f;
        float dy = 0.0f;

        // 计算 Unit 中心点距离矩形四边的距离
        if (my_pos.x < rect.getMinX()) dx = rect.getMinX() - my_pos.x;
        else if (my_pos.x > rect.getMaxX()) dx = my_pos.x - rect.getMaxX();

        if (my_pos.y < rect.getMinY()) dy = rect.getMinY() - my_pos.y;
        else if (my_pos.y > rect.getMaxY()) dy = my_pos.y - rect.getMaxY();

        float dist_to_edge_sq = dx * dx + dy * dy;

        return dist_to_edge_sq <= range_sq_;
    }
    else {
        // --- 针对单位的中心/半径检测 ---
        float target_radius = 20.0f;
        float dist_sq = my_pos.getDistanceSq(target_pos);

        // 有效射程 = (攻击距离 + 目标半径)^2
        float range_val = std::sqrt(range_sq_);
        float effective_range = range_val + target_radius;

        return dist_sq <= effective_range * effective_range;
    }
}

bool AttackComp::TryAttack(BaseEntity* target) {
    // 1. 基础校验
    if (!target) return false;
    if (target->IsMarkedForDestruction()) return false; // 目标已经死了

    // 2. 冷却检查
    if (attack_timer_ > 0.0f) {
        return false; // 技能还在冷却中
    }

    // 3. 范围检查
    if (!IsTargetInRange(target)) {
        return false; // 够不着
    }

    // 重置冷却
    attack_timer_ = attack_speed_;

    // 造成伤害
    DealDamage(target);

    // 返回 true，通知外部(Unit)播放攻击动作
    return true;
}

void AttackComp::DealDamage(BaseEntity* target) {
    if (!target) return;

    // 获取攻击发起者 (挂载这个组件的 Unit)
    auto attacker = dynamic_cast<BaseEntity*>(this->getParent());
    if (!attacker) return;

    // [核心转接逻辑]
    // 根据 projectile_type_ 决定走哪条路

    if (projectile_type_ == Core::ProjectileType::kNone) {
        // 分支 A: 近战 / 瞬间伤害
        // 适用于: 野蛮人 (kNone), 巨人 (kNone), 炸弹人 (kNone)
        // 注意: 炸弹人的自爆逻辑是在 ResolveMeleeAttack 里处理的，所以必须走这里
        CombatResolver::GetInstance()->ResolveMeleeAttack(attacker, target, damage_);
    }
    else {
        // 分支 B: 远程 / 投射物
        // 适用于: 弓箭手 (kArrow), 飞龙 (kFireBall)
        // 这会生成一个飞行的 Sprite，等它飞到目标身上时，CombatResolver 会自动调用 ApplyDamage
        CombatResolver::GetInstance()->SpawnProjectile(attacker, target, damage_, projectile_type_);
    }
}