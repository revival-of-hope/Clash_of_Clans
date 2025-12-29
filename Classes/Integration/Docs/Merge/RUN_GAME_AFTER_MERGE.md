# Run Game After Merge (UI + Integration)

These steps assume the real engine/gameplay project is available and that this UI/Integration module is merged in.

---

## 1) CMake options (mock vs real)

### Mock-only UI + integration tests (no real engine)
Use the canonical mock-only configuration:
```
cmake -S . -B build -DUSE_COCOS_ENGINE=OFF -DBUILD_APP=OFF -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### Real game build (Cocos + gameplay + engine)
Use the Windows CMake workflow from `README.md` and explicitly enable the real engine:
```
cmake -S . -B build -G "Visual Studio 17 2022" -A win32 -T v143 -DUSE_COCOS_ENGINE=ON -DBUILD_APP=ON -DBUILD_TESTS=OFF
cmake --build build --config Debug
```

**Output executable:** `Clash_of_Clans` (from `CMakeLists.txt` `APP_NAME`), typically under `build/bin/Debug/` on Windows.

---

## 2) CMakeLists.txt changes (what/where)

This repo already defines the mock/real toggle at compile time:
- `CMakeLists.txt` adds `target_compile_definitions(${APP_NAME} PRIVATE USE_COCOS_ENGINE=1)` and sets `USE_COCOS_ENGINE=0` for `mock_tests` and `app_shell`.

**If you are merging into another project:**
- Ensure the **same compile definitions** exist for the corresponding targets so `Integration::GameServices` can select mock vs real at compile time.
  - Location: `CMakeLists.txt` under the `if(BUILD_APP)` block and the `if(BUILD_TESTS)` block.

---

## 3) App entrypoint (what must be wired)

App startup must create and run the boot scene via the Integration entrypoint:
- File: `Classes/Main/AppDelegate.cpp`
  - Uses `Integration::ResolveSceneFlowService()`
  - Calls `CreateBootScene()` and `Director::runWithScene(boot_scene)`

If your host project uses a different app entrypoint, mirror this wiring so the scene flow contract is exercised.

---

## 4) Assets & Resources requirements

**Resources root must be available at runtime** (working directory should contain `Resources/`).
- Per `UiAssetCatalog`, UI expects assets like:
  - `Resources/UI/Icons/coin.png`
  - `Resources/UI/Buttons/BattleSearchButton.png`
- Per `LevelManager`, map paths are:
  - `maps/map_a.tmx`
  - `maps/map_b.tmx`

Ensure these assets exist in the merged project’s `Resources/` folder and are copied into the runtime working directory.

---

## 5) Visual tests / smoke run checklist

1. **Boot Scene → Menu Scene**
   - The app should start on the boot scene and transition to a simple menu (per the scene flow contract).
2. **Menu Scene UI**
   - Labels for **Start**, **Map A**, **Map B**, **Leagues**, **Replays** appear (placeholders are acceptable).
3. **Map selection**
   - Selecting Map A/B should update the selected map path.
4. **Start Game**
   - Pressing **Start** should transition to the game scene using `BattleLaunchParams` (map path + seed).
5. **Results Scene**
   - Triggering results should show the results scene, fed by the `BattleEndEvent` payload.

If any step fails, check that the scene flow entrypoint and resource paths are correct, and confirm you built with `-DUSE_COCOS_ENGINE=ON` for real builds.

