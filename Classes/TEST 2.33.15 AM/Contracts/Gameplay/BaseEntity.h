// Source: Classes/Integration/Docs/Gameplay Module API Reference.md.md (BaseEntity)
// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
// Base class for all game entities (Units, Buildings).
// Inherits from cocos2d::Node to support rendering and hierarchy.

#ifndef GAMEPLAY_ENTITIES_BASE_ENTITY_H_
#define GAMEPLAY_ENTITIES_BASE_ENTITY_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"

/**
 * @brief Base class for game entities
 * All objects in the scene (buildings, units) should inherit from this class.
 * Provides ID management, faction management, and lifecycle flags.
 */
class BaseEntity : public cocos2d::Node {
public:
    BaseEntity();
    virtual ~BaseEntity();

    // Core initialization
    virtual bool init() override;

    // [New] Lifecycle management, used to maintain the global registry
    virtual void onEnter() override;
    virtual void onExit() override;

    /**
     * @brief Logic update for every frame
     * override indicates: I want to override update(float dt) in cocos2d::Node
     * @param dt Time interval from the last frame to the current frame (seconds)
     */
    virtual void update(float dt) override;

    // Unique instance ID
    void set_instance_id(int id) { instance_id_ = id; }
    int get_instance_id() const { return instance_id_; }

    void set_owner_id(int id) { owner_id_ = id; }
    int get_owner_id() const { return owner_id_; }

    /**
     * @brief Check if the target is an ally
     * Determined based on whether the owner_ids are the same.
     * @param other Another entity
     * @return true If they belong to the same faction (same ID)
     */
    bool IsAlly(const BaseEntity* other) const {
        if (!other) return false;
        return this->owner_id_ == other->get_owner_id();
    }

    /**
     * @brief Mark for destruction
     * The entity will not disappear immediately, but will be safely removed in the next frame update to prevent iterator invalidation.
     */
    void MarkForDestruction() { is_marked_for_destruction_ = true; }

    /**
     * @brief Check if marked for destruction
     */
    bool IsMarkedForDestruction() const { return is_marked_for_destruction_; }

    /**
     * @brief Get center point world coordinates
     * Convenient for calculating distance and alignment judgment.
     */
    cocos2d::Vec2 GetCenterPosition() const;

    // [New] Get all active BaseEntities in the current scene
    // Solves ownership issues for "UI needs a list" or "Logic needs to iterate targets"
    static cocos2d::Vector<BaseEntity*>& GetAllEntities() { return global_entities_; }

protected:
    int instance_id_;                 ///< Unique ID
    bool is_marked_for_destruction_;  ///< Destruction flag
    int owner_id_;                    ///< Owner ID

private:
    // [Core] Authoritative entity registry
    // Only BaseEntity itself can add/remove, external access is read-only
    static cocos2d::Vector<BaseEntity*> global_entities_;
};

#endif // GAMEPLAY_ENTITIES_BASE_ENTITY_H_
