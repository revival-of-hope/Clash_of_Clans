// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of CombatResolver.
//
// Path: Classes/Gameplay/Logic/CombatResolver.cpp

#include "Contract/GamePlay/CombatResolver.h"
#include "Contract/GamePlay/HealthComp.h"
#include "Contract/GamePlay/Unit.h"     
#include "Contract/GamePlay/Building.h" 
#include <cmath>

CombatResolver* CombatResolver::GetInstance() {
    static CombatResolver instance;
    return &instance;
}

void CombatResolver::Initialize(cocos2d::Node* battle_layer) {
    this->battle_layer_ = battle_layer;

    for (auto& proj : projectiles_) {
        if (proj.target) {
            proj.target->release();
        }
    }
    this->projectiles_.clear();

    if (battle_layer_) {
        battle_layer_->unschedule("CombatResolver_Update_Key");

        battle_layer_->schedule([this](float dt) {
            this->Update(dt);
            }, 0.0f, "CombatResolver_Update_Key");

        cocos2d::log("CombatResolver: Engine Started. Update loop is active.");
    }
}

void CombatResolver::Update(float dt) {
    if (!battle_layer_) return;

    for (auto it = projectiles_.begin(); it != projectiles_.end(); ) {
        ProjectileObject& proj = *it;

        bool is_target_logically_alive = false;
        cocos2d::Vec2 target_current_pos;

        if (proj.target) {
            if (!proj.target->IsMarkedForDestruction()) {
                auto hp = dynamic_cast<HealthComp*>(proj.target->getChildByName("HealthComp"));
                if (hp && !hp->IsDead()) {
                    is_target_logically_alive = true;
                }
            }
        }

        if (is_target_logically_alive) {
            target_current_pos = proj.target->GetCenterPosition();
            proj.last_known_pos = target_current_pos;
        }
        else {
            if (!proj.target_lost) {
                proj.target_lost = true;
                if (proj.target) {
                    proj.target->release();
                    proj.target = nullptr;
                }
            }
            target_current_pos = proj.last_known_pos;
        }

        cocos2d::Vec2 current_pos = proj.sprite->getPosition();
        cocos2d::Vec2 dir = target_current_pos - current_pos;
        float dist_sq = dir.getLengthSq();

        float hit_threshold = 15.0f;

        if (dist_sq <= hit_threshold * hit_threshold) {
            if (!proj.target_lost && proj.target) {
                ApplyDamage(proj.target, proj.damage);
                PlayImpactVFX(target_current_pos, proj.type);

                proj.target->release();
                proj.target = nullptr;
            }
            else {
                PlayImpactVFX(target_current_pos, proj.type);
            }

            proj.sprite->removeFromParent();
            it = projectiles_.erase(it);
        }
        else {
            dir.normalize();
            cocos2d::Vec2 new_pos = current_pos + (dir * proj.speed * dt);
            proj.sprite->setPosition(new_pos);

            float angle = CC_RADIANS_TO_DEGREES(atan2(dir.y, dir.x));
            proj.sprite->setRotation(-angle);

            ++it;
        }
    }
}

void CombatResolver::ResolveMeleeAttack(BaseEntity* attacker, BaseEntity* target, int damage) {
    if (!target || !attacker) return;

    bool is_wall_breaker = false;
    auto unit_attacker = dynamic_cast<Unit*>(attacker);

    if (unit_attacker && unit_attacker->GetTroopType() == Core::TroopType::kWallBreaker) {
        is_wall_breaker = true;
    }

    if (is_wall_breaker) {
        int final_damage = damage;

        auto building_target = dynamic_cast<Building*>(target);
        if (building_target && building_target->GetBuildingType() == Core::BuildingType::kWall) {
            final_damage *= 40;
            cocos2d::log("Combat: WallBreaker Hit Wall! Bonus Dmg: %d", final_damage);
        }

        ApplyDamage(target, final_damage);
        PlayImpactVFX(target->GetCenterPosition(), Core::ProjectileType::kFireBall);

        auto my_hp = dynamic_cast<HealthComp*>(attacker->getChildByName("HealthComp"));
        if (my_hp) {
            my_hp->TakeDamage(99999);
        }
    }
    else {
        ApplyDamage(target, damage);
    }
}

