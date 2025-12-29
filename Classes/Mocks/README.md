
# Mocks (Deterministic Test Doubles)

## Purpose
Mocks provide deterministic, in-memory stand-ins for Engine/Gameplay so Dev C can integrate
without the real engine or runtime assets.

## Determinism Rules
* **No randomness** unless a fixed seed is explicitly documented.
* **No wall-clock time** dependencies.
* All state transitions must be reproducible across runs.

## Underspecified Behavior Policy
If behavior is not specified by docs or Contract:
1. Choose a **simple mock-only default** behavior.
2. Document the default in this README.
3. Add a question to `Classes/Contract/CONTRACT_GAPS.md`.

Current mock-only defaults:
* `Building::CollectResource(max)` returns up to `max` from stored resource, then decrements storage. Resource buildings start with 50% of capacity and produce at a fixed linear rate capped by capacity.
* `Unit::GetDamage()` returns `level * 8` (minimum 8 if level <= 0). `CanAttack` respects `GeneralType` bitmasks; Baby Dragon counts as `kAir`.
* `Unit::GetOccupiedRect()` returns a circle-based bounding box sized by the collision radius (default 20px).
* `CostQuery` formulas are deterministic: building placement = gold `100 * level` (elixir half of gold), building upgrade = gold `150 * (level+1)` (+50 for Cannons), troop training = elixir `25 * level`, population = 1, with simple time estimates per API.
* `SaveLoadService::SaveSnapshot()` returns the last loaded blob (default: "MOCK_SNAPSHOT").
* `SaveLoadService::LoadSnapshot(blob)` returns false when blob is empty; otherwise updates the stored blob and increments the determinism tick.
* `MapLayer` defaults to 64-unit tiles (matching Core grid constants) and no blocked coordinates until `SetBlockedTilesForTesting` is called.
* `TilePlacementController::SnapToValidTile` snaps using the attached map; `CanPlaceAt` returns false when the snapped tile is in the blocked set.
* `InputRouter` returns `false` when UI consumption is enabled; otherwise it snaps to the map grid and enables the attached highlighter.
* `SceneFlowService` transitions through Boot → Menu → Game → Results and stores the last launch/results parameters.
* `GameEventManager::GetLastBattleEnded` returns the most recently broadcast `BattleEndEvent` (including duration, troop counts, and spells used).

## CocosShim Policy (Stub-Only)
Mocks must compile against the stubbed Cocos boundary in `Classes/Mocks/CocosShim/**`.
**Do not** include or link the real Cocos engine in mock builds/tests.

## Mock-only build contract
Tests are intended to run against the mock-only configuration:
- `USE_COCOS_ENGINE=OFF`
- `BUILD_APP=OFF`
- `BUILD_TESTS=ON`

In this mode, `mock_tests` should link only mock/contract surfaces (`Classes/Mocks/**`, `Classes/Contract/**`, `Classes/Integration/GameServices.cpp`) and should not require real engine assets or gameplay logic.

Smoke coverage for `GameServices` relies on the mocks shipped in this repo; no external downloads or additional libraries are required beyond the standard toolchain.

## Canonical Build/Test Commands
Use repo-standard flags to keep the build isolated from the real engine:

```bash
cmake -S . -B build -DUSE_COCOS_ENGINE=OFF -DBUILD_APP=OFF -DBUILD_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
````

## Isolation Verification Commands

Use these to prove `mock_tests` does **not** link to the real engine:

```bash
cmake --build build --target mock_tests --verbose
ldd build/mock_tests || true
readelf -d build/mock_tests | grep -E 'NEEDED|RPATH|RUNPATH' || true
strings build/mock_tests | grep -iE 'cocos|cocos2d|engine' || true
```

Note: seeing `cocos2d::` symbol names in `strings` output is expected because the shim defines
those types; it does **not** indicate linkage to the real engine.

```
