// GameConstants.h
// This file defines global constants, enums, and architectural agreements
// used throughout the project.

// Vital Agreement: Coordinate System
// * Logic coordinates use integer (x, y) values starting at (0, 0).
// * The origin (0, 0) is the bottom-left tile of the logical grid.
#ifndef CORE_GAME_CONSTANTS_H_ 
#define CORE_GAME_CONSTANTS_H_
#include <type_traits> // Required to enable bitwise operators on enum class

namespace Core {
// 1. Grid & World Settings
// Use constexpr instead of #define for type safety.
constexpr int kTileWidth = 64;
constexpr int kTileHeight = 32;
constexpr int kMapWidth = 40;
constexpr int kMapHeight = 40;

// 2. Identification Enums
enum class BuildingType {
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

enum class TroopType {
    kBarbarian = 0,
    kArcher = 1,
    kGiant = 2,
    kWallBreaker = 3,
    kBabyDragon = 4
};

enum class ProjectileType {
    kArrow = 0,
    kCannonBall = 1,
    kRocket = 2,
    kFireBall = 3,
    kNone = 99
};

// Used to define the type of a unit as a target, and the target type of a building.
enum class GeneralType : unsigned int {
    kNone   = 0,        // 0000
    kGround = 1 << 0,   // 0001 (1)
    kAir    = 1 << 1,   // 0010 (2)
    
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
enum class ZOrder {
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

} // namespace Core

#endif // CORE_GAME_CONSTANTS_H_