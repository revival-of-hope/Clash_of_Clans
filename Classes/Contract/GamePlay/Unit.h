// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Unit class representing movable troops.
// Contract header for external modules.
//
// Path: Classes/Contract/GamePlay/Unit.h

#ifndef CONTRACT_GAMEPLAY_UNIT_H_
#define CONTRACT_GAMEPLAY_UNIT_H_

#include "Contract/GamePlay/BaseEntity.h"
#include "Core/GameConfig.h"

class EntityAnimationController;

/**
 * @brief 兵种单位类
 * 继承自 BaseEntity，拥有移动、攻击和状态机逻辑。
 */
class Unit : public BaseEntity {
public:
    static Unit* create(Core::TroopType type, int level, int owner_id);
    virtual bool init(Core::TroopType type, int level, int owner_id);
    virtual void update(float dt) override;

    // =========================================================================
    // 基础属性查询
    // =========================================================================

    Core::GeneralType GetGeneralType() const { return stats_.unit_type_; }
    Core::TroopType GetTroopType() const { return type_; }
    int GetDamage() const { return stats_.damage_per_shot_; }
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }
    int GetLevel() const { return level_; }
    float GetAttackSpeed() const { return stats_.attack_speed_; }
    float GetMoveSpeed() const { return stats_.move_speed_; }
    int GetMaxHP() const { return stats_.max_hp_; }
    int GetHousingSpace() const { return stats_.housing_space_; }

    // =========================================================================
    // 战斗接口
    // =========================================================================

    bool CanAttack(Core::GeneralType target_type) const;

    // =========================================================================
    // 状态查询接口 - 契约层
    // =========================================================================

    Core::UnitAnimationState GetState() const { return current_state_; }
    Core::Facing GetFacing() const { return current_facing_; }

    bool IsMoving() const {
        return current_state_ == Core::UnitAnimationState::kMove;
    }
    bool IsAttacking() const {
        return current_state_ == Core::UnitAnimationState::kAttack;
    }
    bool IsDead() const {
        return current_state_ == Core::UnitAnimationState::kDead;
    }
    bool IsIdle() const {
        return current_state_ == Core::UnitAnimationState::kIdle;
    }

    // =========================================================================
    // 占地信息 - 契约层
    // =========================================================================

    cocos2d::Rect GetOccupiedRect() const;
    float GetCollisionRadius() const { return collision_radius_; }

    // =========================================================================
    // 状态控制
    // =========================================================================

    void SetState(Core::UnitAnimationState new_state);
    void SetFacing(Core::Facing facing);

    // =========================================================================
    // 动画触发接口
    // =========================================================================

    void PlayAttackAnimation();
    void PlayHurtEffect();
    void PlayDeathAnimation();

private:
    Core::UnitStats stats_;
    Core::TroopType type_;
    cocos2d::Sprite* visual_sprite_ = nullptr;
    EntityAnimationController* animation_controller_ = nullptr;

    Core::UnitAnimationState current_state_ = Core::UnitAnimationState::kIdle;
    Core::Facing current_facing_ = Core::Facing::kRight;
    int level_;
    float collision_radius_ = 20.0f;

    std::string GetSpriteSheetFilename(Core::TroopType type);
    void GetFrameSize(Core::TroopType type, int& out_width, int& out_height);
};

#endif // CONTRACT_GAMEPLAY_UNIT_H_