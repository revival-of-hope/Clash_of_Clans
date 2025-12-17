### **Update Log2: Combat Logic Refactor**


Date: 2025-12-17  
Author: dev C (full stack)
Module: Core
Affected Module: GamePlay, Engine

**Core Changes:**

- Added hitbox support
- Added Full Animation Support for both engine and gameplay
- Added uint_8 to improve Cache friendliness


### Hitbox Support Explanation

**Summary**
Introduced a data-driven hitbox system for Units and Buildings to support collision, combat, and selection logic.

**Details**

* Added `HitboxDef` (blueprint) to `UnitStats` and `BuildingStats` for defining hitbox size and offsets per type/level.
* Added optional `AABB` (Axis-Aligned Bounding Box) to `UnitData` and `BuildingData` as a cached, world-space hitbox.
* Enables fast collision checks, projectile hits, and interaction queries without per-frame recomputation.

**Impact**

* Improves performance and clarity in combat and collision systems.
* Maintains POD-only structures and system decoupling.

---
