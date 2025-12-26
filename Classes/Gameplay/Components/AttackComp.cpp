// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of AttackComp.
//
// Path: Classes/Gameplay/Components/AttackComp.cpp

#include "Gameplay/Components/AttackComp.h"
#include "Contract/GamePlay/HealthComp.h"
#include "Contract/GamePlay/Building.h"
#include "Contract/GamePlay/Unit.h"
#include "Contract/GamePlay/CombatResolver.h"
#include "Core/GameConstants.h"

bool AttackComp::init() {
    if (!cocos2d::Node::init()) return false;
    this->scheduleUpdate();

    damage_ = 0;
    range_sq_ = 0.0f;
    attack_speed_ = 1.0f;
    attack_timer_ = 0.0f;
    projectile_type_ = Core::ProjectileType::kNone;

    return true;
}

void AttackComp::InitStats(int damage, float range_pixels, float attack_speed,
    Core::ProjectileType projectile_type) {
    damage_ = damage;
    range_sq_ = range_pixels * range_pixels;
    attack_speed_ = attack_speed;
    projectile_type_ = projectile_type;
    attack_timer_ = 0.0f;
}

void AttackComp::update(float dt) {
    if (attack_timer_ > 0.0f) {
        attack_timer_ -= dt;
        if (attack_timer_ < 0.0f) attack_timer_ = 0.0f;
    }
}

bool AttackComp::IsTargetInRange(BaseEntity* target) const {
    if (!target) return false;

    auto attacker = this->getParent();
    if (!attacker) return false;

    cocos2d::Vec2 my_pos = attacker->getPosition();

    auto target_building = dynamic_cast<Building*>(target);
    if (target_building) {
        cocos2d::Rect rect = target_building->GetOccupiedRect();
        float dx = 0.0f, dy = 0.0f;

        if (my_pos.x < rect.getMinX()) dx = rect.getMinX() - my_pos.x;
        else if (my_pos.x > rect.getMaxX()) dx = my_pos.x - rect.getMaxX();

        if (my_pos.y < rect.getMinY()) dy = rect.getMinY() - my_pos.y;
        else if (my_pos.y > rect.getMaxY()) dy = my_pos.y - rect.getMaxY();

        return (dx * dx + dy * dy) <= range_sq_;
    }
    else {
        float target_radius = 20.0f;

        auto target_unit = dynamic_cast<Unit*>(target);
        if (target_unit) {
            target_radius = target_unit->GetCollisionRadius();
        }

        float dist_sq = my_pos.getDistanceSq(target->getPosition());
        float range_val = std::sqrt(range_sq_);
        float effective_range = range_val + target_radius;
        return dist_sq <= effective_range * effective_range;
    }
}

bool AttackComp::TryAttack(BaseEntity* target) {
    if (!target) return false;
    if (target->IsMarkedForDestruction()) return false;
    if (attack_timer_ > 0.0f) return false;
    if (!IsTargetInRange(target)) return false;

    attack_timer_ = attack_speed_;

    auto parent = this->getParent();

    if (auto unit = dynamic_cast<Unit*>(parent)) {
        unit->PlayAttackAnimation();
    }
    else if (auto building = dynamic_cast<Building*>(parent)) {
        building->PlayAttackAnimation();
    }

    DealDamage(target);
    return true;
}

void AttackComp::DealDamage(BaseEntity* target) {
    if (!target) return;

    auto attacker = dynamic_cast<BaseEntity*>(this->getParent());
    if (!attacker) return;

    if (projectile_type_ == Core::ProjectileType::kNone) {
        CombatResolver::GetInstance()->ResolveMeleeAttack(attacker, target, damage_);
    }
    else {
        CombatResolver::GetInstance()->SpawnProjectile(attacker, target, damage_, projectile_type_);
    }
}