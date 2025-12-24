# Scene Flow and Entrypoint (Dev C SOT)

This document defines the canonical Boot → Menu → Game → Results pipeline for Dev C.
Engine/Gameplay remain black boxes; Dev C owns the scene wiring and uses mocks by default.

## Entry contract
* **Factory:** `Integration::SceneFlowService` (Contracts path: `Classes/Contracts/Integration/SceneFlowService.h`).
* **Entry point:** `AppDelegate` (or test harness) calls `CreateSceneFlowService()` then runs `CreateBootScene()`.
* **States:** `kBoot` → `kMenu` → `kGame` → `kResults`; transitions are explicit API calls.
* **Invariant:** `GetCurrentStage()` always reflects the most recent successful transition; creation of scenes is idempotent.

### API surface
```
cocos2d::Scene* SceneFlowService::CreateBootScene();
cocos2d::Scene* SceneFlowService::ShowMenuScene();
cocos2d::Scene* SceneFlowService::StartGame(const BattleLaunchParams& params);
cocos2d::Scene* SceneFlowService::ShowResults(const ResultsScreenData& results);
SceneStage SceneFlowService::GetCurrentStage() const;
```

### Semantics
* **CreateBootScene()** – returns a lightweight splash/loading scene. Does not start gameplay logic.
* **ShowMenuScene()** – returns the menu scene and sets the stage to `kMenu`.
* **StartGame(params)** – transitions to gameplay; `params.map_path` is opaque; `params.seed` seeds deterministic mocks.
* **ShowResults(results)** – transitions to the settlement scene using the provided `ResultsScreenData` (see Results payload spec).
* **Idempotency:** Calling the same transition twice reuses the latest stage value but returns a fresh `Scene` instance for safety in UI tests.

### Usage example (AppDelegate pseudocode)
```
auto flow = Integration::CreateSceneFlowService();
auto boot = flow->CreateBootScene();
Director::getInstance()->runWithScene(boot);

// Later in boot callback
auto menu = flow->ShowMenuScene();
Director::getInstance()->replaceScene(menu);

// Starting a battle
Integration::BattleLaunchParams launch{"maps/mock.tmx", /*seed=*/42};
auto game_scene = flow->StartGame(launch);
Director::getInstance()->replaceScene(game_scene);

// Showing results
Integration::ResultsScreenData results{summary_event, /*elapsed_seconds=*/180, /*troops_deployed=*/5, /*troops_remaining=*/2};
auto results_scene = flow->ShowResults(results);
Director::getInstance()->replaceScene(results_scene);
```

### Error handling
* If the service receives empty map paths or negative seeds, it still transitions but reports them in mock-accessible fields for validation.
* Null returns are not allowed; mocks always allocate a `cocos2d::Scene` instance.

### Mock expectations
* Mocks record the last `BattleLaunchParams` and `ResultsScreenData` to support tests.
* Default mock scenes are empty `cocos2d::Scene` instances with no children; UI code may attach layers after creation.
