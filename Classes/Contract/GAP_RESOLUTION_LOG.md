# Gap Resolution Log

This log tracks resolved and unresolved contract gaps for Dev C integration.

## GAP-001 — Gameplay include path standard
**Decision summary:** Use Contract include paths (`Classes/Contract/Gameplay/...`) as the canonical public API for Dev C.  
**Options considered:**  
- Keep `Gameplay/Public/...` (legacy path in doc)  
- Use `Public/Gameplay/...` (integration example path)  
- Use `Classes/Contract/Gameplay/...` (**chosen**)  
**SOT docs updated:**  
- `Classes/Integration/Docs/Gameplay Module API Reference.md.md` (Physical Architecture & Public Headers; Integration Example includes)  
**Contract headers changed:** None (documentation-only change).  
**Mock changes:** None.  
**Migration notes:** Replace `Gameplay/Public/...` and `Public/Gameplay/...` includes with `Classes/Contract/Gameplay/...`.  
**Verification:** (see end of log)

## GAP-002 — Unit level/bounds + HealthComp HP accessors (also GAP-DEV-C-004)
**Decision summary:** Add `Unit::GetLevel()` and `Unit::GetOccupiedRect()` for UI hit-testing; add `HealthComp::GetCurrentHP()` / `GetMaxHP()` for absolute HP display.  
**Options considered:**  
- Add level/bounds to `Unit` and HP getters to `HealthComp` (**chosen**)  
- Add shared bounds/HP getters to `BaseEntity`  
- Require UI to infer bounds or poll via events only  
**SOT docs updated:**
- `Classes/Integration/Docs/Gameplay Module API Reference.md.md` (Unit + HealthComp sections)
**Dev C mapping:** Resolves `GAP-DEV-C-004` from the Dev C spec.
**Contract headers changed:**  
- `Classes/Contract/Gameplay/Unit.h`  
- `Classes/Contract/Gameplay/HealthComp.h`  
**Mock changes:**  
- `Classes/Mocks/GameplayMock/Unit.cpp`  
- `Classes/Mocks/GameplayMock/HealthComp.cpp`  
- `Classes/Mocks/README.md` (mock default for `Unit::GetOccupiedRect()`)  
**Migration notes:** None.  
**Verification:** (see end of log)

## GAP-003 — EntitySpawnEvent includes initial HP (also GAP-DEV-C-005)
**Decision summary:** Add `current_hp` and `max_hp` to `EntitySpawnEvent` so UI can initialize health bars without a follow-up query.  
**Options considered:**  
- Add `current_hp`/`max_hp` to `EntitySpawnEvent` (**chosen**)  
- Require immediate `OnEntityDamaged` broadcast after spawn  
- Require UI to query a HealthComp immediately after spawn  
**SOT docs updated:**
- `Classes/Integration/Docs/Gameplay_Public_GameEvents_Documentation.md` (EntitySpawnEvent payload)
**Dev C mapping:** Resolves `GAP-DEV-C-005` from the Dev C spec.
**Contract headers changed:**  
- `Classes/Contract/Gameplay/GameEvents.h`  
**Mock changes:** None (event is broadcast-only).  
**Migration notes:** None.  
**Verification:** (see end of log)

## GAP-004 — BaseEntity documented in SOT
**Decision summary:** Document `BaseEntity` as the common entity base class for `Unit` and `Building`, including lifecycle and ID APIs.  
**Options considered:**  
- Document `BaseEntity` in SOT (**chosen**)  
- Remove `BaseEntity` from Contract and use `cocos2d::Node` directly  
- Keep `BaseEntity` as an undocumented internal header  
**SOT docs updated:**  
- `Classes/Integration/Docs/Gameplay Module API Reference.md.md` (BaseEntity section; Unit/Building inheritance)  
**Contract headers changed:**  
- `Classes/Contract/Gameplay/BaseEntity.h` (traceability comment)  
**Mock changes:** None (mock already exists).  
**Migration notes:** None.  
**Verification:** (see end of log)

