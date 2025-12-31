// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Building class. Handles construction, resources, and defensive logic.
// [UPDATE] Added animation controller, GameEvents support
//
// Path: Classes/Contract/Gameplay/Building.h

#ifndef CONTRACT_GAMEPLAY_BUILDING_H_
#define CONTRACT_GAMEPLAY_BUILDING_H_

#include "Classes/Contract/Gameplay/BaseEntity.h"
#include "Core/GameConfig.h"

// Forward declaration
class EntityAnimationController;

/**
 * @brief Building Entity Class
 * Contains logic for construction, upgrading, resource production, and defense attacks.
 */
class Building : public BaseEntity {
public:
    // Static creation function, automatically handles memory management (autorelease)
    static Building* create(Core::BuildingType type, int level, int owner_id);

    // Initialization function: reads config, loads images
    virtual bool init(Core::BuildingType type, int level, int owner_id);

    // Lifecycle methods - Used for obstacle registration and GameEvents
    virtual void onEnter() override;
    virtual void onExit() override;

    // Frame update: handles attack cooldowns, resource production, construction progress
    virtual void update(float dt) override;

    // ==========================================================================
    // Property Query Interfaces
    // ==========================================================================

    int GetLevel() const { return level_; }
    Core::BuildingType GetBuildingType() const { return type_; }
    Core::BuildingAnimationState GetState() const { return current_state_; }
    int GetWidthInTiles() const { return stats_.width_; }
    int GetHeightInTiles() const { return stats_.height_; }

    /**
     * @brief Get the occupied rectangle (Pixel coordinates)
     */
    cocos2d::Rect GetOccupiedRect() const;

    // ==========================================================================
    // Construction Interfaces
    // ==========================================================================

    /**
     * @brief Checks if currently constructing/upgrading
     */
    bool IsConstructing() const {
        return current_state_ == Core::BuildingAnimationState::kConstructing;
    }

    /**
     * @brief Start construction or upgrade
     * @param duration Time required for construction (seconds)
     */
    void StartConstruction(float duration);

    /**
     * @brief Get construction progress (0.0 ~ 1.0)
     */
    float GetConstructionProgress() const;

    // ==========================================================================
    // Combat Interfaces
    // ==========================================================================

    /**
     * @brief Determine if able to attack the target
     * @param target_type Type of target (Ground/Air)
     */
    bool CanAttack(Core::GeneralType target_type) const {
        return (static_cast<unsigned int>(stats_.target_type_) &
            static_cast<unsigned int>(target_type)) != 0;
    }

    int GetDamage() const { return stats_.damage_; }
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }

    // ==========================================================================
    // Economy Interfaces
    // ==========================================================================

    /**
     * @brief Get currently stored resource amount (for Mines/Collectors)
     */
    int GetStoredResource() const { return static_cast<int>(stored_resource_); }

    /**
     * @brief Get storage percentage (0.0 ~ 1.0)
     */
    float GetStoragePercentage() const;

    /**
     * @brief Collect resources
     * @param max_amount Maximum limit for this collection (-1 means unlimited)
     * @return int Actual amount of resources collected
     */
    int CollectResource(int max_amount = -1);

    // ==========================================================================
    // Animation Interfaces
    // ==========================================================================

    /**
     * @brief Play attack animation
     */
    void PlayAttackAnimation();

    /**
     * @brief Play hurt blink effect
     */
    void PlayHurtEffect();

    /**
     * @brief Play destroyed animation
     */
    void PlayDestroyedAnimation();

private:
    // Core Data
    Core::BuildingStats stats_;
    Core::BuildingType type_;
    int level_;

    // State
    Core::BuildingAnimationState current_state_ = Core::BuildingAnimationState::kIdle;
    float construction_timer_ = 0.0f;
    float construction_duration_ = 0.0f;

    // Economy State
    float stored_resource_ = 0.0f;

    // Visual Components
    cocos2d::Sprite* visual_sprite_ = nullptr;
    EntityAnimationController* animation_controller_ = nullptr;

    // Obstacle registration flag
    bool obstacle_registered_ = false;

    // ==========================================================================
    // Helper Functions
    // ==========================================================================

    /**
     * @brief Switch building state
     */
    void SetState(Core::BuildingAnimationState new_state);

    /**
     * @brief Combat logic update
     */
    void UpdateCombatLogic(float dt);

    /**
     * @brief Resource production logic
     */
    void ProduceResource(float dt);

    /**
     * @brief Returns sprite sheet filename based on building type
     */
    static std::string GetSpriteSheetFilename(Core::BuildingType type);

    /**
     * @brief Gets frame size based on building type
     */
    static void GetFrameSize(Core::BuildingType type, int& out_width, int& out_height);

    /**
     * @brief Convert building type to projectile type
     */
    static Core::ProjectileType GetProjectileTypeFromBuilding(Core::BuildingType type);
};

#endif // CONTRACT_GAMEPLAY_BUILDING_H_