# Core Source of Truth (SOT)

`Classes/Core` is the **authoritative, read-only source of shared game data** (constants, enums, structs, tunables). Every module MUST pull common values from here to prevent drift, conflicting IDs, or mismatched balance numbers.

## Purpose and scope
- Centralize globally shared identifiers and configuration to **eliminate hardcoded duplicates**.
- Define stable shapes (structs/enums) so UI, integration, and Gameplay speak the same language.
- Core is not business logic; it is the data contract the rest of the project consumes.

## What belongs here (authoritative only)
- Canonical constants/identifiers used across features (`GameConstants.h`).
- Shared data structures that model cross-cutting game entities (`GameStructs.h`).
- Tunables and lookup tables surfaced through the config layer (`GameConfig.h` / `GameConfig.cpp`).

## What does NOT belong here
- Feature-specific logic or helper functions.
- UI-only constants, screen/layout values, or scene wiring.
- One-off values used in a single module.
- Private structs/types intended to live only inside a feature or test.

## How to depend on Core
- Treat Core as **read-only contract**: other modules MUST NOT redefine or shadow its constants, enums, or structs locally.
- Include the needed headers instead of copying values (e.g., `#include "Classes/Core/GameConstants.h"`).
- Query `GameConfig` for tunables; do not hardcode numbers, IDs, or asset names that already exist here.
- If a value is missing, add it to Core first, then update callers to consume it.

## Change workflow checklist
1. Add or update the canonical constant/struct/tunable in the appropriate Core file.
2. Prefer extending existing enums/structs before introducing new types.
3. Update all call sites to **read from Core** instead of keeping local copies.
4. Remove duplicated literals/definitions elsewhere after the Core change.
5. Keep Core entries neutral and cross-feature (no feature-specific helpers).

## Examples
**Bad (hardcoded/duplicated):**
```cpp
// In a UI controller
const int kArcherCost = 50;  // drift risk if balance changes
```

**Good (Core-based):**
```cpp
#include "Classes/Core/GameConfig.h"

const int archer_cost = GameConfig::GetTroopCost(TroopType::kArcher);
```
