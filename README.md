# **CLASH OF CLANS (DEV-FACING-README)**

### **Overview**

This project is a real-time strategy (RTS) and simulation game. The game blends resource gathering, base building, troop training, and combat simulation, referencing the Gameplay mechanics of "Clash of Clans".

Team Size: 3 Developers  
Timeline: 1 Month

### **Table of Contents**
Please take the English version as the standard/authoritative one.
1. [Core Features](https://www.google.com/search?q=%23core-features)  
2. [Game Content & Assets](https://www.google.com/search?q=%23game-content--assets)  
3. [Architecture Philosophy](https://www.google.com/search?q=%23architecture-philosophy)  
4. [Project Structure](https://www.google.com/search?q=%23project-structure)  
5. [Responsibility & Dependency](https://www.google.com/search?q=%23responsibility-and-dependency)  
6. [Coding Conventions](https://www.google.com/search?q=%23coding-conventions)

### ---

### **Core Features**

The game focuses on a "Main Village" hub system involving economy management and automated combat.

* **Economy & Resources:** Management of Gold, Elixir, and Population/Capacity.  
* **Building System:** Includes Resource Generation (Mines), Army Camps, Defense Towers, and Storage. All buildings support at least 3 upgrade levels.  
* **Combat System:**  
  * **Troops:** Archers, Barbarians, Wall Breakers, and Giants.  
  * **AI:** Pathfinding (A\*), Attack Determination, and Building Priority (e.g., Giants targeting defenses).  
  * **Flow:** Deployment \-\> Auto-Combat \-\> Win/Loss Determination.  
* **Maps:** Support for at least 2 different maps.  
* **Audio:** Background music and dynamic combat sound effects.

#### **Optional / Extension Features**

* Air Troops (e.g., Baby Dragon) and Air Defenses.  
* Traps & Bomb Systems.  
* Replay System.  
* Multiplayer/Alliance Systems.

### ---

### **Game Content & Assets**

#### **1\. Troops (Class Base: Troop)**

*Specification:* Each unit requires four state sequence frames: **Idle, Move, Attack, Die**.

* **Barbarian:** Melee unit.  
* **Archer:** Ranged unit (Projectile: Arrow).  
* **Giant:** High HP Tank (Prioritizes Defenses).  
* **Wall Breaker:** Carries Bomb (Kamikaze unit).  
* **Baby Dragon (Optional):** Air Unit (requires shadow rendering).

#### **2\. Buildings (Class Base: Building)**

*Specification:* Each building must provide visual appearances for 3 levels (lvl1, lvl2, lvl3).

* **Economy:** Gold Mine, Elixir Collector, Gold Storage, Elixir Storage.  
* **Military:** Barracks (Troop production), Army Camp.  
* **Defense:** Cannon (Rotating turret), Archer Tower, Air Defense (Optional), Wall.  
* **Core:** Town Hall.

#### **3\. VFX & Projectiles**

* **Projectiles:** Arrow, CannonBall, Rocket, FireBall.  
* **VFX:** Explosion (Wall Breaker/Shells), Fire Impact, Spawn Smoke (Deployment).

#### **4\. UI Elements**

* **HUD:** Unit overhead health bars (Background/Ally/Enemy).  
* **Icons:** Gold/Elixir resources.  
* **Controls:** Deployment Cards (Bottom screen), Selection Highlight Frames.

### ---

### **Architecture Philosophy**

* **Unidirectional Data Flow:** Avoid circular dependencies and minimize coupling.  
* **Clear Structure:** Maintain a logical, consistent hierarchy.  
* **Self-Contained Modules:** Keep modules independent where possible.  
* **Centralized Shared Definitions:** Place shared constants and variables in a core contract/module.

### ---

### **Project Structure**

The directory structure is strictly divided by developer responsibility to minimize merge conflicts.
Currently, its a draft. Reference only.

```text
Classes/
├── Contract/
│   ├── CONTRACT_GAPS.md
│   ├── Engine
│   ├── Gameplay
│   ├── GAP_RESOLUTION_LOG.md
│   ├── Integration
│   └── README.md
├── Core/
│   ├── GameConfig.cpp
│   ├── GameConfig.h
│   ├── GameConstants.h
│   ├── GameStructs.h
│   └── README.md
├── Engine/
│   ├── Input
│   ├── Logic
│   ├── MapSystem
│   ├── Public
│   └── RenderSystem
├── Gameplay/
│   ├── Components
│   ├── Entities
│   ├── Logic
│   └── Public
├── Integration/
│   ├── Docs/ (SOT)
        ├── DevC_Spec.md
        ├── Engine Module API Reference.md
        ├── Gameplay Cost Query API.md
        ├── Gameplay Module API Reference.md.md
        ├── Gameplay_Public_GameEvents_Documentation.md
        ├── Input Routing Policy.md
        ├── Placement Validation and TMX Conventions.md
        ├── readme_core.md
        ├── SaveLoad and Determinism API.md
        └── Scene Flow and Entrypoint.md 
│   ├── GameServices.cpp
│   └── GameServices.h
├── Main/
│   ├── AppDelegate.cpp
│   ├── AppDelegate.h
│   ├── TestScene.cpp
│   └── TestScene.h
├── Mocks/
│   ├── CocosShim
│   ├── EngineMock
│   ├── GameplayMock
│   ├── IntegrationMock
│   ├── README.md
│   └── Tests
└── Scenes/
    ├── GameScene.cpp
    └── GameScene.h

(below is a DRAFT TBD by dev C)
├── Managers/                                <-- Dev C orchestration (NEW) – “glue layer”, no Gameplay rules.
│   ├── LevelManager.h/.cpp                  <-- Owns map selection, spawn lists, authoritative all_buildings/all_units registry.
│   ├── UIManager.h/.cpp                     <-- UI flow + input routing; calls EconomySystem/GameConfig; never Gameplay internals.
│   ├── AudioManager.h/.cpp                  <-- Event→sound mapping + only place calling AudioEngine (wrapper + listener).
│   └── GameDirector.h/.cpp                  <-- (Optional) high-level state machine: BuildMode/BattleMode/Results.

├── Scenes/                                  <-- Dev C scenes (NEW) – keep Main/TestScene thin.
│   ├── GameScene.h/.cpp                     <-- The real scene: creates MapLayer + Managers + HUD overlay layers.
│   ├── BootScene.h/.cpp                     <-- (Optional) preload assets, go to menu/game.
│   └── MenuScene.h/.cpp                     <-- (Optional) main menu / map select.

├── UI/                                      <-- Dev C visuals (NEW) – purely presentation.
│   ├── HUD/                                 <-- Screen-space HUD (resource bars, popups).
│   │   ├── HUDLayer.h/.cpp
│   │   └── HealthBarWidget.h/.cpp
│   ├── Menus/                               <-- Shop, pause menu, results popup.
│   │   ├── ShopPanel.h/.cpp
│   │   └── ResultsPopup.h/.cpp
│   └── Widgets/                             <-- Reusable buttons/bars; no Gameplay includes.

└── Integration/                             <-- Dev C Contract + adapters (NEW, optional but strongly recommended)
    ├── Input/                               <-- “UI consumes first” helpers; translates cocos events into engine/game commands.
    │   └── InputRouter.h/.cpp
    ├── Events/                              <-- If GameEvents exists: event queue, adapters.
    │   └── GameEventBridge.h/.cpp
    └── Docs/                                <-- Dev C facing docs (or keep in root Docs/)
        ├── DevC_Contract.md
        ├── Dependencies.md
        └── Determinism.md
└── Main/
    ├── AppDelegate.cpp
    ├── AppDelegate.h
    ├── TestScene.cpp
    └── TestScene.h
```

### ---

**Responsibility and Dependency**

| Layer | Owner | Responsibility | Architectural Constraint |
| :---- | :---- | :---- | :---- |
| **App / UI** | **Dev C** (FullStack) | Menus, HUD, Input Handling, Audio | Can access Gameplay & Engine. |
| **Gameplay** | **Dev B** (Gameplay) | Rules, AI, Unit Stats, Map Logic | Can access Engine. **Cannot** access UI. |
| **Core Engine** | **Dev A** (Engine) | Rendering, Physics Math, Asset Loading | **Cannot** access Gameplay or UI. |

### ---

### **Naming Conventions**

We follow the **Google Style** naming conventions.

| Category | Convention | Example |
| :---- | :---- | :---- |
| **Types** (class/struct/enums) | PascalCase | GameConfig, UnitStats |
| **Methods** | PascalCase | GetTroopStats() |
| **Variables** | snake\_case\_ (member) snake\_case (local) | max\_hp\_, attack\_speed temp\_damage |
| **Enum values** | kPascalCase | kTownHall, kGiant |
| **Compile-time constants** | kPascalCase | kTileWidth, kMaxTroops |

#### **TODO Comments**

Use Google-style TODO annotations to assign tasks or mark incomplete features:

C++

// TODO(developer\_name): Explanation of what needs to be done.  
// Example:  
// TODO(DevB): Implement A\* pathfinding optimization here.  



### ---
### **DEV INFO**

**NOTE:** Windows is the primary supported build environment for now, but contributors (including you) may be on other OS (such as macOS). You MUST NOT introduce OS-locked solutions (such as Windows-only APIs) anywhere unless they’re explicitly isolated and labeled.

#### **Engine**

- Engine: **Cocos2d-x** (vendored/forked in-repo)
- Declared version: **4.0.0** (`COCOS2D_VERSION = 0x00040000`)
- Commit: `508fbe2cb50910bbc34e00d000cf700e67b38750`


#### **Build Environment (Locked)**


- Engine target: **Cocos2d-x v4.0** (CMake + Visual Studio 2022 workflow)
- C++ standard: **C++17**
- Windows toolchain:
  - Visual Studio: **Visual Studio 2022** (v17.x)
  - MSVC toolset: **v143** (CMake `-T v143`)
  - Architecture: **Win32** (CMake `-A win32`)
- CMake: **source-of-truth** for building. Minimum required CMake version in this codebase is 3.6.
  - Note: if legacy `.proj` files exist, treat them as secondary/obsolete compared to CMake.
- Third-party dependencies: fetched via `download-deps.py` (e.g., openssl/curl/zlib/etc.).
- Python: **Python 2.7.18** is mandatory for cocos setup tools. Do **not** add it to PATH (avoid system conflicts).

#### **Repo bootstrap (first-time setup)**

1. Install **Python 2.7.18** (do **not** add to PATH).
2. Run `python setup.py` in the **cocos root**. TODO(cocos_root_path): fill the path in this repo where `setup.py` lives.
3. Run `python download-deps.py` (Internet required) to fetch third-party libraries.
4. Configure the project with CMake (next section).

#### **Build & run (Windows, canonical)**

Open **Developer Command Prompt for VS 2022**, then:

```bat
cd /d <Project_Root>
cmake -S . -B build -G "Visual Studio 17 2022" -A win32 -T v143
cmake --build build --config Debug
````

* Build outputs typically go to: `build/bin/Debug/`

#### Run (Windows)

* Executable to run: TODO(exe_name): identify the produced `.exe` under `build/bin/Debug/`.

  * Possible (unconfirmed): `Clash_of_Clans_Combat_Test.exe` (based on window title in `AppDelegate.cpp`), or a default `cocos_test.exe`.
* Working directory must be a folder that contains `Resources/`.

  * If assets fail to load, set the working directory to the project root **or** ensure `Resources/` is copied into the build output folder.

#### Platform support

* Windows: supported (primary dev environment).
* iOS: planned (AudioEngine hooks exist in `AppDelegate`, but current docs are Windows-only).
* Android: out-of-scope (docs explicitly say to skip NDK setup).

#### Known build pitfalls (Windows)

* Missing runtime DLLs like `MSVCR100.dll` / `MSVCR110.dll` / `MSVCR120.dll`:

  * Cause: some prebuilt cocos v4 libraries depend on older MSVC runtimes.
  * Fix: install Visual C++ Redistributables for **VS2010 / VS2012 / VS2013** (**x86**).

#### iOS build steps

* Status: TBD. TODO(iOS_build_details): add Xcode version + signing + build instructions once available.


**Notes:** Adds the REPORT’s environment lock, bootstrap steps, copy-pasteable CMake/VS2022 build commands (Win32/v143), platform support notes, and the known DLL pitfall + fix, without changing existing project content.