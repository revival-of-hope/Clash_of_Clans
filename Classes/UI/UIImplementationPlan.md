# UI Implementation Plan (Menu, Build, Attack)

## Sources of Truth
- Gameplay event payloads: `Classes/Contract/Gameplay/GameEvents.h` (resource/troop/building/battle events).
- Scene navigation: `Classes/Integration/Docs/Scene Flow and Entrypoint.md` (`SceneFlowService` stages and launch params).
- Map selection state: `Classes/Managers/LevelManager.h` (map path + seed for start-game wiring).
- Player identity: `Classes/Contract/Integration/PlayerIdentityService.h` (player id/name/icon for UI display).
- UI assets: `Classes/UI/README.md` (catalog under `Resources/UI`).

## High-Level UI Goal
Deliver a product-level, polished UI for **Menu**, **Build**, and **Attack** that is:
- Driven by **real Gameplay contracts** (`Classes/Contract/**`) and ready for the real Cocos engine.
- **Testable in mock-only mode** via `UiStateStore` + `UiPresentationBinding` without requiring Cocos runtime/assets.
- **Strictly presentation-only** (no Gameplay rules or predictions).

## Plan Alignment Notes (Gaps + Adjustments)
### Gaps / misalignments
- The plan assumes asset-backed widgets exist; current scenes only render placeholder nodes/labels.
- Build/Attack mode switching is described as UI-driven, but there is no Gameplay contract event for mode; current implementation infers mode from scene stage/battle state.
- Menu UI button/input wiring is not yet connected to `InputRouter` or concrete UI widgets.

### Concrete adjustments
- Implement Cocos UI nodes from `UiRenderPlan` assets (sprites/labels) while keeping mock tests focused on render plans.
- Drive `UiMode` explicitly from scene stage (Menu/Build/Attack) to avoid Gameplay assumptions until a contract event exists.
- Add deterministic tests around menu start/map selection using `SceneFlowService` and `LevelManager` (no Cocos runtime).

## Feature Audit (Build/Attack/Menu/HUD)
### 1) Build mode
- **Read Core: BuildingTypes + configs**
  - **Status:** Implemented (Core data exists; UI wiring not required).
  - **Evidence:** `Classes/Core/GameConstants.h` :: `enum class BuildingType`
    - **Pointer:** `rg -n "enum class BuildingType" Classes/Core/GameConstants.h`
  - **Evidence:** `Classes/Core/GameConfig.cpp` :: `GameConfig::GetBuildingStats`
    - **Pointer:** `rg -n "GetBuildingStats" Classes/Core/GameConfig.cpp`
  - **Dependencies:** `Core::BuildingType`, `Core::GameConfig::GetBuildingStats`.
- **Bottom bar: select building type + place on map (show construction time)**
  - **Status:** Partially implemented (build palette + selection wired; placement missing in scene).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `BuildPaletteSnapshot`, `AppendBuildPalette`
    - **Pointer:** `rg -n "BuildPaletteSnapshot|AppendBuildPalette" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateBuildPalette`, `HandleTap` (build_select)
    - **Pointer:** `rg -n "UpdateBuildPalette|build_select" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Core::BuildingType` (`Classes/Core/GameConstants.h`), `CostQuery::GetBuildingPlacementCost` / `GetBuildingConstructionTime` (`Classes/Contract/Gameplay/CostQuery.h`).
  - **Skipped:** Placement flow (needs `MapLayer` + `TilePlacementController`, not available in `GameStageScene`).
