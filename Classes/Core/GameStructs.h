// GameStructs.h
// Pure Data Structures (PODs) for communication between Systems.
#ifndef GAME_STRUCTS_H
#define GAME_STRUCTS_H

#include "GameConstants.h"
#include <string>

namespace Core {

// Hitbox PODs
// Axis-aligned box definition relative to an entity's "origin".
// For Units: origin is typically (x_, y_) in world pixels.
// For Buildings: origin is typically the world position of (grid_x_, grid_y_)
// converted using tile size (e.g., grid * kTileSize).
struct HitboxDef {
    float offset_x_;   // Offset from entity origin to hitbox top-left (world units)
    float offset_y_;
    float width_;      // Hitbox size in world units (pixels)
    float height_;
};

// Explicit world-space AABB (computed/cached).
struct AABB {
    float x_;          // Top-left x in world units (pixels)
    float y_;          // Top-left y
    float width_;
    float height_;
};

// 1. The Save Data
struct BuildingData {
    int instance_id_;          // Unique ID (e.g., 1001)
    int owner_id_;             // The Player ID of the building's owner. Used to distinguish enemies and allies.

    BuildingType type_;        // What is it?
    int level_;                // 1, 2, or 3

    int grid_x_;               // Logic Coordinate (0–39), NOT screen pixel X
    int grid_y_;               // Logic Coordinate (0–39)

    // Logic State
    bool is_constructing_;     // Is it currently upgrading?
    long finish_time_;         // Timestamp for when upgrade finishes

    // Animation
    BuildingAnimationState animation_state_;

    // Hitbox (optional cached world-space; useful for selection/combat queries)
    AABB hitbox_world_;
};

struct UnitData {
    int instance_id_;       // Unique ID for this specific unit
    int owner_id_;          // The Player ID of the unit's owner. Used to distinguish enemies and allies.

    TroopType type_;        // To look up the UnitStats blueprint
    int level_;             // To look up the correct stats row

    float x_;               // Exact float position (pixel precision)
    float y_;

    int current_hp_;        // Current health (mutable)

    // Logic State
    int target_id_;         // Instance ID of what it is attacking (-1 if none)
    bool is_alive_;         // Simple flag for cleanup

    // Animation
    Facing direction_;
    UnitAnimationState animation_state_;

    // Hitbox (optional cached world-space; useful for projectile collision, melee range, clicks)
    AABB hitbox_world_;
};

// 2. The Combat Stats
struct UnitStats {
    int max_hp_;
    int damage_per_shot_;
    float attack_speed_;           // Seconds between attacks (e.g., 1.0f)
    float move_speed_;             // Tiles per second
    float range_;                  // Attack range in tiles
    int housing_space_;            // How much capacity it takes (e.g., Giant = 5)
    BuildingType favorite_target_; // Logic: Giant targets defense
    ProjectileType projectile_;    // What does it shoot?
    GeneralType unit_type_;        // Its own type as a target

    // Hitbox blueprint (world units/pixels).
    // Used to compute UnitData::hitbox_world_ from (x_, y_).
    HitboxDef hitbox_def_;
};

struct BuildingStats {
    int max_hp_;
    int damage_;               // 0 for non-defenses
    float range_;
    float attack_speed_;
    int width_;                // Size in tiles
    int height_;
    int resource_capacity_;    // Storage capacity
    int troop_capacity_;       // Specifically for Army Camp
    int production_rate_;      // Per hour
    GeneralType target_type_;

    // Hitbox blueprint (world units/pixels).
    // Used to compute BuildingData::hitbox_world_ from (grid_x_, grid_y_) converted to world pos.
    HitboxDef hitbox_def_;
};

} // namespace Core

#endif  // GAME_STRUCTS_H
