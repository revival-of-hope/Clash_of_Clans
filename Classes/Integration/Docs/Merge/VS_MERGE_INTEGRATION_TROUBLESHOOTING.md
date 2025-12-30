# Visual Studio Merge Integration Instructions (Windows)

This document captures **two observed merge problems** in Visual Studio (Windows) and provides
actionable integration steps based on the reported build log.

---

## Reported Problems (from build log)

1) **Visual Studio reports missing include paths unless `Classes/` is removed**  
2) **Removing `Classes/` to “fix” includes causes numerous build errors**

These issues are connected: **removing `Classes/` breaks canonical Contract includes** and
causes the build to pick up the wrong headers or fail to resolve the Contract API surface.

---

## Integration Guidance

### 1) Keep canonical Contract includes (do **not** remove `Classes/`)
**Do not strip `Classes/` from include directives.** The repository enforces canonical Contract
includes (`#include "Classes/Contract/..."`) and a lint test exists to block alternate roots.

**Why (evidence):**
- `scripts/check_includes.sh` explicitly fails if include paths are shortened.  
- `CMakeLists.txt` includes `include_path_lint` as a test.

**Action:**
- Ensure the Visual Studio include search path contains the repository root so
  `Classes/...` resolves.
  - In a CMake-generated VS solution, this should already be handled; if it is not,
    reconfigure CMake and reload the solution.
  - If you must adjust manually, add the repo root to `Additional Include Directories`
    rather than modifying source includes.

---

### 2) Avoid compiling mock-only tests in the app build
The build log shows `MockTests.cpp` compiling in the **main app** build and failing on
test-only APIs (`SetBlockedTilesForTesting`, `SnapToValidTile`, etc.).  
These APIs exist specifically for the mock-only test target and are **not meant to be
compiled into the real app build**.

**Why (evidence):**
- `MockTests.cpp` is under `Classes/Mocks/Tests/**` and is compiled into the `mock_tests`
  target in `CMakeLists.txt`.
- The app target uses `file(GLOB_RECURSE GAME_SOURCE "Classes/*.cpp")`, which can
  unintentionally include test sources in the **app** target when `BUILD_APP=ON`.

**Action:**
- For Visual Studio app builds, ensure `BUILD_TESTS=OFF` (or exclude `Classes/Mocks/**`)
  from the main app target.
  - If using CMake presets: create a VS-only preset with `-DBUILD_TESTS=OFF`.
  - If using manual VS project edits, exclude `Classes/Mocks/**` and
    `Classes/Mocks/Tests/**` from the main `Clash_of_Clans` project.

---

### 3) Resolve `AppDelegate.h` include resolution
The build log reports:
```
main.cpp: error C1083: Cannot open include file "AppDelegate.h"
```

**Action:**
- Ensure the include search path includes **`Classes/Main`** or the repo root so that
  `Classes/Main/AppDelegate.h` resolves.
- Do **not** replace includes with shorter, non-canonical paths; fix the include directories instead.

---

### 4) Cocos audio namespace mismatch (`cocos2d::experimental::AudioEngine`)
The build log reports:
```
CocosAudioSink.cpp: error C2039: "experimental" is not a member of cocos2d
```

**Action:**
1) Verify which AudioEngine namespace is provided by your Cocos2d-x version
   (e.g., `cocos2d::AudioEngine` vs `cocos2d::experimental::AudioEngine`).
2) Update `Classes/Integration/Engine/CocosAudioSink.cpp` to match the namespace exposed
   by the actual Cocos headers in your VS toolchain.
3) If you are building in mock-only mode (`USE_COCOS_ENGINE=OFF`), this file is compiled
   but the runtime methods are stubbed; the namespace still must match at compile time.

---

### 5) `cocos2d::Rect` member errors (`x`, `y`, `width`, `height`)
The build log reports missing members for `cocos2d::Rect` in Menu/GameStage/Results scenes.

**Action:**
1) Confirm the exact `Rect` definition in your Cocos2d-x headers.
   - If the struct uses `origin` + `size`, update hit‑test code to match that API.
2) Confirm that `cocos2d.h` (or the correct geometry header) is included before
   using `cocos2d::Rect`.
3) Avoid mixing Cocos geometry types from different header versions in the same TU.

---

### 6) Contract vs engine MapLayer / TilePlacementController API mismatches
The build log reports missing methods on `MapLayer` and `TilePlacementController` when compiling
`InputRouterImpl.cpp` and `MockTests.cpp`.

**Action:**
1) Verify the include paths resolve to **Contract headers**:
   - `Classes/Contract/Engine/MapLayer.h`
   - `Classes/Contract/Engine/TilePlacementController.h`
2) Do **not** remove `Classes/` from includes. If the compiler is resolving to a different
   class (e.g., an engine-side `MapLayer`), you will see missing Contract methods such as
   `WorldToTile` and `TileToWorldCenter`.
3) Ensure mock-only APIs (`SetBlockedTilesForTesting`, `SetTileSizeForTesting`) are not
   compiled into the real app target (see section 2).

---

## Quick Checklist (Windows / Visual Studio)
- ✅ Keep `#include "Classes/Contract/..."` intact.  
- ✅ Make sure repo root is in include dirs (fix VS settings, not source).  
- ✅ Build **mock tests** with `USE_COCOS_ENGINE=OFF`, `BUILD_TESTS=ON`.  
- ✅ Build **app** with `BUILD_TESTS=OFF` to avoid test-only source compilation.  
- ✅ Verify Cocos AudioEngine namespace in your version and update `CocosAudioSink.cpp`.  
- ✅ Align `cocos2d::Rect` usage with the actual definition in your Cocos headers.  

---

## Cocos2d Alignment Report (Repo-Verified)

This section documents the Cocos2d-x definitions **as they exist in this repository** to
help resolve Windows merge errors.

### AudioEngine namespace in this repo
**Observed:** `AudioEngine` is declared inside the `cocos2d` namespace (not `cocos2d::experimental`).

**Evidence:**
- `cocos2d/cocos/audio/include/AudioEngine.h` uses `NS_CC_BEGIN`/`NS_CC_END`, which expands to
  `namespace cocos2d { ... }`.  
  - `AudioEngine` class begins at line 87 within the `NS_CC_BEGIN` block.

**Implication:**
- Code should call `cocos2d::AudioEngine::play2d(...)` in this repo’s Cocos2d-x version.
- Calls to `cocos2d::experimental::AudioEngine` will fail to compile against this header set.

### cocos2d::Rect layout in this repo
**Observed:** `cocos2d::Rect` stores **`origin`** and **`size`**, not `x/y/width/height` fields.

**Evidence:**
- `cocos2d/cocos/math/CCGeometry.h` defines `Rect` with:
  - `Vec2 origin;`
  - `Size size;`

**Implication:**
- Code that accesses `Rect.x`, `Rect.y`, `Rect.width`, `Rect.height` will not compile
  against this repo’s Cocos2d-x headers.
- Use `rect.origin.x`, `rect.origin.y`, `rect.size.width`, `rect.size.height`
  or the provided accessor methods (`getMinX()`, `getMaxX()`, etc.).
