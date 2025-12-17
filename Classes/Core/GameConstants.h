// GameConstants.h
// This file defines global constants, enums, and architectural agreements
// used throughout the project.

// Vital Agreement: Coordinate System
// * Logic coordinates use integer (x, y) values starting at (0, 0).
// * The origin (0, 0) is the bottom-left tile of the logical grid.
#ifndef GAME_CONSTANTS_H_ 
#define GAME_CONSTANTS_H_
#include <cstdint> // for uint8_t

namespace Core {
    // 1. Grid & World Settings
    // Use constexpr instead of #define for type safety.
    constexpr int kTileWidth = 64;
    constexpr int kTileHeight = 64;
    constexpr int kMapWidth = 40;
    constexpr int kMapHeight = 40;

    // 2. Identification Enums
    enum class BuildingType : uint8_t {
        kTownHall = 0,
        kGoldMine = 1,
        kElixirCollector = 2,
        kGoldStorage = 3,
        kElixirStorage = 4,
        kBarracks = 5,
        kCannon = 6,
        kArcherTower = 7,
        kAirDefense = 8,
        kWall = 9,
        kArmyCamp = 10,
        kNone = 99
    };

    enum class TroopType : uint8_t {
        kBarbarian = 0,
        kArcher = 1,
        kGiant = 2,
        kWallBreaker = 3,
        kBabyDragon = 4
    };

    enum class ProjectileType : uint8_t {
        kArrow = 0,
        kCannonBall = 1,
        kRocket = 2,
        kFireBall = 3,
        kNone = 99
    };
    enum class CampType {
        kPlayer = 0, // 进攻士兵
        kEnemy = 1,  // 敌人建筑
        kNeutral = 2
    };
    // Used to define the type of a unit as a target, and the target type of a building.
    enum class GeneralType : unsigned int {
        kNone = 0,        // 0000
        kGround = 1 << 0,   // 0001 (1)
        kAir = 1 << 1,   // 0010 (2)

        // We combine flags using the OR operator (|)
        kGroundAndAir = kGround | kAir // 0011 (3)
    };

    // Helper to allow using '&' and '|' on the enum class directly
    inline GeneralType operator|(GeneralType a, GeneralType b) {
        return static_cast<GeneralType>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }
    inline bool operator&(GeneralType a, GeneralType b) {
        return (static_cast<unsigned int>(a) & static_cast<unsigned int>(b)) != 0;
    }

// 3. Rendering Layers (Z-Order)
// NOTE: Since this is an enum class, you must static_cast<int> 
// when passing to Cocos2d functions: 
// node->setLocalZOrder(static_cast<int>(ZOrder::kUnits));
enum class ZOrder : uint8_t {
    kGround = 0,
    kDecoration = 10,
    kBuildingBase = 20,
    kShadows = 25,
    kUnits = 30,
    kProjectiles = 40,
    kExplosions = 50,
    kUiHud = 100,
    kUiPopup = 200
};

// 4. Physics/Collision Tags
constexpr int kTagBuilding = 1001;
constexpr int kTagUnit = 1002;
constexpr int kTagProjectile = 1003;

// 5. Animation
enum class BuildingAnimationState : uint8_t {
    kIdle = 0,           
    kConstructing = 1,
    // gameplay   
    kDamaged = 2,        
    kDestroyed = 3
};


enum class UnitAnimationState : uint8_t {
    kIdle = 0,
    kMove = 1,
    kAttack = 2,
    kHit = 3,
    kDead = 4
};

enum class Facing : uint8_t {
    kUp = 0,
    kDown = 1,
    kLeft = 2,
    kRight = 3
};

} // namespace Core

#endif // CORE_GAME_CONSTANTS_H_
