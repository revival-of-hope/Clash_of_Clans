// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Unit class representing movable troops.
// Manages State Machine (Idle, Move, Attack) and Components.
// [UPDATE] Added onEnter() for GameEvents, animation controller members
//
// Path: Classes/Contract/GamePlay/Unit.h

#ifndef CONTRACT_GAMEPLAY_UNIT_H_
#define CONTRACT_GAMEPLAY_UNIT_H_

#include "Contract/GamePlay/BaseEntity.h"
#include "Core/GameConfig.h"

// 前向声明
class EntityAnimationController;

/**
 * @brief 兵种单位类
 * 继承自 BaseEntity，拥有移动、攻击和状态机逻辑。
 */
class Unit : public BaseEntity {
public:
    /**
     * @brief 静态工厂方法，创建 Unit 实例
     * @param type 兵种类型
     * @param level 等级
     * @param owner_id 所有者 ID
     * @return Unit* 创建成功的对象，autorelease
     */
    static Unit* create(Core::TroopType type, int level, int owner_id);

    virtual bool init(Core::TroopType type, int level, int owner_id);

    // [NEW] 生命周期函数 - 用于广播 EntitySpawnEvent
    virtual void onEnter() override;

    virtual void update(float dt) override;

    // ==========================================================================
    // 属性查询接口
    // ==========================================================================

    Core::GeneralType GetGeneralType() const { return stats_.unit_type_; }
    Core::TroopType GetTroopType() const { return type_; }
    int GetDamage() const { return stats_.damage_per_shot_; }
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }
    int GetLevel() const { return level_; }
    float GetCollisionRadius() const { return collision_radius_; }

    /**
     * @brief 获取单位占用的矩形区域
     */
    cocos2d::Rect GetOccupiedRect() const;

    // ==========================================================================
    // 战斗接口
    // ==========================================================================

    /**
     * @brief 核心接口：判断我能否攻击某种类型的目标
     * @param target_type 目标的 GeneralType (Ground/Air)
     * @return true 如果可以攻击，false 则不能
     */
    bool CanAttack(Core::GeneralType target_type) const;

    // ==========================================================================
    // 状态与动画接口
    // ==========================================================================

    /**
     * @brief 切换单位状态
     * 会处理状态切换时的逻辑，如播放动画、停止寻路等
     * @param new_state 新状态 (Core::UnitAnimationState)
     */
    void SetState(Core::UnitAnimationState new_state);

    /**
     * @brief 设置朝向
     * @param facing 新朝向
     */
    void SetFacing(Core::Facing facing);

    Core::UnitAnimationState GetState() const { return current_state_; }
    Core::Facing GetFacing() const { return current_facing_; }

    /**
     * @brief 播放攻击动画
     */
    void PlayAttackAnimation();

    /**
     * @brief 播放受伤闪烁效果
     */
    void PlayHurtEffect();

    /**
     * @brief 播放死亡动画
     */
    void PlayDeathAnimation();

private:
    // 核心数据
    Core::UnitStats stats_;
    Core::TroopType type_;
    int level_;
    float collision_radius_ = 20.0f;

    // 状态
    Core::UnitAnimationState current_state_ = Core::UnitAnimationState::kIdle;
    Core::Facing current_facing_ = Core::Facing::kRight;

    // 视觉组件
    cocos2d::Sprite* visual_sprite_ = nullptr;
    EntityAnimationController* animation_controller_ = nullptr;

    // ==========================================================================
    // 辅助函数
    // ==========================================================================

    /**
     * @brief 根据兵种类型返回精灵图文件名
     */
    static std::string GetSpriteSheetFilename(Core::TroopType type);

    /**
     * @brief 根据兵种类型获取帧尺寸
     */
    static void GetFrameSize(Core::TroopType type, int& out_width, int& out_height);
};

#endif // CONTRACT_GAMEPLAY_UNIT_H_