## GAP-DEV-C-003 — Cost query APIs for shop/deploy
**Decision summary:** Introduce `CostQuery` with read-only cost queries for building placement and troop training.  
**SOT docs updated:**  
- `Classes/Integration/Docs/Gameplay Cost Query API.md` (CostQuery)  
**Contract headers changed:**  
- `Classes/Contract/Gameplay/CostQuery.h`  
**Mock changes:**  
- `Classes/Mocks/GameplayMock/CostQuery.cpp`  
- `Classes/Mocks/README.md` (default cost formulas)  
**Migration notes:** None.  
**Verification:** (see end of log)

## GAP-DEV-C-008 — Save/Load snapshots
**Decision summary:** Add `SaveLoadService` with opaque snapshot save/load APIs.  
**SOT docs updated:**  
- `Classes/Integration/Docs/SaveLoad and Determinism API.md` (SaveLoadService)  
**Contract headers changed:**  
- `Classes/Contract/Integration/SaveLoadService.h`  
**Mock changes:**  
- `Classes/Mocks/IntegrationMock/SaveLoadService.cpp`  
- `Classes/Mocks/README.md` (snapshot defaults)  
**Migration notes:** None.  
**Verification:** (see end of log)

## GAP-DEV-C-009 — JSON policy for save/load
**Decision summary:** Declare snapshot blobs as opaque bytes/strings; JSON is **not required**.
**SOT docs updated:**
- `Classes/Integration/Docs/SaveLoad and Determinism API.md` (Semantics)
**Contract headers referenced:**
- `Classes/Contract/Integration/SaveLoadService.h` (policy enforced; no change required)
**Mock changes:** None.
**Migration notes:** None.
**Verification:** (see end of log)

## GAP-DEV-C-010 — Determinism hooks (tick + hash)
**Decision summary:** Add `GetDeterminismTick()` and `GetStateHash()` to `SaveLoadService` for read-only verification.
**SOT docs updated:**
- `Classes/Integration/Docs/SaveLoad and Determinism API.md` (SaveLoadService)
**Contract headers changed:**
- `Classes/Contract/Integration/SaveLoadService.h`
**Mock changes:**
- `Classes/Mocks/IntegrationMock/SaveLoadService.cpp`
**Migration notes:** None.
**Verification:** (see end of log)

## GAP-DEV-C-001 — Scene flow entry + boot/menu/game/results pipeline
**Decision summary:** Define `Integration::SceneFlowService` to drive Boot → Menu → Game → Results with deterministic stage tracking and cached launch/result parameters.
**SOT docs updated:**
- `Classes/Integration/Docs/Scene Flow and Entrypoint.md`
- `Classes/Integration/Docs/DevC_Spec.md` (canonical scene flow)
**Contract headers changed:**
- `Classes/Contract/Integration/SceneFlowService.h`
**Mock changes:**
- `Classes/Mocks/IntegrationMock/SceneFlowService.cpp`
**Migration notes:** AppDelegate/test harness should construct the service via `CreateSceneFlowService()` and drive transitions explicitly.
**Verification:** (see end of log)

## GAP-DEV-C-002 — Placement validation + TMX conventions
**Decision summary:** Standardize a Core-aligned 64-unit grid, `collision` layer with `blocked=true` / `walkable=false`, and expose validation helpers on `MapLayer` + `TilePlacementController`.
**SOT docs updated:**
- `Classes/Integration/Docs/Placement Validation and TMX Conventions.md`
- `Classes/Integration/Docs/Engine Module API Reference.md` (placement validation section)
**Contract headers changed:**
- `Classes/Contract/Engine/MapLayer.h`
- `Classes/Contract/Engine/TilePlacementController.h`
**Mock changes:**
- `Classes/Mocks/EngineMock/MapLayer.cpp`
- `Classes/Mocks/EngineMock/TilePlacementController.cpp`
**Core alignment correction:** Default tile size now uses `Core::kTileWidth`/`kTileHeight` (64) to match `Classes/Core/GameConstants.h`.
**Migration notes:** Maps should mark blocked tiles on the `collision` layer; UI should call `SnapToValidTile` + `CanPlaceAt` before placement.
**Verification:** (see end of log)

