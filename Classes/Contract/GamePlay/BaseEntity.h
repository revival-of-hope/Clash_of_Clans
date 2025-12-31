// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Base class for all game entities (Units, Buildings).
// Inherits from cocos2d::Node to support rendering and hierarchy.
//
// Path: Classes/Contract/Gameplay/BaseEntity.h

#ifndef CONTRACT_GAMEPLAY_BASE_ENTITY_H_
#define CONTRACT_GAMEPLAY_BASE_ENTITY_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"

/**
 * @brief Base Class for Game Entities
 * All objects in the scene (Buildings, Units) should inherit from this class.
 * Provides ID management, faction management, and lifecycle tagging.
 */
class BaseEntity : public cocos2d::Node {
public:
    BaseEntity();
    virtual ~BaseEntity();

    // Core Initialization
    virtual bool init() override;

    // Lifecycle Management, used to maintain global registry
    virtual void onEnter() override;
    virtual void onExit() override;

    /**
     * @brief Logic update for each frame
     * @param dt Time interval from previous frame to current frame (seconds)
     */
    virtual void update(float dt) override;

    // ==========================================================================
    // ID Management
    // ==========================================================================

    void set_instance_id(int id) { instance_id_ = id; }
    int get_instance_id() const { return instance_id_; }

    void set_owner_id(int id) { owner_id_ = id; }
    int get_owner_id() const { return owner_id_; }

    // ==========================================================================
    // Faction Check
    // ==========================================================================

    /**
     * @brief Determine if target is an ally
     * @param other Another entity
     * @return true if same faction (ID is same)
     */
    bool IsAlly(const BaseEntity* other) const {
        if (!other) return false;
        return this->owner_id_ == other->get_owner_id();
    }

    // ==========================================================================
    // Lifecycle Management
    // ==========================================================================

    /**
     * @brief Mark for destruction
     * Entity will not disappear immediately, but will be safely removed in the next update.
     */
    void MarkForDestruction() { is_marked_for_destruction_ = true; }

    /**
     * @brief Check if marked for destruction
     */
    bool IsMarkedForDestruction() const { return is_marked_for_destruction_; }

    /**
     * @brief Get center point world coordinates
     */
    cocos2d::Vec2 GetCenterPosition() const;

    // ==========================================================================
    // Global Entity Registry
    // ==========================================================================

    /**
     * @brief Get all active BaseEntity in the current scene
     */
    static cocos2d::Vector<BaseEntity*>& GetAllEntities() { return global_entities_; }

protected:
    int instance_id_ = -1;            ///< Unique ID
    int owner_id_ = 0;                ///< Owner ID (0=Player, 1=Enemy)
    bool is_marked_for_destruction_ = false;  ///< Destruction Flag

private:
    static cocos2d::Vector<BaseEntity*> global_entities_;
};

#endif // CONTRACT_GAMEPLAY_BASE_ENTITY_H_