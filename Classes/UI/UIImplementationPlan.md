# UI Implementation Plan (Menu, Build, Attack)

## Sources of Truth
- Gameplay event payloads: `Classes/Contract/Gameplay/GameEvents.h` (resource/troop/building/battle events).
- Scene navigation: `Classes/Integration/Docs/Scene Flow and Entrypoint.md` (`SceneFlowService` stages and launch params).
- Map selection state: `Classes/Managers/LevelManager.h` (map path + seed for start-game wiring).
- UI assets: `Classes/UI/README.md` (catalog under `Resources/UI`).

## High-Level UI Goal
Deliver a product-level, polished UI for **Menu**, **Build**, and **Attack** that is:
- Driven by **real gameplay contracts** (`Classes/Contract/**`) and ready for the real Cocos engine.
- **Testable in mock-only mode** via `UiStateStore` + `UiPresentationBinding` without requiring Cocos runtime/assets.
- **Strictly presentation-only** (no gameplay rules or predictions).

## Plan Alignment Notes (Gaps + Adjustments)
### Gaps / misalignments
- The plan assumes asset-backed widgets exist; current scenes only render placeholder nodes/labels.
- Build/Attack mode switching is described as UI-driven, but there is no gameplay contract event for mode; current implementation infers mode from scene stage/battle state.
- Menu UI button/input wiring is not yet connected to `InputRouter` or concrete UI widgets.

### Concrete adjustments
- Implement Cocos UI nodes from `UiRenderPlan` assets (sprites/labels) while keeping mock tests focused on render plans.
- Drive `UiMode` explicitly from scene stage (Menu/Build/Attack) to avoid gameplay assumptions until a contract event exists.
- Add deterministic tests around menu start/map selection using `SceneFlowService` and `LevelManager` (no Cocos runtime).

## Feature Plan Based on Current APIs & Assets
This plan lists **what we can build now** using existing contracts, scenes, and UI assets—without inventing gameplay rules.

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
- `EconomySystem` / `CostQuery` for cost & affordability queries (read-only, no gameplay rules).
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
- Combat simulation, targeting, or troop logic (gameplay-owned).

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
- Maintain deterministic rendering by polling `UiStateStore::GetSnapshot()` each frame (no direct gameplay queries) and keeping Cocos nodes keyed by `entity_id` for health bars/building overlays.

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