## GAP-DEV-C-006 — Input routing policy
**Decision summary:** Add `InputRouter` to enforce UI-first pointer handling with optional map/highlighter routing and deterministic last-route state.
**SOT docs updated:**
- `Classes/Integration/Docs/Input Routing Policy.md`
**Contract headers changed:**
- `Classes/Contract/Engine/InputRouter.h`
**Mock changes:**
- `Classes/Mocks/EngineMock/InputRouter.cpp`
**Migration notes:** Scenes attach the active `MapLayer`/`TileHighlighter`, toggle UI capture with `SetUiConsumesInput`, and use boolean returns to decide if the map handled the event.
**Verification:** (see end of log)

## GAP-DEV-C-007 — Results payload for settlement screen
**Decision summary:** Extend `BattleEndEvent` with duration, troop counts, and spells used; cache the latest event via `GameEventManager::GetLastBattleEnded` for results scene consumption.
**SOT docs updated:**
- `Classes/Integration/Docs/Gameplay_Public_GameEvents_Documentation.md` (BattleEndEvent)
**Contract headers changed:**
- `Classes/Contract/Gameplay/GameEvents.h`
**Mock changes:**
- `Classes/Mocks/GameplayMock/GameEvents.cpp`
**Migration notes:** Results UI should read from the broadcasted event or fetch the cached event if a listener was not registered in time.
**Verification:** (see end of log)

---

## Reconciliation (Log vs Trackers)

| Gap ID | In Log? | In CONTRACT_GAPS? | In DevC_Spec (SOT)? | Verified? | Evidence |
| --- | --- | --- | --- | --- | --- |
| GAP-001 | Yes | No | No | Yes | Gameplay Module API Reference.md.md — Include Path Standard |
| GAP-002 | Yes | No | No | Yes | Gameplay Module API Reference.md.md — Unit/HealthComp; Contract Unit.h/HealthComp.h |
| GAP-003 | Yes | No | No | Yes | Gameplay_Public_GameEvents_Documentation.md — EntitySpawnEvent; GameEvents.h |
| GAP-004 | Yes | No | No | Yes | Gameplay Module API Reference.md.md — BaseEntity section; BaseEntity.h |
| GAP-DEV-C-001 | Yes | No | No | Yes | Scene Flow and Entrypoint.md; SceneFlowService.h; IntegrationMock/SceneFlowService.cpp |
| GAP-DEV-C-002 | Yes | No | No | Yes | Placement Validation and TMX Conventions.md; MapLayer.h; TilePlacementController.h; EngineMock MapLayer/TilePlacementController |
| GAP-DEV-C-003 | Yes | No | No | Yes | Gameplay Cost Query API.md; CostQuery.h; GameplayMock/CostQuery.cpp |
| GAP-DEV-C-004 | Yes (via GAP-002) | No | No | Yes | Gameplay Module API Reference.md.md — Unit/HealthComp; Unit.h; HealthComp.h; GameplayMock Unit/HealthComp |
| GAP-DEV-C-005 | Yes (via GAP-003) | No | No | Yes | Gameplay_Public_GameEvents_Documentation.md — EntitySpawnEvent; GameEvents.h |
| GAP-DEV-C-006 | Yes | No | No | Yes | Input Routing Policy.md; InputRouter.h; EngineMock/InputRouter.cpp |
| GAP-DEV-C-007 | Yes | No | No | Yes | Gameplay_Public_GameEvents_Documentation.md — BattleEndEvent; GameEvents.h; GameplayMock/GameEvents.cpp |
| GAP-DEV-C-008 | Yes | No | No | Yes | SaveLoad and Determinism API.md; SaveLoadService.h; IntegrationMock/SaveLoadService.cpp |
| GAP-DEV-C-009 | Yes | No | No | Yes | SaveLoad and Determinism API.md (Semantics); SaveLoadService.h |
| GAP-DEV-C-010 | Yes | No | No | Yes | SaveLoad and Determinism API.md — determinism hooks; SaveLoadService.h; IntegrationMock/SaveLoadService.cpp |

## Verification (commands + results)
- cmake -S . -B build -DUSE_COCOS_ENGINE=OFF -DBUILD_APP=OFF -DBUILD_TESTS=ON : **PASS**
- cmake --build build : **PASS**
- ctest --test-dir build --output-on-failure : **PASS**
