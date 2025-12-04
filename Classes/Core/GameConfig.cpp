// GameConfig.cpp
// Used for balance tuning.
// Hardcoded (instead of using individual classes) for Compilation Safety, Speed, and the "Flyweight" concept.
#include "GameConfig.h"

namespace Core {

    // 1. The Singleton Implementation (Meyers Singleton)
    GameConfig* GameConfig::GetInstance() { 
        static GameConfig instance;
        return &instance;
    }

    // 2. The Logic Implementation
    UnitStats GameConfig::GetTroopStats(TroopType type, int level)  {
        UnitStats stats;
        if (type == TroopType::kGiant) {
            stats.max_hp_ = 300 + (level * 50);  // Level scaling
            stats.damage_per_shot_ = 11;
            stats.move_speed_ = 1.5f;            // Slow
            stats.housing_space_ = 5;
            stats.range_ = 1.0f;                 // Melee
            stats.favorite_target_ = BuildingType::kCannon;  // Or generic DEFENSE
        }
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
            stats.max_hp_ = 500;
        }
        return stats;
    }
} // namespace Core