- **Clicking a building shows its config data**
  - **Status:** Partially implemented (state + render; no hit-test wiring).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `SelectedBuildingSnapshot`, `UpdateSelectedBuilding`
    - **Pointer:** `rg -n "SelectedBuildingSnapshot|UpdateSelectedBuilding" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateSelectedBuilding`
    - **Pointer:** `rg -n "UpdateSelectedBuilding" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Core::GameConfig::GetBuildingStats` (`Classes/Core/GameConfig.h`), `Gameplay::EntitySpawnEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
  - **Skipped:** Entity click hit-test (no contract API to map clicks → entity id).
- **Collecting Gold & Elixir (UI + state update path)**
  - **Status:** Partially implemented (HUD updates on events; no collect UI).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `HudState::ApplyResourceUpdate`, `UiStateStore::OnResourceChanged`
    - **Pointer:** `rg -n "ApplyResourceUpdate|OnResourceChanged" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateHud`
    - **Pointer:** `rg -n "UpdateHud" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::ResourceUpdateEvent` (`Classes/Contract/Gameplay/GameEvents.h`), `EconomySystem::TryCollectResource` (`Classes/Contract/Gameplay/EconomySystem.h`).
  - **Skipped:** Collect action requires a `Building*` handle not available in UI scene.
- **Boost construction time / gold production time (optional)**
  - **Status:** Not implemented (no boost APIs in Gameplay contracts).
  - **Evidence:** `Classes/Contract/Gameplay/CostQuery.h` :: available cost/time APIs
    - **Pointer:** `rg -n "GetBuildingPlacementCost|GetBuildingConstructionTime|GetTroopTrainingCost" Classes/Contract/Gameplay/CostQuery.h`
  - **Dependencies:** N/A (no contract support).

### 2) Attack mode
- **Countdown timer HUD**
  - **Status:** Implemented (battle timer state + label).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `BattleState::OnBattleStarted`, `UiPresentationBinding::AppendBattle`
    - **Pointer:** `rg -n "OnBattleStarted|AppendBattle" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateBattlePanel`
    - **Pointer:** `rg -n "UpdateBattlePanel" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::BattleStartEvent` (`Classes/Contract/Gameplay/GameEvents.h`), `UiStateStore`.
- **Deployment bar counts + selection highlight**
  - **Status:** Implemented (render-plan + scene labels).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `AppendDeployment`
    - **Pointer:** `rg -n "AppendDeployment" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateDeploymentBar`
    - **Pointer:** `rg -n "UpdateDeploymentBar" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::TroopCountUpdateEvent`, `Gameplay::DeploymentSelectionEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
- **Results panel (victory/defeat + stars)**
  - **Status:** Implemented (results labels show result/stars/loot).
  - **Evidence:** `Classes/Scenes/ResultsScene.cpp` :: `ResultsScene::init`
    - **Pointer:** `rg -n "Result|Stars|Loot" Classes/Scenes/ResultsScene.cpp`
  - **Dependencies:** `Gameplay::BattleEndEvent` (`Classes/Contract/Gameplay/GameEvents.h`), `Integration::ResultsScreenData` (`Classes/Contract/Integration/SceneFlowService.h`).
- **Matchmaking cost display**
  - **Status:** Implemented (Find Match label + cost; spend gold on tap).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `MatchmakingSnapshot`, `AppendMatchmaking`
    - **Pointer:** `rg -n "MatchmakingSnapshot|AppendMatchmaking" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateMatchmakingPanel`, `HandleTap` (find_match)
    - **Pointer:** `rg -n "UpdateMatchmakingPanel|find_match" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `CostQuery::GetMatchmakingCost` (`Classes/Contract/Gameplay/CostQuery.h`), `EconomySystem::CanAffordCost` (`Classes/Contract/Gameplay/EconomySystem.h`).
  - **Skipped:** Matchmaking service call (no contract API).
- **Troop bar + inspect (config-driven)**
  - **Status:** Partially implemented (display counts + selection + stats; no buy/deploy).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `TroopInspectSnapshot`, `AppendTroopInspect`
    - **Pointer:** `rg -n "TroopInspectSnapshot|AppendTroopInspect" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateDeploymentBar`, `UpdateTroopInspect`
    - **Pointer:** `rg -n "UpdateDeploymentBar|UpdateTroopInspect" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Core::GameConfig::GetTroopStats` (`Classes/Core/GameConfig.h`), `Gameplay::TroopCountUpdateEvent`, `Gameplay::DeploymentSelectionEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
  - **Skipped:** Train/deploy UI wiring pending; command contract now defined (`Classes/Contract/Gameplay/TroopCommandService.h`).
