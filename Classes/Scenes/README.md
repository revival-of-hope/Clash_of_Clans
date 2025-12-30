# Scenes Overview (Stage 6 Map Selection)

This repo currently delivers the Stage 6 requirement for two-map support through the scene layer. Key facts:

- **MenuScene** exposes `SelectMapA`, `SelectMapB`, and `StartSelectedMap()` to drive map choice and launch. On init it defaults selection to Map A and verifies it is entered from the Menu stage before starting a game.
- **LevelManager** is the single source of truth for the selected map path and optional seed. `SelectMapA/SelectMapB` store opaque paths (`maps/map_a.tmx`, `maps/map_b.tmx`), and `GetSelectedMapPath()/GetSeed()` are used when building `BattleLaunchParams`.
- **Battle launch wiring**: `MenuScene::StartSelectedMap()` constructs `Integration::BattleLaunchParams` from `LevelManager` and calls `SceneFlowService::StartGame`, so the chosen map path/seed propagate into the start-game pipeline.
- **Tests**: `TestMenuSceneMapSelectionPropagation` (in `Classes/Mocks/Tests/MockTests.cpp`) selects map A then map B with different seeds and asserts `SceneFlowService::GetLastLaunchParams()` reflects each choice, ensuring distinct selections are observed.

These behaviors mean Stage 6 is fully implemented in durable, production-path code without relying on asset scanning or engine/Gameplay internals.

## Audit Appendix (Stage 6 Map Selection)
- Files changed + rationale (commit `d387e36823574f6522aa9419182a27af5a4d514a`, `git show --stat`):
  - `Classes/Managers/LevelManager.h`: singleton map/seed storage with ResetSelection for isolation.
  - `Classes/Scenes/MenuScene.{h,cpp}`: exposes SelectMapA/SelectMapB/StartSelectedMap wiring `BattleLaunchParams` into `StartGame`.
  - `Classes/Mocks/Tests/MockTests.cpp`: adds `TestMenuSceneMapSelectionPropagation` asserting distinct map_path/seed propagation and cleanup.
  - `Classes/Scenes/README.md`: documents Stage 6 behavior and audit evidence.
  - Diff stat: 5 files, 130 insertions, 2 deletions (Stage 6 implementation).
- Call-chain proof (selection → StartGame):
  - `MenuScene::SelectMapA/SelectMapB` routes choices into `LevelManager` defaults/selection. (`MenuScene.cpp` lines 18, 27, 30, 34)
  - `MenuScene::StartSelectedMap` builds `Integration::BattleLaunchParams` from `LevelManager` and calls `scene_flow_->StartGame`. (`MenuScene.cpp` lines 37-44)
  - `SceneFlowServiceImpl::GetLastLaunchParams` exposes the last launch params recorded by StartGame. (`SceneFlowServiceImpl.cpp` lines 29-36)
- Test proof:
  - `TestMenuSceneMapSelectionPropagation` asserts `launch_a.map_path == GetMapAPath`, `launch_b.map_path == GetMapBPath`, and seeds 101/202 differ, with ResetSelection before/after. (`MockTests.cpp` lines 276-310)
  - `ctest --test-dir build --output-on-failure`: `mock_tests` and `include_path_lint` both PASS (see output with test names + status).
- Sensitivity proof (intentional break → fail → revert → pass):
  - Temporarily forced `LevelManager::SelectMapB` to reuse Map A; `ctest --test-dir build --output-on-failure` then failed at `launch_b.map_path == GetMapBPath()` assertion in `TestMenuSceneMapSelectionPropagation`.
  - Restored `SelectMapB` to use Map B; reran the same ctest command and all tests passed.
- Singleton hygiene:
  - `LevelManager::ResetSelection` resets map_path/seed to defaults to avoid leak across scenes/tests. (`LevelManager.h` lines 28-37)
  - `TestMenuSceneMapSelectionPropagation` calls ResetSelection before and after exercising selections to isolate singleton state. (`MockTests.cpp` lines 280, 309)
