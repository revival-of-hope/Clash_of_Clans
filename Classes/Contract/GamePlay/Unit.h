// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Unit class representing movable troops.
// Manages State Machine (Idle, Move, Attack) and Components.
// [UPDATE] Added onEnter() for GameEvents, animation controller members
//
// Path: Classes/Contract/Gameplay/Unit.h

#ifndef CONTRACT_GAMEPLAY_UNIT_H_
#define CONTRACT_GAMEPLAY_UNIT_H_

#include "Classes/Contract/Gameplay/BaseEntity.h"
#include "Core/GameConfig.h"

// Forward declaration
class EntityAnimationController;

/**
 * @brief Troop Unit Class
 * Inherits from BaseEntity, possessing movement, attack, and state machine logic.
 */
class Unit : public BaseEntity {
public:
    /**
     * @brief Static factory method to create a Unit instance
     * @param type Troop type
     * @param level Level
     * @param owner_id Owner ID
     * @return Unit* The created object, autorelease
     */
    static Unit* create(Core::TroopType type, int level, int owner_id);

    virtual bool init(Core::TroopType type, int level, int owner_id);

    // [NEW] Lifecycle method - Used to broadcast EntitySpawnEvent
    virtual void onEnter() override;

    virtual void update(float dt) override;

    // ==========================================================================
    // Property Query Interfaces
    // ==========================================================================

    Core::GeneralType GetGeneralType() const { return stats_.unit_type_; }
    Core::TroopType GetTroopType() const { return type_; }
    int GetDamage() const { return stats_.damage_per_shot_; }
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }
    int GetLevel() const { return level_; }
    float GetCollisionRadius() const { return collision_radius_; }

    /**
     * @brief Get the rectangular area occupied by the unit
     */
    cocos2d::Rect GetOccupiedRect() const;

    // ==========================================================================
    // Combat Interfaces
    // ==========================================================================

    /**
     * @brief Core Interface: Determine if I can attack a specific target type
     * @param target_type Target's GeneralType (Ground/Air)
     * @return true if attackable, false otherwise
     */
    bool CanAttack(Core::GeneralType target_type) const;

    // ==========================================================================
    // State and Animation Interfaces
    // ==========================================================================

    /**
     * @brief Switch unit state
     * Handles logic during state switching, such as playing animation, stopping pathfinding, etc.
     * @param new_state New state (Core::UnitAnimationState)
     */
    void SetState(Core::UnitAnimationState new_state);

    /**
     * @brief Set facing direction
     * @param facing New facing direction
     */
    void SetFacing(Core::Facing facing);

    Core::UnitAnimationState GetState() const { return current_state_; }
    Core::Facing GetFacing() const { return current_facing_; }

    /**
     * @brief Play attack animation
     */
    void PlayAttackAnimation();

    /**
     * @brief Play hurt blink effect
     */
    void PlayHurtEffect();

    /**
     * @brief Play death animation
     */
    void PlayDeathAnimation();

private:
    // Core Data
    Core::UnitStats stats_;
    Core::TroopType type_;
    int level_;
    float collision_radius_ = 20.0f;

    // State
    Core::UnitAnimationState current_state_ = Core::UnitAnimationState::kIdle;
    Core::Facing current_facing_ = Core::Facing::kRight;

    // Visual Components
    cocos2d::Sprite* visual_sprite_ = nullptr;
    EntityAnimationController* animation_controller_ = nullptr;

    // ==========================================================================
    // Helper Functions
    // ==========================================================================

    /**
     * @brief Returns sprite sheet filename based on troop type
     */
    static std::string GetSpriteSheetFilename(Core::TroopType type);

    /**
     * @brief Gets frame size based on troop type
     */
    static void GetFrameSize(Core::TroopType type, int& out_width, int& out_height);
};

#endif // CONTRACT_GAMEPLAY_UNIT_H_