- **Win battle → trophy increment**
  - **Status:** Implemented (results panel displays trophy delta/total).
  - **Evidence:** `Classes/Scenes/ResultsScene.cpp` :: trophies label
    - **Pointer:** `rg -n "Trophies" Classes/Scenes/ResultsScene.cpp`
  - **Dependencies:** `Gameplay::BattleEndEvent::trophies_earned` / `trophies_total` (`Classes/Contract/Gameplay/GameEvents.h`).
- **Random base generation**
  - **Status:** Not implemented (contract now defined; no UI wiring yet).
  - **Evidence:** `Classes/Contract/Integration/SceneFlowService.h` :: `BattleLaunchParams::use_random_base`
    - **Pointer:** `rg -n "use_random_base" Classes/Contract/Integration/SceneFlowService.h`
  - **Dependencies:** `Integration::BattleLaunchParams` (`Classes/Contract/Integration/SceneFlowService.h`).

### 3) Menu / Player identity
- **Create player name and optional icon; generate unique player id**
  - **Status:** Not implemented (identity UI not wired; persistence semantics defined).
  - **Evidence:** `Classes/Integration/Docs/DevC_Spec.md` :: `PlayerIdentityService`
    - **Pointer:** `rg -n "PlayerIdentityService" Classes/Integration/Docs/DevC_Spec.md`
  - **Evidence:** `Classes/Contract/Integration/PlayerIdentityService.h` :: `CreateIdentity`
    - **Pointer:** `rg -n "CreateIdentity" Classes/Contract/Integration/PlayerIdentityService.h`
  - **Dependencies:** `Integration::PlayerIdentity`, `Integration::PlayerIdentityService`.
- **Player id ownership association for troops/buildings**
  - **Status:** Partially implemented (local owner id used for ally flag + deployment selection).
  - **Evidence:** `Classes/Core/GameStructs.h` :: `Core::BuildingData::owner_id_`, `Core::UnitData::owner_id_`
    - **Pointer:** `rg -n "owner_id_" Classes/Core/GameStructs.h`
  - **Evidence:** `Classes/Integration/Docs/DevC_Spec.md` :: ownership mapping rule
    - **Pointer:** `rg -n "Ownership mapping rule" Classes/Integration/Docs/DevC_Spec.md`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `ResolveLocalOwnerId`
    - **Pointer:** `rg -n "ResolveLocalOwnerId" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Core::BuildingData`, `Core::UnitData`, `Integration::PlayerIdentity::player_id`.
- **Menu navigation + map selection**
  - **Status:** Implemented (placeholder labels + selection + Start wiring).
  - **Evidence:** `Classes/Scenes/MenuScene.cpp` :: `HandleTap`, `StartSelectedMap`
    - **Pointer:** `rg -n "HandleTap|StartSelectedMap" Classes/Scenes/MenuScene.cpp`
  - **Dependencies:** `Integration::SceneFlowService` (`Classes/Contract/Integration/SceneFlowService.h`), `LevelManager` (`Classes/Managers/LevelManager.h`).
- **Leagues/Replays buttons (stub actions)**
  - **Status:** Partially implemented (labels + tap recording only).
  - **Evidence:** `Classes/Scenes/MenuScene.cpp` :: `HandleTap` + label creation
    - **Pointer:** `rg -n "Leagues|Replays|HandleTap" Classes/Scenes/MenuScene.cpp`
  - **Dependencies:** N/A (no contract services defined).
  - **Skipped:** League/replay service APIs not defined in contract.
- **Menu audio configuration**
  - **Status:** Implemented (menu music + UI click).
  - **Evidence:** `Classes/Scenes/MenuScene.cpp` :: `PlayMenuMusic`, `PlayUiClick`
    - **Pointer:** `rg -n "PlayMenuMusic|PlayUiClick" Classes/Scenes/MenuScene.cpp`
  - **Evidence:** `Classes/Managers/AudioManager/AudioManager.cpp` :: menu mappings
    - **Pointer:** `rg -n "menu_music|ui_click" Classes/Managers/AudioManager/AudioManager.cpp`
  - **Dependencies:** `AudioManager` (`Classes/Managers/AudioManager/AudioManager.h`), `IAudioSink` (`Classes/Contract/Engine/AudioSink.h`).

