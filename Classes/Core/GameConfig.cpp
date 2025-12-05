// GameConfig.cpp
// Used for balance tuning.
// Hardcoded for Compilation Safety, Speed, and the "Flyweight" concept.
#include "GameConfig.h"

namespace Core {

    // 1. The Singleton Implementation
    GameConfig* GameConfig::GetInstance() {
        static GameConfig instance;
        return &instance;
    }

    // 2. The Logic Implementation
    UnitStats GameConfig::GetTroopStats(TroopType type, int level) {
        UnitStats stats;
        
        // Default initialization
        stats.unit_type_ = GeneralType::kGround; 
        
        switch (type) {
        // ---------------------------------------------------------
        // Ground Units
        // ---------------------------------------------------------
        case TroopType::kBarbarian:
            stats.max_hp_ = 110 + (level * 15);
            stats.damage_per_shot_ = 15 + (level * 2);
            stats.attack_speed_ = 1.0f;
            stats.move_speed_ = 2.5f;
            stats.housing_space_ = 1;
            stats.range_ = 0.5f;
            stats.favorite_target_ = BuildingType::kNone;
            stats.projectile_ = ProjectileType::kNone;
            stats.unit_type_ = GeneralType::kGround; // IS Ground
            break;

        case TroopType::kArcher:
            stats.max_hp_ = 45 + (level * 5);
            stats.damage_per_shot_ = 20 + (level * 3);
            stats.attack_speed_ = 1.0f;
            stats.move_speed_ = 3.5f;
            stats.housing_space_ = 1;
            stats.range_ = 3.5f;
            stats.favorite_target_ = BuildingType::kNone;
            stats.projectile_ = ProjectileType::kArrow;
            stats.unit_type_ = GeneralType::kGround; // IS Ground
            break;

        case TroopType::kGiant:
            stats.max_hp_ = 400 + (level * 50);
            stats.damage_per_shot_ = 25 + (level * 5);
            stats.attack_speed_ = 2.0f;
            stats.move_speed_ = 1.5f;
            stats.housing_space_ = 5;
            stats.range_ = 1.0f;
            stats.favorite_target_ = BuildingType::kCannon;
            stats.projectile_ = ProjectileType::kNone;
            stats.unit_type_ = GeneralType::kGround; // IS Ground
            break;

        case TroopType::kWallBreaker:
            stats.max_hp_ = 25 + (level * 5);
            stats.damage_per_shot_ = 40;
            stats.attack_speed_ = 1.0f;
            stats.move_speed_ = 4.5f;
            stats.housing_space_ = 2;
            stats.range_ = 1.0f;
            stats.favorite_target_ = BuildingType::kWall;
            stats.projectile_ = ProjectileType::kNone;
            stats.unit_type_ = GeneralType::kGround; // IS Ground
            break;

        // ---------------------------------------------------------
        // Air Units
        // ---------------------------------------------------------
        case TroopType::kBabyDragon:
            stats.max_hp_ = 900 + (level * 100);
            stats.damage_per_shot_ = 70 + (level * 10);
            stats.attack_speed_ = 1.5f;
            stats.move_speed_ = 2.5f;
            stats.housing_space_ = 10;
            stats.range_ = 2.0f;
            stats.favorite_target_ = BuildingType::kNone;
            stats.projectile_ = ProjectileType::kFireBall;
            stats.unit_type_ = GeneralType::kAir;   // IS Air
            break;
        }
        return stats;
    }

    BuildingStats GameConfig::GetBuildingStats(BuildingType type, int level) {
        BuildingStats stats;
        
        // Initialize defaults
        stats.damage_ = 0;
        stats.range_ = 0.0f;
        stats.attack_speed_ = 0.0f;
        stats.resource_capacity_ = 0;
        stats.production_rate_ = 0;
        stats.troop_capacity_ = 0;           
        stats.target_type_ = GeneralType::kNone; // Default: Attacks nothing

        switch (type) {
        // ---------------------------------------------------------
        // Passive Buildings
        // ---------------------------------------------------------
        case BuildingType::kTownHall:
            stats.width_ = 4;
            stats.height_ = 4;
            stats.max_hp_ = 2000 + (level * 500);
            stats.resource_capacity_ = 1000 + (level * 1000);
            break;

        case BuildingType::kWall:
            stats.width_ = 1;
            stats.height_ = 1;
            stats.max_hp_ = 1000 + (level * 500);
            break;

        // ---------------------------------------------------------
        // Defensive Buildings
        // ---------------------------------------------------------
        case BuildingType::kCannon:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 800 + (level * 100);
            stats.damage_ = 40 + (level * 10);
            stats.attack_speed_ = 1.0f;
            stats.range_ = 7.0f;
            stats.target_type_ = GeneralType::kGround; // TARGETS Ground
            break;

        case BuildingType::kArcherTower:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 700 + (level * 80);
            stats.damage_ = 25 + (level * 8);
            stats.attack_speed_ = 0.5f;
            stats.range_ = 8.0f;
            // [BITMASK MAGIC] Targets Ground OR Air (1 | 2 = 3)
            stats.target_type_ = GeneralType::kGround | GeneralType::kAir; 
            break;

        case BuildingType::kAirDefense:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 900 + (level * 100);
            stats.damage_ = 150 + (level * 20);
            stats.attack_speed_ = 1.0f;
            stats.range_ = 10.0f;
            stats.target_type_ = GeneralType::kAir; // TARGETS Air
            break;

        // ---------------------------------------------------------
        // Economy / Army
        // ---------------------------------------------------------
        case BuildingType::kGoldMine:
        case BuildingType::kElixirCollector:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 500 + (level * 50);
            stats.production_rate_ = 200 + (level * 50);
            stats.resource_capacity_ = 500 + (level * 200);
            break;

        case BuildingType::kGoldStorage:
        case BuildingType::kElixirStorage:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 1500 + (level * 200);
            stats.resource_capacity_ = 5000 + (level * 5000);
            break;

        case BuildingType::kArmyCamp:
            stats.width_ = 5;
            stats.height_ = 5;
            stats.max_hp_ = 600 + (level * 60);
            stats.troop_capacity_ = 20 + (level * 10);
            break;

        case BuildingType::kBarracks:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 600 + (level * 60);
            break;
        }
        return stats;
    }
} // namespace Core