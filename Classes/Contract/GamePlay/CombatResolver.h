// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Singleton for managing projectiles and resolving damage calculation.
// Contract header for external modules.
//
// Path: Classes/Contract/Gameplay/CombatResolver.h

#ifndef CONTRACT_GAMEPLAY_COMBAT_RESOLVER_H_
#define CONTRACT_GAMEPLAY_COMBAT_RESOLVER_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include "Classes/Contract/Gameplay/BaseEntity.h"
#include <vector>

/**
 * @brief Combat Resolver System (Singleton)
 * Responsibilities:
 * 1. Projectile Management: Generate, move, and destroy all flying arrows, shells, fireballs.
 * 2. Damage Arbitration: Responsible for the execution of final HP deduction.
 * 3. Damage Calculation: Future extension for defense reduction, attribute counters, etc.
 */
class CombatResolver {
public:
    static CombatResolver* GetInstance();

    /**
     * @brief Initialize Combat System
     * @param battle_layer Pass in the Layer of the combat scene
     */
    void Initialize(cocos2d::Node* battle_layer);

    /**
     * @brief Frame Update: Handle projectile flight
     */
    void Update(float dt);

    /**
     * @brief Initiate Melee Attack (Instant Hit)
     */
    void ResolveMeleeAttack(BaseEntity* attacker, BaseEntity* target, int damage);

    /**
     * @brief Initiate Ranged Attack (Ranged Attack)
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