### 4) HUD (always visible)
- **Resource HUD (gold/elixir/gems)**
  - **Status:** Implemented (state + render + scene labels).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `HudState::ApplyResourceUpdate`, `UiPresentationBinding::AppendHud`
    - **Pointer:** `rg -n "ApplyResourceUpdate|AppendHud" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateHud`
    - **Pointer:** `rg -n "UpdateHud" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::ResourceUpdateEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
- **Health bars for entities**
  - **Status:** Partially implemented (state tracked; no render-plan output yet).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `HealthBarState::OnEntitySpawned`
    - **Pointer:** `rg -n "HealthBarState::OnEntitySpawned" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `UiPresentationBinding::BuildRenderPlan` (no health bar rendering)
    - **Pointer:** `rg -n "BuildRenderPlan" Classes/UI/UiStateModels.h`
  - **Dependencies:** `Gameplay::EntitySpawnEvent`, `Gameplay::DamageEvent`, `Gameplay::EntityDestroyEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
- **Building progress overlays**
  - **Status:** Partially implemented (render-plan entries and placeholder nodes only).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `BuildingState::OnBuildingStateChanged`, `AppendBuildingOverlays`
    - **Pointer:** `rg -n "OnBuildingStateChanged|AppendBuildingOverlays" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `UpdateBuildingOverlays`
    - **Pointer:** `rg -n "UpdateBuildingOverlays" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::EntitySpawnEvent`, `Gameplay::BuildingStateEvent`, `Gameplay::EntityDestroyEvent` (`Classes/Contract/Gameplay/GameEvents.h`).

## Implementation Plan for Missing Features
**Status:** Core contract blockers resolved; remaining items are UI wiring or integration.

### Milestone A — Build-mode palette + placement (presentation-only)
1. **Data sources (SOT):**
   - Building types: `Core::BuildingType` (`Classes/Core/GameConstants.h`).
   - Costs + construction time: `CostQuery::GetBuildingPlacementCost`, `GetBuildingConstructionTime` (`Classes/Contract/Gameplay/CostQuery.h`).
   - Placement validation: `InputRouter` + `TilePlacementController` (`Classes/Contract/Engine/InputRouter.h`, `TilePlacementController.h`).
2. **UI state additions (UiStateStore):**
   - Add `BuildPaletteSnapshot` with `selected_type`, `selected_level`, `ResourceCost cost`, `float build_time_seconds`.
   - Add setters on `UiStateStore` to update selection from UI interactions (no Gameplay logic).
3. **Render-plan keys + assets/text fallback:**
   - Keys: `build_palette_<BuildingType>`, `build_place_button`, `build_info_panel`.
   - Assets: prefer `Resources/UI/Buttons/UpgradeButton.png` and `Resources/UI/Columns/resources num display column.png` (from `Classes/UI/README.md`); fallback to labels if assets missing.
4. **Scene wiring (GameStageScene):**
   - In build mode, render palette buttons; route taps via `InputRouter` to set `UiStateStore` selection.
   - On confirm placement, call `TilePlacementController::startPlacement(...)` using a UI placeholder sprite from `Resources/UI/Columns/Troop Placeholder.png` (per `Classes/UI/README.md`) and show construction time from `CostQuery`.
5. **Mock-driven tests (ctest OFF mode):**
   - Add mock test verifying `UiPresentationBinding` produces palette render items for a snapshot with `BuildPaletteSnapshot`.
   - Add mock test asserting selection updates the render-plan keys deterministically (no Cocos runtime).

### Milestone B — Building inspection panel
1. **Data sources (SOT):**
   - Building stats: `Core::GameConfig::GetBuildingStats` (`Classes/Core/GameConfig.h/.cpp`).
   - Entity identity: `Gameplay::EntitySpawnEvent` (`Classes/Contract/Gameplay/GameEvents.h`) for instance IDs.