void CombatResolver::SpawnProjectile(BaseEntity* attacker, BaseEntity* target, int damage, Core::ProjectileType type) {
    if (!battle_layer_ || !attacker || !target) return;

    ProjectileObject proj;
    proj.id = 0;
    proj.target = target;
    proj.target->retain();
    proj.damage = damage;
    proj.type = type;
    proj.speed = GetProjectileSpeed(type);
    proj.target_lost = false;
    proj.last_known_pos = target->GetCenterPosition();

    std::string filename = GetProjectileFilename(type);

    if (cocos2d::FileUtils::getInstance()->isFileExist(filename)) {
        proj.sprite = cocos2d::Sprite::create(filename);
    }
    else {
        auto draw = cocos2d::DrawNode::create();
        draw->drawSolidRect(cocos2d::Vec2(-5, -2), cocos2d::Vec2(5, 2), cocos2d::Color4F::YELLOW);

        proj.sprite = cocos2d::Sprite::create();
        proj.sprite->addChild(draw);
    }

    if (proj.sprite) {
        proj.sprite->setPosition(attacker->GetCenterPosition());
        proj.sprite->setLocalZOrder(static_cast<int>(Core::ZOrder::kProjectiles));
        battle_layer_->addChild(proj.sprite);
        projectiles_.push_back(proj);
    }
    else {
        proj.target->release();
    }
}

void CombatResolver::ApplyDamage(BaseEntity* target, int damage) {
    if (!target) return;
    auto hp = dynamic_cast<HealthComp*>(target->getChildByName("HealthComp"));
    if (hp) {
        hp->TakeDamage(damage);
    }
}

void CombatResolver::PlayImpactVFX(const cocos2d::Vec2& pos, Core::ProjectileType type) {
    if (!battle_layer_) return;

    auto explosion = cocos2d::DrawNode::create();

    cocos2d::Color4F color = cocos2d::Color4F::WHITE;
    float radius = 10.0f;
    float duration = 0.2f;

    if (type == Core::ProjectileType::kFireBall) {
        color = cocos2d::Color4F::ORANGE;
        radius = 15.0f;
        duration = 0.4f;
    }
    else if (type == Core::ProjectileType::kCannonBall) {
        color = cocos2d::Color4F(0.2f, 0.2f, 0.2f, 1.0f);
        radius = 15.0f;
    }

    explosion->drawSolidCircle(cocos2d::Vec2::ZERO, radius, 0, 10, color);
    explosion->setPosition(pos);
    explosion->setLocalZOrder(static_cast<int>(Core::ZOrder::kExplosions));
    battle_layer_->addChild(explosion);

    auto seq = cocos2d::Sequence::create(
        cocos2d::ScaleTo::create(0.1f, 1.5f),
        cocos2d::FadeOut::create(duration),
        cocos2d::RemoveSelf::create(),
        nullptr
    );
    explosion->runAction(seq);
}

std::string CombatResolver::GetProjectileFilename(Core::ProjectileType type) {
    switch (type) {
    case Core::ProjectileType::kArrow:      return "Projectile_Icon/Arrow.png";
    case Core::ProjectileType::kCannonBall: return "Projectile_Icon/CannonBall.png";
    case Core::ProjectileType::kFireBall:   return "Projectile_Icon/FireBall.png";
    case Core::ProjectileType::kRocket:     return "Projectile_Icon/Rocket.png";
    default:                                return "Projectile_Icon/Default.png";
    }
}

float CombatResolver::GetProjectileSpeed(Core::ProjectileType type) {
    switch (type) {
    case Core::ProjectileType::kArrow:      return 400.0f;
    case Core::ProjectileType::kCannonBall: return 300.0f;
    case Core::ProjectileType::kFireBall:   return 250.0f;
    case Core::ProjectileType::kRocket:     return 500.0f;
    default:                                return 300.0f;
    }
}