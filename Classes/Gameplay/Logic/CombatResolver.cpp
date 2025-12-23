// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of CombatResolver.

#include "Contract/Gameplay/CombatResolver.h"
#include "Contract/Gameplay/HealthComp.h" // [关键] 必须引用，用于检查逻辑死亡
#include "Contract/Gameplay/Unit.h"     
#include "Contract/Gameplay/Building.h" 
#include <cmath>

// 单例实现
CombatResolver* CombatResolver::GetInstance() {
    static CombatResolver instance;
    return &instance;
}

void CombatResolver::Initialize(cocos2d::Node* battle_layer) {
    this->battle_layer_ = battle_layer;
    // 清理旧数据前，记得 release 所有目标的引用
    for (auto& proj : projectiles_) {
        if (proj.target) {
            proj.target->release();
        }
    }
    this->projectiles_.clear();

    // 我们借用 battle_layer (也就是 Scene) 的调度器来驱动 CombatResolver::Update
    if (battle_layer_) {
        // 先取消可能存在的旧调度（防止重复初始化导致加速）
        battle_layer_->unschedule("CombatResolver_Update_Key");

        // [修复] 权威的步进策略 (Authoritative Step Policy)
        // [BUG FIX] 之前传入 10 被引擎解释为 interval (10秒执行一次)，导致投射物不动。
        // 现在改为 0.0f，表示每帧执行。
        battle_layer_->schedule([this](float dt) {
            this->Update(dt);
            }, 0.0f, "CombatResolver_Update_Key");

        cocos2d::log("CombatResolver: Engine Started. Update loop is active.");
    }
}