2. **UI state additions:**
   - Extend snapshot with `SelectedBuildingSnapshot` (entity_id, type, level).
3. **Render-plan keys + assets/text fallback:**
   - Keys: `building_inspect_panel`, `building_stat_<field>`.
   - Text fallback: show HP, size (w/h), production rate, capacity via labels.
4. **Scene wiring (GameStageScene):**
   - On entity click, call `UiStateStore::SetSelectedEntity(...)` and populate `SelectedBuildingSnapshot`.
5. **Mock-driven tests:**
   - Add mock test that `SelectedBuildingSnapshot` maps to a render-plan panel with expected stat labels.

### Milestone C — Resource collection affordance (UI-only)
1. **Data sources (SOT):**
   - Collection: `EconomySystem::TryCollectResource(Building*)` (`Classes/Contract/Gameplay/EconomySystem.h`).
   - HUD updates: `Gameplay::ResourceUpdateEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
2. **UI state additions:**
   - Add `collect_available` flag on `SelectedBuildingSnapshot` based on stored resource > 0 (from `Building::GetStoredResource()`).
3. **Render-plan keys + assets/text fallback:**
   - Key: `building_collect_button` with `Resources/UI/Buttons/StoreButton.png` fallback label “Collect”.
4. **Scene wiring (GameStageScene):**
   - On collect button tap, call `EconomySystem::TryCollectResource(selected_building)` and rely on `OnResourceChanged` for HUD update.
5. **Mock-driven tests:**
   - Add mock test verifying collect button appears when `collect_available` is true.

## Feature Plan Based on Current APIs & Assets
This plan lists **what we can build now** using existing contracts, scenes, and UI assets—without inventing Gameplay rules.

### Menu (Navigation + Map Select)
**Available APIs/Assets**
- `SceneFlowService::ShowMenuScene()` and `StartGame(params)` for navigation.
- `LevelManager::SelectMapA/SelectMapB`, `GetSelectedMapPath`, `GetSeed`.
- UI assets: `Resources/UI/Backgrounds/settings.png`, `Resources/UI/Buttons/BattleSearchButton.png`, `Resources/UI/Buttons/return_botton.png`.

**Features to implement**
- Background panel + start button using the above assets.
- Map A/B selection buttons (visual toggle + label of active map path).
- Start action calls `StartGame` with `LevelManager` selection and seed.
- Optional: show seed value in a small label for mock/test determinism.

**Not in scope yet**
- Real UI input binding for touch/click (placeholder nodes exist; wire later via InputRouter).

### Build Mode (Base Editing HUD)
**Available APIs/Assets**
- `Gameplay::GameEvents` for resources, building spawn/state, entity destroy, damage.
- `EconomySystem` / `CostQuery` for cost & affordability queries (read-only, no Gameplay rules).
- UI assets: `Resources/UI/Icons/coin.png`, `Resources/UI/Icons/elixir.png`, `Resources/UI/Columns/resources num display column.png`.

**Features to implement**
- HUD resource bar (gold/elixir values + capacity) driven by `ResourceUpdateEvent`.
- Building progress list (construction/upgrade) using `BuildingStateEvent` + `EntitySpawnEvent`.
- Selection highlight & health bars based on `EntitySpawnEvent`, `DamageEvent`, `EntityDestroyEvent`.
- Cost display for selected building (via `CostQuery` read-only query) **only for display**.

**Not in scope yet**
- Placement validation logic or tile rules (defer to `TilePlacementController` + InputRouter).

### Attack Mode (Battle HUD + Results)
**Available APIs/Assets**
- `Gameplay::BattleStartEvent` / `BattleEndEvent` / `TroopCountUpdateEvent` / `DeploymentSelectionEvent`.
- UI assets: `Resources/UI/Icons/timeleft.png`, `Resources/UI/Backgrounds/Real victory.png`, `Resources/UI/Backgrounds/shop.png`, `Resources/UI/Backgrounds/0-3 stars.png`, `Resources/UI/Columns/Troop Placeholder.png`.

**Features to implement**
- Countdown timer HUD using `BattleStartEvent` time limit.
- Deployment bar counts + selection highlight from troop events.
- Results panel (victory/defeat + stars + loot summary) driven by `BattleEndEvent`.

**Not in scope yet**
- Combat simulation, targeting, or troop logic (Gameplay-owned).

## RenderPlan → Cocos Node Rules (Product-Grade)
- **Stable keys:** Use deterministic keys for UI nodes (e.g., `hud_gold`, `hud_elixir`, `build_progress_<entity_id>`, `results_panel`, `menu_start`).
- **Update vs rebuild:** Reuse existing nodes when a key is present; only create new nodes for new keys and remove nodes for keys that disappeared.
- **Lifecycle cleanup:** When an entry is absent from the render plan, remove the corresponding node from parent and erase the key from the registry.
- **Caching:** Reuse sprite frames and fonts where possible (e.g., shared label font + size per UI layer).
- **Layout policy:** Anchor points centered; positions derived from a shared layout constants object so layouts are deterministic across builds.
- **Safe area (future):** If a safe-area API becomes available, offset HUD anchors by safe-area insets; otherwise use fixed offsets.

## Acceptance Criteria (Polish Baseline)
### Menu
- Background visible, Start button visible and enabled.
- Map A/B selection visibly reflects active selection.
- Leagues and Replays buttons visible (stub actions).
- Start transitions to Game with selected map and seed.

### Build
- Resource HUD visible and updates on resource events.
- Build panel visible and togglable via input.
- At least one building progress entry displays when events arrive.
- Selection highlight and health bar entries appear/disappear on spawn/destroy events.
- Save Base / Load Base actions are present (stub).

### Attack + Results
- Countdown HUD displays on BattleStart.
- Deployment bar shows counts + selection highlight.
- Results panel shows victory/defeat background, stars, and loot values on BattleEnd.
- Replay button visible on Results (stub action).

## Missing APIs / Assets / Contracts
- **Leagues UI:** No contract/API to fetch leagues data or navigate to a leagues screen.
- **Replay playback:** No contract/API to request a replay or playback data.
- **Save/Load base:** No contract/API for saving/loading base layouts (no Save/Load service in Contract/Integration).
- **Safe-area layout:** No contract or engine API surfaced for safe-area insets.
- **UI input plumbing:** No contract-level touch/click dispatch API for UI widgets; current scenes rely on deterministic `HandleTap(...)` hooks for tests.

## Current State Model (after this update)
- `UiStateStore` now captures HUD (resources), selection, health bars, deployment bar, building progress, battle lifecycle, and an explicit `UiMode` (Menu/Build/Attack) snapshot for renderers.
- Building progress listens to `EntitySpawnEvent`/`BuildingStateEvent`; battle HUD listens to `BattleStartEvent`/`BattleEndEvent` for countdown + settlement payloads.

## Plan: Build a visible production UI

### 1) Menu Layer (SceneFlow + Level selection)
- Use `SceneFlowService::ShowMenuScene()` to render the menu shell, sourcing selected map/seed from `LevelManager` (default Map A, optional seed) and routing Start to `SceneFlowService::StartGame(params)`.
- Bind map selection buttons to `LevelManager::SelectMapA/SelectMapB` and display the active map name/path; reuse `Resources/UI/Backgrounds` + `Buttons` for layout.
- Surface a “Battle” call-to-action that switches `UiMode` to `kAttack` once the game scene is entered.

### 2) Build Mode UI (Base editing)
- When the game scene enters builder mode, set `UiMode::kBuild` and drive widgets from `UiStateStore`:
  - Resource bar uses `HudSnapshot` (Gold/Elixir values/capacity).
  - Building list uses static icons from `Resources/UI/Columns/` plus building thumbnails; placement preview listens to InputRouter and shows valid/invalid tiles via `TilePlacementController` (contract-driven, no rule inference).
  - Construction/upgrade panels read `building_progress` entries: show state (constructing/idle/destroyed) and progress bars using `time_remaining/total_build_time`.
- Hook `GameEventManager` events: `OnBuildingStateChanged` updates progress UI; `OnResourceChanged` refreshes cost availability; `OnEntityDestroyed` clears selection/progress displays.

### 3) Attack Mode UI (Battle HUD)
- On `BattleStartEvent`, set `UiMode::kAttack`, display countdown from `battle.time_limit_seconds`, and start BGM through existing `AudioManager` wiring; show deployment bar using `deployment` snapshot (counts + selection highlight).
- Health bars overlay entities using `health_bars` entries (ally flag controls color); selection highlight uses `selection` snapshot.
- On `BattleEndEvent`, stop countdown, freeze deployment, and present results panel using cached `battle.last_battle_end` (stars, destruction%, loot). Use `Resources/UI/Backgrounds` victory/defeat assets.

### 4) Shared HUD components
- Deploy a top resource row (gold/elixir) for both build and attack modes using `HudSnapshot` and icons from `Resources/UI/Icons/`.
- Maintain deterministic rendering by polling `UiStateStore::GetSnapshot()` each frame (no direct Gameplay queries) and keeping Cocos nodes keyed by `entity_id` for health bars/building overlays.

### 5) Event Wiring & Lifecycle
- Attach `UiStateStore` to `GameEventManager` during scene init; detach on exit to avoid dangling listeners.
- Scene transitions follow `SceneFlowService` stages: Boot → Menu (UiMode kMenu) → Game (set to kBuild or kAttack based on entry) → Results overlay; keep UI controllers stateless across scene recreations, rehydrate from `LevelManager` + `GameEventManager` cache (`GetLastBattleEnded`).

### 6) Stage 4.5 Binding (Presentation)
- Use `UiPresentationBinding` (deterministic presenter-props) to convert `UiStateSnapshot` into render plans per frame; this keeps runtime compatible with Cocos nodes and mock harnesses.
- Render plan surfaces assets + layout constants only (no engine types):
  - HUD icons: `Resources/UI/Icons/coin.png`, `Resources/UI/Icons/elixir.png` with formatted `current/capacity` labels.
  - Menu shell: background `Resources/UI/Backgrounds/settings.png`, start button `Resources/UI/Buttons/BattleSearchButton.png`.
  - Build overlays: `Resources/UI/Columns/resources num display column.png` progress bars keyed by `entity_id` with clamped ratios.
  - Attack/results: countdown icon `Resources/UI/Icons/timeleft.png`; victory background `Resources/UI/Backgrounds/Real victory.png`; defeat fallback `Resources/UI/Backgrounds/shop.png`; stars (`0-3 stars.png`).
- Layout constants (durable defaults, tuned later without logic changes): hud padding 8px, hud icon 32px, hud font 18pt, countdown font 24pt, progress height 14px, results padding 12px.

### 7) Verification Strategy
- Extend mock-driven tests to assert: (a) builder progress reacts to BuildingState events, (b) battle HUD records start/end data and mode flag, (c) menu selection still routes through `LevelManager` into `StartGame` (existing Stage 6 coverage).
- Use mock `GameEventManager` broadcasts to prove UI state changes without Cocos runtime; run via `ctest` in mock mode.
- **Player name/icon HUD**
  - **Status:** Partially implemented (name label only; icon not mapped).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `PlayerIdentitySnapshot`, `AppendIdentity`
    - **Pointer:** `rg -n "PlayerIdentitySnapshot|AppendIdentity" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `player_label_` in `BuildStaticUi`
    - **Pointer:** `rg -n "player_label_" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Integration::PlayerIdentityService` (`Classes/Contract/Integration/PlayerIdentityService.h`).
  - **Skipped:** Icon asset mapping undefined (icon_id semantics not specified).
