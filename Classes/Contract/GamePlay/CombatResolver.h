// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Singleton for managing projectiles and resolving damage calculation.
// Contract header for external modules.
//
// Path: Classes/Contract/GamePlay/CombatResolver.h

#ifndef CONTRACT_GAMEPLAY_COMBAT_RESOLVER_H_
#define CONTRACT_GAMEPLAY_COMBAT_RESOLVER_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include "Contract/GamePlay/BaseEntity.h"
#include <vector>

/**
 * @brief 战斗仲裁系统 (单例)
 * 职责:
 * 1. 投射物管理: 生成、移动、销毁所有飞行的箭矢、炮弹、火球。
 * 2. 伤害仲裁: 负责最终扣血的执行。
 * 3. 伤害计算: 未来可以添加防御力减免、属性克制等公式。
 */
class CombatResolver {
public:
    static CombatResolver* GetInstance();

    /**
     * @brief 初始化战斗系统
     * @param battle_layer 传入战斗场景的 Layer
     */
    void Initialize(cocos2d::Node* battle_layer);

    /**
     * @brief 帧更新：处理投射物飞行
     */
    void Update(float dt);

    /**
     * @brief 发起近战攻击 (Instant Hit)
     */
    void ResolveMeleeAttack(BaseEntity* attacker, BaseEntity* target, int damage);

    /**
     * @brief 发起远程攻击 (Ranged Attack)
     */
    void SpawnProjectile(BaseEntity* attacker, BaseEntity* target, int damage, Core::ProjectileType type);

private:
    struct ProjectileObject {
        int id;
        cocos2d::Sprite* sprite;
        BaseEntity* target;
        int damage;
        float speed;
        Core::ProjectileType type;
        cocos2d::Vec2 last_known_pos;
        bool target_lost;
    };

    std::vector<ProjectileObject> projectiles_;
    cocos2d::Node* battle_layer_ = nullptr;

    std::string GetProjectileFilename(Core::ProjectileType type);
    float GetProjectileSpeed(Core::ProjectileType type);
    void PlayImpactVFX(const cocos2d::Vec2& pos, Core::ProjectileType type);
    void ApplyDamage(BaseEntity* target, int damage);
};

#endif // CONTRACT_GAMEPLAY_COMBAT_RESOLVER_H_