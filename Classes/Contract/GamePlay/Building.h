// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Building class. Handles construction, resources, and defensive logic.
// Contract header for external modules.
//
// Path: Classes/Contract/GamePlay/Building.h

#ifndef CONTRACT_GAMEPLAY_BUILDING_H_
#define CONTRACT_GAMEPLAY_BUILDING_H_

#include "Contract/GamePlay/BaseEntity.h"
#include "Core/GameConfig.h"

class EntityAnimationController;

/**
 * @brief 建筑实体类
 * 包含建造、升级、资源生产和防御攻击的逻辑。
 */
class Building : public BaseEntity {
public:
    static Building* create(Core::BuildingType type, int level, int owner_id);
    virtual bool init(Core::BuildingType type, int level, int owner_id);
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void update(float dt) override;

    // =========================================================================
    // 基础属性查询
    // =========================================================================

    int GetLevel() const { return level_; }
    Core::BuildingType GetBuildingType() const { return type_; }

    // =========================================================================
    // 状态查询接口 - 契约层
    // =========================================================================

    Core::BuildingAnimationState GetState() const { return current_state_; }

    bool IsConstructing() const {
        return current_state_ == Core::BuildingAnimationState::kConstructing;
    }

    bool IsDestroyed() const {
        return current_state_ == Core::BuildingAnimationState::kDestroyed;
    }

    float GetConstructionProgress() const;

    void StartConstruction(float duration);

    // =========================================================================
    // 战斗接口
    // =========================================================================

    bool CanAttack(Core::GeneralType target_type) const {
        return (static_cast<unsigned int>(stats_.target_type_) &
            static_cast<unsigned int>(target_type)) != 0;
    }

    int GetDamage() const { return stats_.damage_; }
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }
    float GetAttackSpeed() const { return stats_.attack_speed_; }

    // =========================================================================
    // 经济接口
    // =========================================================================

    int GetStoredResource() const { return static_cast<int>(stored_resource_); }
    float GetStoragePercentage() const;
    int GetResourceCapacity() const { return stats_.resource_capacity_; }
    int CollectResource(int max_amount = -1);

    // =========================================================================
    // 占地信息 - 契约层
    // =========================================================================

    cocos2d::Rect GetOccupiedRect() const;
    int GetWidthInTiles() const { return stats_.width_; }
    int GetHeightInTiles() const { return stats_.height_; }

    // =========================================================================
    // 动画触发接口
    // =========================================================================

    void PlayAttackAnimation();
    void PlayHurtEffect();
    void PlayDestroyedAnimation();

private:
    Core::BuildingStats stats_;
    Core::BuildingType type_;
    cocos2d::Sprite* visual_sprite_ = nullptr;
    EntityAnimationController* animation_controller_ = nullptr;

    int level_;
    float construction_timer_;
    float construction_duration_;
    Core::BuildingAnimationState current_state_ = Core::BuildingAnimationState::kIdle;
    float stored_resource_ = 0.0f;
    bool obstacle_registered_ = false;

    std::string GetSpriteSheetFilename(Core::BuildingType type);
    void GetFrameSize(Core::BuildingType type, int& out_width, int& out_height);
    void UpdateCombatLogic(float dt);
    Core::ProjectileType GetProjectileTypeFromBuilding(Core::BuildingType type);
    void ProduceResource(float dt);
    void SetState(Core::BuildingAnimationState new_state);
};

#endif // CONTRACT_GAMEPLAY_BUILDING_H_