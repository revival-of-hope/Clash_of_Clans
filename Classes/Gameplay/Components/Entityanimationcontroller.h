// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Unified Animation Controller for both Unit and Building sprites.
//
// Path: Classes/Gameplay/Components/EntityAnimationController.h

#ifndef GAMEPLAY_COMPONENTS_ENTITY_ANIMATION_CONTROLLER_H_
#define GAMEPLAY_COMPONENTS_ENTITY_ANIMATION_CONTROLLER_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include <unordered_map>
#include <string>

/**
 * @brief 统一的实体动画控制器
 *
 * 支持两种精灵图布局:
 *
 * Unit 布局 (4x4):
 * ┌────────┬────────┬────────┬────────┐
 * │ UP_A   │ UP_B   │ EMPTY  │ EMPTY  │
 * ├────────┼────────┼────────┼────────┤
 * │ RIGHT_A│ RIGHT_B│ EMPTY  │ EMPTY  │
 * ├────────┼────────┼────────┼────────┤
 * │ DOWN_A │ DOWN_B │ EMPTY  │ EMPTY  │
 * ├────────┼────────┼────────┼────────┤
 * │ ATTACK │ HURT   │ DEAD   │ EMPTY  │
 * └────────┴────────┴────────┴────────┘
 *
 * Building 布局 (2x2):
 * ┌────────┬────────┐
 * │ IDLE   │ ATTACK │
 * ├────────┼────────┤
 * │ HURT   │ DEAD   │
 * └────────┴────────┘
 */
class EntityAnimationController : public cocos2d::Node {
public:
    enum class LayoutType {
        kUnit,
        kBuilding
    };

    CREATE_FUNC(EntityAnimationController);

    virtual bool init() override;
    virtual void update(float dt) override;

    void InitWithSprite(cocos2d::Sprite* sprite,
        const std::string& sprite_sheet_path,
        LayoutType layout,
        int frame_width,
        int frame_height);

    // =========================================================================
    // 通用接口
    // =========================================================================

    void PlayAttackAnimation(float duration = 0.3f);
    void PlayHurtFlash();
    void PlayDeathAnimation();
    bool IsPlayingOneShot() const { return is_playing_attack_ || is_hurt_flashing_; }
    bool IsDead() const { return is_dead_; }

    // =========================================================================
    // Unit 专用接口
    // =========================================================================

    void SetUnitAnimationState(Core::UnitAnimationState state);
    void SetFacing(Core::Facing facing);
    Core::Facing GetFacing() const { return current_facing_; }
    Core::UnitAnimationState GetUnitAnimationState() const { return unit_state_; }

    // =========================================================================
    // Building 专用接口
    // =========================================================================

    void SetBuildingAnimationState(Core::BuildingAnimationState state);
    Core::BuildingAnimationState GetBuildingAnimationState() const { return building_state_; }

private:
    enum class UnitFrameIndex {
        kUpA = 0, kUpB = 1,
        kRightA = 4, kRightB = 5,
        kDownA = 8, kDownB = 9,
        kAttack = 12, kHurt = 13, kDead = 14
    };

    enum class BuildingFrameIndex {
        kIdle = 0, kAttack = 1,
        kHurt = 2, kDead = 3
    };

    cocos2d::SpriteFrame* GetFrameAt(int index);
    void UpdateCurrentFrame();
    int GetUnitDirectionalFrameIndex(bool is_frame_b);

    cocos2d::Sprite* target_sprite_ = nullptr;
    cocos2d::Texture2D* sprite_sheet_ = nullptr;
    std::string sprite_sheet_path_;

    LayoutType layout_type_ = LayoutType::kUnit;
    int frame_width_ = 64;
    int frame_height_ = 64;
    int cols_ = 4;

    Core::UnitAnimationState unit_state_ = Core::UnitAnimationState::kIdle;
    Core::Facing current_facing_ = Core::Facing::kRight;
    Core::BuildingAnimationState building_state_ = Core::BuildingAnimationState::kIdle;

    float animation_timer_ = 0.0f;
    float animation_interval_ = 0.2f;
    bool is_frame_b_ = false;

    bool is_playing_attack_ = false;
    float attack_timer_ = 0.0f;
    float attack_duration_ = 0.3f;
    int attack_frame_phase_ = 0;

    bool is_hurt_flashing_ = false;
    float hurt_flash_timer_ = 0.0f;

    bool is_dead_ = false;

    std::unordered_map<int, cocos2d::SpriteFrame*> frame_cache_;
};

#endif // GAMEPLAY_COMPONENTS_ENTITY_ANIMATION_CONTROLLER_H_