#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

// 1. Grid & World Settings
//TODO(yh): Modify the macros where needed.
#define kTileWidth 64
#define kTileHeight 32  // Isometric ratio usually 2:1
#define kMapWidth 40    // The CoC grid is usually 40x40 or 44x44
#define kMapHeight 40

// 2. Identification Enums (Used by Everyone)
enum class BuildingType {
    kTownHall = 0,
    kGoldMine = 1,
    kElixirCollector = 2,
    kGoldStorage = 3,
    kElixirStorage = 4,
    kBarracks = 5,
    kCannon = 6,
    kArcherTower = 7,
    kAirDefense = 8,  // Optional
    kWall = 9,
    kNone = 99
};

enum class TroopType {
    kBarbarian = 0,
    kArcher = 1,
    kGiant = 2,
    kWallBreaker = 3,
    kBabyDragon = 4 // Optional
};

enum class ProjectileType {
    kArrow = 0,
    kCannonBall = 1,
    kRocket = 2,
    kFireBall = 3,
    kNone = 99 // Melee units
};

// 3. Rendering Layers (Z-Order)
enum ZOrder {
    kZGround = 0,
    kZDecoration = 10,
    kZBuildingBase = 20,
    kZShadows = 25,
    kZUnits = 30,      // Units must appear "in front" or "behind" based on Y-pos
    kZProjectiles = 40,
    kZExplosions = 50,
    kZUiHud = 100,     // Health bars
    kZUiPopup = 200    // Pause Menu
};

// 4. Physics/Collision Tags
#define kTagBuilding 1001
#define kTagUnit 1002
#define kTagProjectile 1003

#endif // GAME_CONSTANTS_H
