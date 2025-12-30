# Scenes Overview

This repo currently delivers the Stage 6 requirement for two-map support through the scene layer. Key facts:

- **MenuScene** exposes `SelectMapA`, `SelectMapB`, and `StartSelectedMap()` to drive map choice and launch. On init it defaults selection to Map A and verifies it is entered from the Menu stage before starting a game.
- **LevelManager** is the single source of truth for the selected map path and optional seed. `SelectMapA/SelectMapB` store opaque paths (`maps/map_a.tmx`, `maps/map_b.tmx`), and `GetSelectedMapPath()/GetSeed()` are used when building `BattleLaunchParams`.
- **Battle launch wiring**: `MenuScene::StartSelectedMap()` constructs `Integration::BattleLaunchParams` from `LevelManager` and calls `SceneFlowService::StartGame`, so the chosen map path/seed propagate into the start-game pipeline.
- **Tests**: `TestMenuSceneMapSelectionPropagation` (in `Classes/Mocks/Tests/MockTests.cpp`) selects map A then map B with different seeds and asserts `SceneFlowService::GetLastLaunchParams()` reflects each choice, ensuring distinct selections are observed.

These behaviors mean Stage 6 is fully implemented in durable, production-path code without relying on asset scanning or engine/Gameplay internals.