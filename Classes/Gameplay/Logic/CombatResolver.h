// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Singleton for managing projectiles and resolving damage calculation.

#ifndef GAMEPLAY_LOGIC_COMBAT_RESOLVER_H_
#define GAMEPLAY_LOGIC_COMBAT_RESOLVER_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include "Gameplay/Entities/BaseEntity.h"
#include <vector>

/**
 * @brief 战斗仲裁系统 (单例)
 * 职责:
 * 1. 投射物管理: 生成、移动、销毁所有飞行的箭矢、炮弹、火球。
 * 2. 伤害仲裁: 也就是 "Who hit whom"，负责最终扣血的执行。
 * 3. 伤害计算: 未来可以在这里添加防御力减免、属性克制等公式。
 */
class CombatResolver {
public:
    // 单例访问
    static CombatResolver* GetInstance();

    /**
     * @brief 初始化战斗系统
     * @param battle_layer 传入战斗场景的 Layer，用于挂载投射物和 Update 调度
     */
    void Initialize(cocos2d::Node* battle_layer);

    /**
     * @brief 帧更新：处理投射物飞行
     * 由 Scene 的 schedule 调用
     */
    void Update(float dt);

    /**
     * @brief 发起近战攻击 (Instant Hit)
     * 直接结算伤害，无飞行过程
     * @param attacker 攻击者
     * @param target 受害者
     * @param damage 伤害值
     */
    void ResolveMeleeAttack(BaseEntity* attacker, BaseEntity* target, int damage);

    /**
     * @brief 发起远程攻击 (Ranged Attack)
     * 生成一个投射物，投射物命中后才会结算伤害
     * @param attacker 攻击者
     * @param target 锁定目标
     * @param damage 携带伤害
     * @param type 投射物类型
     */
    void SpawnProjectile(BaseEntity* attacker, BaseEntity* target, int damage, Core::ProjectileType type);

private:
    // 内部结构：定义一个飞行中的投射物
    struct ProjectileObject {
        int id;                     // 唯一ID
        cocos2d::Sprite* sprite;    // 外观
        BaseEntity* target;         // 锁定目标 (弱引用)
        int damage;                 // 携带的伤害
        float speed;                // 飞行速度 (像素/秒)
        Core::ProjectileType type;  // 类型 (决定特效)

        // 用于处理目标死亡后的逻辑：飞向最后已知位置
        cocos2d::Vec2 last_known_pos;
        bool target_lost;           // 目标是否已失效
    };

    // 容器：所有活跃的投射物
    std::vector<ProjectileObject> projectiles_;

    // 场景层引用 (用于 addChild)
    cocos2d::Node* battle_layer_ = nullptr;

    // 辅助：获取投射物图片路径
    std::string GetProjectileFilename(Core::ProjectileType type);

    // 辅助：获取投射物速度
    float GetProjectileSpeed(Core::ProjectileType type);

    // 辅助：命中特效
    void PlayImpactVFX(const cocos2d::Vec2& pos, Core::ProjectileType type);

    // 实际伤害执行
    void ApplyDamage(BaseEntity* target, int damage);
};

#endif // GAMEPLAY_LOGIC_COMBAT_RESOLVER_H_