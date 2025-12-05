// GameStructs.h
// Pure Data Structures (PODs) for communication between Systems.
#ifndef GAME_STRUCTS_H
#define GAME_STRUCTS_H

#include "GameConstants.h"
#include <string>

namespace Core {
// 1. The Save Data
struct BuildingData {
    int instance_id_;          // Unique ID (e.g., 1001)
    BuildingType type_;        // What is it?
    int level_;                // 1, 2, or 3
    int grid_x_;               // Logic Coordinate (0–39), NOT screen pixel X
    int grid_y_;               // Logic Coordinate (0–39)
    bool is_constructing_;     // Is it currently upgrading?
    long finish_time_;         // Timestamp for when upgrade finishes
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
    GeneralType unit_type_;      // Its own type as a target
};

struct BuildingStats {
    int max_hp_;
    int damage_;              // 0 for non-defenses
    float range_;
    float attack_speed_;
    int width_;               // Size in tiles
    int height_;
    int resource_capacity_;   // Storage capacity
    int troop_capacity_;      // [NEW] Specifically for Army Camp
    int production_rate_;     // Per hour
    GeneralType target_type_;  // [NEW] 
};
} // namespace Core

#endif  // GAME_STRUCTS_H
