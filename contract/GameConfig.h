#pragma once
#include "GameStructs.h"
#include <map>

// TODO(szj): Modify the values here where needed.
class GameConfig {
public:
    // Singleton access
    static GameConfig* GetInstance();

    // The Lookup Functions
    UnitStats GetTroopStats(TroopType type, int level) {
        // Example Logic (You will fill this out)
        UnitStats stats;
        if (type == TroopType::kGiant) {
            stats.max_hp_ = 300 + (level * 50);  // Level scaling
            stats.damage_per_shot_ = 11;
            stats.move_speed_ = 1.5f;            // Slow
            stats.housing_space_ = 5;
            stats.range_ = 1.0f;                 // Melee
            stats.favorite_target_ = BuildingType::kCannon;  // Or generic DEFENSE
        }
        // ... define others ...
        return stats;
    }

    BuildingStats GetBuildingStats(BuildingType type, int level) {
        BuildingStats stats;
        if (type == BuildingType::kTownHall) {
            stats.width_ = 4;   // Size on Grid
            stats.height_ = 4;
            stats.max_hp_ = 1500;
        } else if (type == BuildingType::kWall) {
            stats.width_ = 1;
            stats.height_ = 1;
            stats.max_hp_ = 500;  // Walls are tough!
        }
        return stats;
    }
    
    // Resource Costs
    int GetUpgradeCost(BuildingType type, int level) {
        // Return gold/elixir cost
        return 500 * level; 
    }
};
