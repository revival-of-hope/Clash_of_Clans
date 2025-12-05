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
    UnitStats GameConfig::GetTroopStats(TroopType type, int level) {
        UnitStats stats;
        switch (type) {
            // ---------------------------------------------------------
            // 1. Barbarian (野蛮人): Melee, balanced stats.
            // ---------------------------------------------------------
        case TroopType::kBarbarian:
            stats.max_hp_ = 110 + (level * 15);      // Medium HP
            stats.damage_per_shot_ = 15 + (level * 2);
            stats.attack_speed_ = 1.0f;              // 1.0s/hit
            stats.move_speed_ = 2.5f;                // Medium speed
            stats.housing_space_ = 1;                // 1 space
            stats.range_ = 0.5f;                     // Melee
            stats.favorite_target_ = BuildingType::kNone; // No preference
            stats.projectile_ = ProjectileType::kNone;
            break;

            // ---------------------------------------------------------
            // 2. Archer (弓箭手): Ranged, high damage, very low HP.
            // ---------------------------------------------------------
        case TroopType::kArcher:
            stats.max_hp_ = 45 + (level * 5);        // Very low HP (Glass Cannon)
            stats.damage_per_shot_ = 20 + (level * 3);
            stats.attack_speed_ = 1.0f;
            stats.move_speed_ = 3.5f;                // Fast speed
            stats.housing_space_ = 1;
            stats.range_ = 3.5f;                     // Ranged
            stats.favorite_target_ = BuildingType::kNone;
            stats.projectile_ = ProjectileType::kArrow; // Projectile
            break;

            // ---------------------------------------------------------
            // 3. Giant (巨人): Tank, targets defenses only.
            // ---------------------------------------------------------
        case TroopType::kGiant:
            stats.max_hp_ = 400 + (level * 50);      // High HP (Tank)
            stats.damage_per_shot_ = 25 + (level * 5);
            stats.attack_speed_ = 2.0f;              // Slow attack
            stats.move_speed_ = 1.5f;                // Slow speed
            stats.housing_space_ = 5;                // 5 spaces
            stats.range_ = 1.0f;                     // Melee
            stats.favorite_target_ = BuildingType::kCannon; // Targets Defenses
            stats.projectile_ = ProjectileType::kNone;
            break;

            // ---------------------------------------------------------
            // 4. Wall Breaker (炸弹人): Targets Walls, high speed/fragile.
            // ---------------------------------------------------------
        case TroopType::kWallBreaker:
            stats.max_hp_ = 25 + (level * 5);        // Extremely fragile
            stats.damage_per_shot_ = 40;             // Base damage (x40 vs Walls in logic)
            stats.attack_speed_ = 1.0f;
            stats.move_speed_ = 4.5f;                // Very fast
            stats.housing_space_ = 2;
            stats.range_ = 1.0f;                     // Suicide/Explosion
            stats.favorite_target_ = BuildingType::kWall; // Targets Walls ONLY
            stats.projectile_ = ProjectileType::kNone;
            break;

            // ---------------------------------------------------------
            // 5. Baby Dragon (飞龙宝宝): Air unit, high HP/DMG.
            // ---------------------------------------------------------
        case TroopType::kBabyDragon:
            stats.max_hp_ = 900 + (level * 100);     // High HP
            stats.damage_per_shot_ = 70 + (level * 10); // High damage
            stats.attack_speed_ = 1.5f;
            stats.move_speed_ = 2.5f;
            stats.housing_space_ = 10;               // Large unit (10 spaces)
            stats.range_ = 2.0f;                     // Short ranged
            stats.favorite_target_ = BuildingType::kNone;
            stats.projectile_ = ProjectileType::kFireBall; // Projectile (Fireball)
            break;
        }
        return stats;
    }

    BuildingStats GetBuildingStats(BuildingType type, int level) {
        BuildingStats stats;
        // [NEW] 初始化默认值，防止垃圾数据
        stats.damage_ = 0;
        stats.range_ = 0.0f;
        stats.attack_speed_ = 0.0f;
        stats.resource_capacity_ = 0;
        stats.production_rate_ = 0;
        stats.troop_capacity_ = 0;              // [NEW] 默认人口为0
        stats.target_type_ = TargetType::kNone; // [NEW] 默认不攻击
        switch (type) {
        case BuildingType::kTownHall:
            stats.width_ = 4;
            stats.height_ = 4;
            stats.max_hp_ = 2000 + (level * 500);
            stats.resource_capacity_ = 1000 + (level * 1000); // 存少量资源
            break;

        case BuildingType::kWall:
            stats.width_ = 1;
            stats.height_ = 1;
            stats.max_hp_ = 1000 + (level * 500);
            break;

        case BuildingType::kCannon:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 800 + (level * 100);
            stats.damage_ = 40 + (level * 10);
            stats.attack_speed_ = 1.0f;
            stats.range_ = 7.0f;
            stats.target_type_ = TargetType::kGround; // [NEW] 只对地
            break;

        case BuildingType::kArcherTower: // [NEW] 新增箭塔
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 700 + (level * 80);
            stats.damage_ = 25 + (level * 8);     // 单发伤害低
            stats.attack_speed_ = 0.5f;           // 攻速快
            stats.range_ = 8.0f;                  // 射程远
            stats.target_type_ = TargetType::kGroundAndAir; // [NEW] 对地且对空
            break;

        case BuildingType::kAirDefense: // [NEW] 新增防空火箭
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 900 + (level * 100);
            stats.damage_ = 150 + (level * 20);   // 对空伤害极高
            stats.attack_speed_ = 1.0f;
            stats.range_ = 10.0f;                 // 超远射程
            stats.target_type_ = TargetType::kAir; // [NEW] 只对空
            break;

            // Resources  - [NEW]
        case BuildingType::kGoldMine:
        case BuildingType::kElixirCollector:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 500 + (level * 50);
            stats.production_rate_ = 200 + (level * 50);  // 每小时产量
            stats.resource_capacity_ = 500 + (level * 200); // 暂存容量
            break;

        case BuildingType::kGoldStorage:
        case BuildingType::kElixirStorage:
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 1500 + (level * 200);       // 血厚肉盾
            stats.resource_capacity_ = 5000 + (level * 5000); // 大容量
            break;

            // Military (军事设施) - [NEW]
        case BuildingType::kArmyCamp: // 兵营
            stats.width_ = 5;
            stats.height_ = 5;
            stats.max_hp_ = 600 + (level * 60);
            stats.troop_capacity_ = 20 + (level * 10); // [NEW] 提供人口
            break;

        case BuildingType::kBarracks: // 训练营 用于解锁新兵种和练兵
            stats.width_ = 3;
            stats.height_ = 3;
            stats.max_hp_ = 600 + (level * 60);
            break;

        }
        return stats;
    }
} // namespace Core