# SceneFlowService (Dev C Integration Notes)

## Purpose
`SceneFlowService` owns the Boot → Menu → Game → Results orchestration for Dev C, exposing scene creation and stage tracking without implementing gameplay or engine internals.

## Contract reference
- API: `Classes/Contract/Integration/SceneFlowService.h`
- Scene factories consumed:
  - `BootScene::Create`
  - `MenuScene::Create`
  - `GameStageScene::Create`
  - `ResultsScene::Create`

## Durable truth vs harness
- Durable implementation: `Classes/Integration/SceneFlowServiceImpl.*` (owns stage transitions and caches launch/results payloads).
- Harness-only mock: `Classes/Mocks/IntegrationMock/SceneFlowService.*` delegates to the durable implementation and records calls; it does **not** define the stage machine.
- Composition root switch: `kUseMocks` in `Classes/Integration/GameServices.cpp` selects mock vs production in one place.

## Entrypoint wiring
- `AppDelegate` should resolve `SceneFlowService` and call `CreateBootScene()`; the `app_shell` target compiles this wiring when `BUILD_APP=OFF`.

## Ownership / gaps
- Returned scenes are freshly created from the factories above; ownership/lifetime (including any Director transition policy) is **unspecified** in the current SOT. Gap is tracked in `Classes/Contract/CONTRACT_GAPS.md` (see G1).