- **Gems + trophies HUD**
  - **Status:** Implemented (state + labels; event-driven).
  - **Evidence:** `Classes/UI/UiStateModels.h` :: `HudState::ApplyResourceUpdate`
    - **Pointer:** `rg -n "Gems|Trophies" Classes/UI/UiStateModels.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `gem_label_`
    - **Pointer:** `rg -n "gem_label_" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::ResourceUpdateEvent` (`Classes/Contract/Gameplay/GameEvents.h`).
- **Available loot (gold/elixir/trophies)**
  - **Status:** Implemented (loot availability event + label).
  - **Evidence:** `Classes/Contract/Gameplay/GameEvents.h` :: `LootAvailabilityEvent`
    - **Pointer:** `rg -n "LootAvailabilityEvent" Classes/Contract/Gameplay/GameEvents.h`
  - **Evidence:** `Classes/Scenes/GameStageScene.cpp` :: `loot_label_`
    - **Pointer:** `rg -n "loot_label_" Classes/Scenes/GameStageScene.cpp`
  - **Dependencies:** `Gameplay::LootAvailabilityEvent` (`Classes/Contract/Gameplay/GameEvents.h`).

## Gap Resolution Plan for Skipped Features (UI-only roadmap)
This plan records the minimum contract/integration actions needed to unblock skipped features without touching Engine/Gameplay implementation code. All changes are confined to `Classes/Contract/**` and `Classes/Integration/Docs/**`, with mock updates under `Classes/Mocks/**` for tests.

### A) HUD currencies + trophies (G8/G9)
1. Update SOT docs to define gems/trophies fields and change events:
   - Add gems/trophies to the Gameplay UI event spec in `Classes/Integration/Docs/Gameplay_Public_GameEvents_Documentation.md`.
2. Extend contract headers to match SOT:
   - Add gems/trophies fields (current + delta) to `Gameplay::ResourceUpdateEvent` or define a new event in `Classes/Contract/Gameplay/GameEvents.h`.
   - Add trophies (delta + total) to `Gameplay::BattleEndEvent` or a separate post-battle event in `GameEvents.h`.
3. Update mocks + tests:
   - Add mock broadcast behavior in `Classes/Mocks/GameplayMock/GameEvents.cpp`.
   - Add mock tests under `Classes/Mocks/Tests/MockTests.cpp` to validate HUD snapshot updates and render-plan output.

### B) Troop training/deployment commands (G10)
1. Define a UI → Gameplay command contract (docs + header):
   - Add a new contract interface (e.g., `Classes/Contract/Gameplay/TroopCommandService.h`) and SOT docs under `Classes/Integration/Docs/**`.
2. Add mock implementation for tests:
   - Implement a mock command service under `Classes/Mocks/GameplayMock/**` and wire it in `Classes/Integration/GameServices.cpp`.
3. Add UI wiring plan (no Gameplay rules):
   - Update `UiStateStore`/`UiPresentationBinding` to reflect command availability and button state; add mock tests for click → command dispatch.

### C) Random base generation (G11)
1. Extend SOT for `Integration::BattleLaunchParams`:
   - Document optional random-generation fields in `Classes/Integration/Docs/Scene Flow and Entrypoint.md`.
2. Update contract type:
   - Add fields to `Classes/Contract/Integration/SceneFlowService.h` and keep default behavior backward compatible.
3. Add mock coverage:
   - Update mock scene flow to store new fields and add tests in `Classes/Mocks/Tests/MockTests.cpp`.

### D) Menu audio configuration (G12)
1. Define SOT behavior:
   - Add menu audio policy and clip identifiers in `Classes/Integration/Docs/**` (or `Classes/Managers/AudioManager/README.md`).
2. Update contract usage points:
   - Wire `AudioManager` calls from `MenuScene` (UI-only) using `IAudioSink` contract in `Classes/Contract/Engine/AudioSink.h`.
3. Add mock tests:
   - Add tests to validate the correct audio clips are played when entering menu or pressing menu actions.

### E) Player identity persistence/uniqueness (G13)
1. Define persistence semantics in SOT:
   - Update `Classes/Integration/Docs/DevC_Spec.md` with explicit creation + persistence rules (session-only vs. stored).
2. Reflect in contract + mocks:
   - Update `PlayerIdentityService` contract docs and mock behavior (`Classes/Mocks/IntegrationMock/PlayerIdentityService.cpp`).
3. Add UI wiring + tests:
   - Add tests that identity survives scene transitions when persistence is required.