void CombatResolver::Update(float dt) {
    if (!battle_layer_) return;

    // 使用迭代器遍历，以便安全删除已命中的投射物
    for (auto it = projectiles_.begin(); it != projectiles_.end(); ) {
        ProjectileObject& proj = *it;

        // ---------------------------------------------------------------------
        // 1. 目标状态检查 (Target Check)
        // ---------------------------------------------------------------------
        // 我们需要判断：目标是否还值得我们去追踪？

        bool is_target_logically_alive = false;
        cocos2d::Vec2 target_current_pos;

        // 因为我们 Retain 了 target，所以 target 指针永远是有效的内存地址
        // 我们只需要检查它的逻辑状态
        if (proj.target) {
            // 如果已经被标记销毁，或者血量归零，视为无效
            if (!proj.target->IsMarkedForDestruction()) {
                auto hp = dynamic_cast<HealthComp*>(proj.target->getChildByName("HealthComp"));
                if (hp && !hp->IsDead()) {
                    is_target_logically_alive = true;
                }
            }
        }

        // ---------------------------------------------------------------------
        // 2. 导航逻辑 (Navigation)
        // ---------------------------------------------------------------------

        if (is_target_logically_alive) {
            // 目标存活：更新目标位置，继续追踪
            target_current_pos = proj.target->GetCenterPosition();
            proj.last_known_pos = target_current_pos; // 随时记录，万一它下一帧死了
        }
        else {
            // 目标失效了
            if (!proj.target_lost) {
                proj.target_lost = true;
                // [关键修复] 目标丢失，不再追踪，赶紧释放引用！
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

        // 命中阈值 (例如 15像素)
        float hit_threshold = 15.0f;

        // ---------------------------------------------------------------------
        // 3. 命中判定 (Hit Detection)
        // ---------------------------------------------------------------------
        if (dist_sq <= hit_threshold * hit_threshold) {
            // 命中
            if (!proj.target_lost && proj.target) {
                ApplyDamage(proj.target, proj.damage);
                PlayImpactVFX(target_current_pos, proj.type);

                // [关键修复] 投射物任务完成，释放目标引用
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
            // -----------------------------------------------------------------
            // 4. 继续飞行 (Move)
            // -----------------------------------------------------------------
            dir.normalize();
            cocos2d::Vec2 new_pos = current_pos + (dir * proj.speed * dt);
            proj.sprite->setPosition(new_pos);

            // 旋转图片朝向目标
            float angle = CC_RADIANS_TO_DEGREES(atan2(dir.y, dir.x));
            proj.sprite->setRotation(-angle);

            ++it;
        }
    }
}

// 近战结算
void CombatResolver::ResolveMeleeAttack(BaseEntity* attacker, BaseEntity* target, int damage) {
    if (!target || !attacker) return;

    // 1. 检查是否为炸弹人 (Wall Breaker)
    bool is_wall_breaker = false;
    auto unit_attacker = dynamic_cast<Unit*>(attacker);

    if (unit_attacker && unit_attacker->GetTroopType() == Core::TroopType::kWallBreaker) {
        is_wall_breaker = true;
    }

    // 逻辑分支 A: 炸弹人自爆逻辑
    if (is_wall_breaker) {
        int final_damage = damage;

        // 炸弹人对墙造成 40 倍伤害
        auto building_target = dynamic_cast<Building*>(target);
        if (building_target && building_target->GetBuildingType() == Core::BuildingType::kWall) {
            final_damage *= 40;
            cocos2d::log("Combat: WallBreaker Hit Wall! Bonus Dmg: %d", final_damage);
        }

        // 造成伤害 (AoE 可以在这里扩展，目前先做单体巨额伤害)
        ApplyDamage(target, final_damage);

        // 使用 CannonBall 的特效作为替代，或者专门做一个更大的
        PlayImpactVFX(target->GetCenterPosition(), Core::ProjectileType::kFireBall);

        // [机制 4] 自杀 (Kamikaze)
        // 获取攻击者自己的血条组件，直接扣除无限血量
        auto my_hp = dynamic_cast<HealthComp*>(attacker->getChildByName("HealthComp"));
        if (my_hp) {
            // 99999 伤害确保必死
            my_hp->TakeDamage(99999);
        }
    }
    // -------------------------------------------------------------------------
    // 逻辑分支 B: 普通近战
    // -------------------------------------------------------------------------
    else {
        // 普通单位，直接扣血
        ApplyDamage(target, damage);
    }
}

// 远程生成
void CombatResolver::SpawnProjectile(BaseEntity* attacker, BaseEntity* target, int damage, Core::ProjectileType type) {
    if (!battle_layer_ || !attacker || !target) return;

    ProjectileObject proj;
    proj.id = 0; // 可以加自增ID
    proj.target = target;
    proj.target->retain();
    proj.damage = damage;
    proj.type = type;
    proj.speed = GetProjectileSpeed(type);
    proj.target_lost = false;
    proj.last_known_pos = target->GetCenterPosition();

    std::string filename = GetProjectileFilename(type);
    // 防御性加载
    if (cocos2d::FileUtils::getInstance()->isFileExist(filename)) {
        proj.sprite = cocos2d::Sprite::create(filename);
    }
    else {
        // 如果找不到箭头图片，暂时用一个小的黄色方块代替，确保能看到东西飞过去
        auto draw = cocos2d::DrawNode::create();
        draw->drawSolidRect(cocos2d::Vec2(-5, -2), cocos2d::Vec2(5, 2), cocos2d::Color4F::YELLOW);

        // 把 DrawNode 包装进 Sprite 有点麻烦，直接创建空 Sprite 然后 addChild DrawNode
        proj.sprite = cocos2d::Sprite::create();
        proj.sprite->addChild(draw);
    }

    if (proj.sprite) {
        proj.sprite->setPosition(attacker->GetCenterPosition());
        proj.sprite->setLocalZOrder(static_cast<int>(Core::ZOrder::kProjectiles)); // 确保在单位上方
        battle_layer_->addChild(proj.sprite);
        projectiles_.push_back(proj);
    }
    else {
        // 如果创建精灵失败，记得把 retain 的释放掉，否则内存泄漏
        proj.target->release();
    }
}

// 内部：扣血执行
void CombatResolver::ApplyDamage(BaseEntity* target, int damage) {
    if (!target) return;
    auto hp = dynamic_cast<HealthComp*>(target->getChildByName("HealthComp"));
    if (hp) {
        hp->TakeDamage(damage);
    }
}

// 内部：特效
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
        color = cocos2d::Color4F(0.2f, 0.2f, 0.2f, 1.0f); // 深灰
        radius = 15.0f; // 更大的爆炸

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