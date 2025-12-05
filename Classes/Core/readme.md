# **Core — README**

This small module defines the foundational data structures, constants, and configuration helpers used by the game engine.

---

## **File Overview**

### **1. `GameConstants.h`**

This header defines all core, global constants and enumerations used throughout the engine:

* **Grid and world constants** (`kTileWidth`, `kMapHeight`, etc.)
* **Strongly typed enums** (`enum class`) for units, buildings, projectiles, and rendering layers
* **Physics/collision tags** for hit detection

All enumerators use the Google style **`kPascalCase`** naming for constants, e.g.:

```cpp
enum class BuildingType {
    kTownHall,
    kGoldMine,
    kElixirStorage,
    ...
};
```

This file acts as the "authoritative source" for identifiers used universally across the engine.

---

### **2. `GameStructs.h`**

Defines the core **data structures** used during gameplay:

* `BuildingData` – Represents buildings as saved in a player’s village
* `UnitStats` – Combat statistics for troops
* `BuildingStats` – Combat/resource statistics for buildings

All struct **member variables follow Google style field naming**:

```
snake_case_   // with trailing underscore
```

Example:

```cpp
int max_hp_;
float attack_speed_;
BuildingType favorite_target_;
```

---

### **3. `GameConfig.h`**

Provides configuration and stat lookup behavior:

* Implements the **singleton** access point:

  ```cpp
  static GameConfig* GetInstance();
  ```
* Provides **lookup functions** that return fully populated stat structs:

  * `GetTroopStats(type, level)`
  * `GetBuildingStats(type, level)`
  * `GetUpgradeCost(type, level)`

Uses the renamed struct fields from `GameStructs.h`:

```cpp
stats.max_hp_ = 300 + (level * 50);
stats.move_speed_ = 1.5f;
stats.favorite_target_ = BuildingType::kCannon;
```

This file centralizes all balancing values, making tuning predictable and consistent.
