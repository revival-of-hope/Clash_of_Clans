# **CLASH OF CLANS (DEV-FACING-README)**

### **Overview**

This project is a real-time strategy (RTS) and simulation game. The game blends resource gathering, base building, troop training, and combat simulation, referencing the Gameplay mechanics of "Clash of Clans".

Team Size: 3 Developers

Timeline: 1 Month

### **Table of Contents**

**Please take the English version as the standard/authoritative one.**

1. [Core Features](https://www.google.com/search?q=%23core-features)
2. [Game Content & Assets](https://www.google.com/search?q=%23game-content--assets)
3. [Architecture Philosophy](https://www.google.com/search?q=%23architecture-philosophy)
4. [Project Structure](https://www.google.com/search?q=%23project-structure)
5. [Responsibility & Dependency](https://www.google.com/search?q=%23responsibility-and-dependency)
6. [Coding Conventions](https://www.google.com/search?q=%23coding-conventions)
7. [Dev Info & Build Instructions](https://www.google.com/search?q=%23dev-info)
8. [中文版本 (Chinese Version)](https://www.google.com/search?q=%23%E4%B8%AD%E6%96%87%E7%89%88%E6%9C%AC-chinese-version)

### ---

### **Core Features**

The game focuses on a "Main Village" hub system involving economy management and automated combat.

* **Economy & Resources:** Management of Gold, Elixir, and Population/Capacity.
* **Building System:** Includes Resource Generation (Mines), Army Camps, Defense Towers, and Storage. All buildings support at least 3 upgrade levels.
* **Combat System:** * **Troops:** Archers, Barbarians, Wall Breakers, and Giants.
* **AI:** Pathfinding (A*), Attack Determination, and Building Priority (e.g., Giants targeting defenses).
* **Flow:** Deployment -> Auto-Combat -> Win/Loss Determination.


* **Maps:** Support for at least 2 different maps.
* **Audio:** Background music and dynamic combat sound effects.

#### **Optional / Extension Features**

* Air Troops (e.g., Baby Dragon) and Air Defenses.
* Traps & Bomb Systems.
* Replay System.
* Multiplayer/Alliance Systems.

### ---

### **Game Content & Assets**

#### **1. Troops (Class Base: Troop)**

*Specification:* Each unit requires four state sequence frames: **Idle, Move, Attack, Die**.

* **Barbarian:** Melee unit.
* **Archer:** Ranged unit (Projectile: Arrow).
* **Giant:** High HP Tank (Prioritizes Defenses).
* **Wall Breaker:** Carries Bomb (Kamikaze unit).
* **Baby Dragon (Optional):** Air Unit (requires shadow rendering).

#### **2. Buildings (Class Base: Building)**

*Specification:* Each building must provide visual appearances for 3 levels (lvl1, lvl2, lvl3).

* **Economy:** Gold Mine, Elixir Collector, Gold Storage, Elixir Storage.
* **Military:** Barracks (Troop production), Army Camp.
* **Defense:** Cannon (Rotating turret), Archer Tower, Air Defense (Optional), Wall.
* **Core:** Town Hall.

#### **3. VFX & Projectiles**

* **Projectiles:** Arrow, CannonBall, Rocket, FireBall.
* **VFX:** Explosion (Wall Breaker/Shells), Fire Impact, Spawn Smoke (Deployment).

#### **4. UI Elements**

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
├── Contract/               # [PUBLIC API] The only supported interface for cross-module communication.
│   ├── Engine/             # Interfaces for MapLayer, Input, and Rendering.
│   ├── Gameplay/           # Interfaces for Units, Buildings, Economy, and Combat.
│   ├── Integration/        # Interfaces for SaveLoad, PlayerIdentity, and SceneFlow services.
│   └── README.md           # Defines the "canonical public API" policy.
│
├── Core/                   # [SHARED DATA] Authoritative read-only source of truth.
│   ├── GameConfig.h/cpp    # Global tunables and lookup tables (e.g., troop costs).
│   ├── GameConstants.h     # Grid size, physics tags, and global enums.
│   └── GameStructs.h       # Shared data structures (UnitStats, BuildingData).
│
├── Engine/                 # [LOW-LEVEL SYSTEMS] Cocos2d-x wrappers and core tech.
│   ├── Input/              # MouseController and input routing.
│   ├── Logic/              # Engine test scenes.
│   ├── MapSystem/          # TMX map loading (MapLayer) and tile highlighting.
│   └── RenderSystem/       # TilePlacementController for visual building placement.
│
├── Gameplay/               # [GAME LOGIC] internal implementation (Hidden behind Contract).
│   ├── Components/         # HealthComp, AttackComp, PathAgent, etc.
│   ├── Entities/           # Unit, Building, and BaseEntity implementations.
│   └── Logic/              # Systems: EconomySystem, CombatResolver, TroopCommand.
│
├── Integration/            # [GLUE CODE] Services connecting Engine, Gameplay, and App.
│   ├── Docs/               # Technical documentation and API references.
│   ├── Engine/             # InputRouter and CocosAudioSink implementations.
│   └── SceneFlowService    # Manages transitions between Boot, Menu, Game, and Results.
│
├── Managers/               # [GLOBAL MANAGERS]
│   ├── AudioManager/       # Handles background music and sound effects (uses CocosAudioSink).
│   └── LevelManager/       # Source of truth for map selection (Stage 6) and seeds.
│
├── Mocks/                  # [TEST DOUBLES] Deterministic stand-ins for testing.
│   ├── CocosShim/          # Stubs for Cocos2d-x types to allow headless testing.
│   ├── GameplayMock/       # In-memory versions of Gameplay systems.
│   └── IntegrationMock/    # Mock services for Save/Load and SceneFlow.
│
├── Scenes/                 # [APP FLOW] High-level scene definitions.
│   ├── BootScene           # Splash/Loading entry point.
│   ├── MenuScene           # Map selection (Map A/B) and game launch.
│   ├── GameScene           # Main battle loop (BattleLaunchParams).
│   └── ResultsScene        # Battle summary and settlement.
│
└── UI/                     # [USER INTERFACE] Pure logic models for UI state.
    ├── UiStateModels.h     # Deterministic state for HUD, selection, and health bars.
    └── README.md           # Documentation for UI state snapshots.

Resources/                  # [ASSETS]

```

### ---

### **Responsibility and Dependency**

| Layer | Owner | Responsibility | Architectural Constraint |
| --- | --- | --- | --- |
| **App / UI** | **Dev C** (FullStack) | Menus, HUD, Input Handling, Audio | Can access Gameplay & Engine. |
| **Gameplay** | **Dev B** (Gameplay) | Rules, AI, Unit Stats, Map Logic | Can access Engine. **Cannot** access UI. |
| **Core Engine** | **Dev A** (Engine) | Rendering, Physics Math, Asset Loading | **Cannot** access Gameplay or UI. |

### ---

### **Naming Conventions**

We follow the **Google Style** naming conventions.

| Category | Convention | Example |
| --- | --- | --- |
| **Types** (class/struct/enums) | PascalCase | GameConfig, UnitStats |
| **Methods** | PascalCase | GetTroopStats() |
| **Variables** | snake_case_ (member) snake_case (local) | max_hp_, attack_speed temp_damage |
| **Enum values** | kPascalCase | kTownHall, kGiant |
| **Compile-time constants** | kPascalCase | kTileWidth, kMaxTroops |

#### **TODO Comments**

Use Google-style TODO annotations to assign tasks or mark incomplete features:

```cpp
// TODO(developer_name): Explanation of what needs to be done.  
// Example:  
// TODO(DevB): Implement A* pathfinding optimization here.  

```

### ---

### **DEV INFO**

**NOTE:** Windows is the primary supported build environment for now, but contributors (including you) may be on other OS (such as macOS). You MUST NOT introduce OS-locked solutions (such as Windows-only APIs) anywhere unless they’re explicitly isolated and labeled.

#### **Engine**

* Engine: **Cocos2d-x** (vendored/forked in-repo)
* Declared version: **4.0.0** (`COCOS2D_VERSION = 0x00040000`)
* Commit: `508fbe2cb50910bbc34e00d000cf700e67b38750`

#### **Build Environment (Locked)**

* Engine target: **Cocos2d-x v4.0** (CMake + Visual Studio 2022 workflow)
* C++ standard: **C++17**
* Windows toolchain:
* Visual Studio: **Visual Studio 2022** (v17.x)
* MSVC toolset: **v143** (CMake `-T v143`)
* Architecture: **Win32** (CMake `-A win32`)


* CMake: **source-of-truth** for building. Minimum required CMake version in this codebase is 3.6.
* Note: if legacy `.proj` files exist, treat them as secondary/obsolete compared to CMake.


* Third-party dependencies: fetched via `download-deps.py` (e.g., openssl/curl/zlib/etc.).
* Python: **Python 2.7.18** is mandatory for cocos setup tools. Do **not** add it to PATH (avoid system conflicts).

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

```

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

---

---

# **中文版本 (Chinese Version)**

# **CLASH OF CLANS (开发人员自述文档)**

### **概览 (Overview)**

本项目是一款实时战略 (RTS) 与模拟游戏。游戏融合了资源采集、基地建设、军队训练以及战斗模拟等元素，参考了《部落冲突 (Clash of Clans)》的玩法机制。

团队规模：3 名开发者

开发周期：1 个月

### **目录**

**请注意：所有内容以英文版本为准。**

1. 核心功能 (Core Features)
2. 游戏内容与资产 (Game Content & Assets)
3. 架构理念 (Architecture Philosophy)
4. 项目结构 (Project Structure)
5. 职责与依赖 (Responsibility & Dependency)
6. 代码规范 (Coding Conventions)
7. 开发环境与构建说明 (Dev Info)

### ---

### **核心功能 (Core Features)**

游戏围绕“主村庄”枢纽系统展开，包含经济管理和自动战斗。

* **经济与资源：** 管理金币 (Gold)、圣水 (Elixir) 和人口/容量 (Population/Capacity)。
* **建筑系统：** 包括资源生产（采集器）、兵营、防御塔和仓库。所有建筑至少支持 3 个升级等级。
* **战斗系统：**
* **军队：** 弓箭手 (Archers)、野蛮人 (Barbarians)、炸弹人 (Wall Breakers) 和巨人 (Giants)。
* **AI：** 寻路 (A*)、攻击判定和建筑优先级（例如：巨人优先攻击防御设施）。
* **流程：** 部署 -> 自动战斗 -> 胜负判定。


* **地图：** 支持至少 2 张不同的地图。
* **音频：** 背景音乐和动态战斗音效。

#### **可选 / 扩展功能**

* 空中部队（例如：飞龙宝宝）和防空设施。
* 陷阱与炸弹系统。
* 回放系统。
* 多人/联盟系统。

### ---

### **游戏内容与资产 (Game Content & Assets)**

#### **1. 军队 (基类: Troop)**

*规范：* 每个单位需要四种状态序列帧：**待机 (Idle)、移动 (Move)、攻击 (Attack)、死亡 (Die)**。

* **Barbarian (野蛮人):** 近战单位。
* **Archer (弓箭手):** 远程单位（投射物：箭矢）。
* **Giant (巨人):** 高生命值坦克（优先攻击防御设施）。
* **Wall Breaker (炸弹人):** 携带炸弹（自杀式单位）。
* **Baby Dragon (飞龙宝宝 - 可选):** 空中单位（需要阴影渲染）。

#### **2. 建筑 (基类: Building)**

*规范：* 每个建筑必须提供 3 个等级的视觉外观 (lvl1, lvl2, lvl3)。

* **经济：** 金矿、圣水收集器、储金罐、圣水瓶。
* **军事：** 训练营（生产军队）、兵营。
* **防御：** 加农炮（旋转炮塔）、箭塔、防空火箭（可选）、城墙。
* **核心：** 大本营 (Town Hall)。

#### **3. 特效与投射物 (VFX & Projectiles)**

* **投射物：** 箭矢、加农炮弹、火箭、火球。
* **特效：** 爆炸（炸弹人/炮弹）、火焰冲击、生成烟雾（部署时）。

#### **4. UI 元素**

* **HUD：** 单位头顶血条（背景/己方/敌方）。
* **图标：** 金币/圣水资源。
* **控制：** 部署卡牌（屏幕底部）、选中高亮框。

### ---

### **架构理念 (Architecture Philosophy)**

* **单向数据流 (Unidirectional Data Flow)：** 避免循环依赖，最小化耦合。
* **清晰的结构：** 保持逻辑清晰、一致的层级结构。
* **自包含模块 (Self-Contained Modules)：** 尽可能保持模块独立。
* **集中式共享定义：** 将共享常量和变量放置在核心契约/模块中。

### ---

### **项目结构 (Project Structure)**

目录结构严格按开发者职责划分，以最大限度减少合并冲突。
目前为草案，仅供参考。

```text
Classes/
├── Contract/               # [公共 API] 跨模块通信的唯一支持接口。
│   ├── Engine/             # 地图层、输入和渲染的接口。
│   ├── Gameplay/           # 单位、建筑、经济和战斗的接口。
│   ├── Integration/        # 存档、玩家身份和场景流服务的接口。
│   └── README.md           # 定义“规范化公共 API”策略。
│
├── Core/                   # [共享数据] 权威的只读事实来源 (Source of Truth)。
│   ├── GameConfig.h/cpp    # 全局可调参数和查找表（如：军队造价）。
│   ├── GameConstants.h     # 网格大小、物理标签和全局枚举。
│   └── GameStructs.h       # 共享数据结构 (UnitStats, BuildingData)。
│
├── Engine/                 # [底层系统] Cocos2d-x 封装和核心技术。
│   ├── Input/              # 鼠标控制器和输入路由。
│   ├── Logic/              # 引擎测试场景。
│   ├── MapSystem/          # TMX 地图加载 (MapLayer) 和瓦片高亮。
│   └── RenderSystem/       # 用于视觉建筑放置的 TilePlacementController。
│
├── Gameplay/               # [游戏逻辑] 内部实现（隐藏在 Contract 之后）。
│   ├── Components/         # HealthComp, AttackComp, PathAgent 等。
│   ├── Entities/           # Unit, Building 和 BaseEntity 的实现。
│   └── Logic/              # 系统：EconomySystem, CombatResolver, TroopCommand。
│
├── Integration/            # [胶水代码] 连接引擎、玩法和应用的服务。
│   ├── Docs/               # 技术文档和 API 参考。
│   ├── Engine/             # InputRouter 和 CocosAudioSink 实现。
│   └── SceneFlowService    # 管理 Boot, Menu, Game 和 Results 之间的转换。
│
├── Managers/               # [全局管理器]
│   ├── AudioManager/       # 处理背景音乐和音效（使用 CocosAudioSink）。
│   └── LevelManager/       # 地图选择（第 6 关）和种子的事实来源。
│
├── Mocks/                  # [测试替身] 用于测试的确定性替代品。
│   ├── CocosShim/          # Cocos2d-x 类型的桩代码，允许无头测试。
│   ├── GameplayMock/       # 玩法系统的内存版本。
│   └── IntegrationMock/    # 存档和场景流的模拟服务。
│
├── Scenes/                 # [应用流程] 高级场景定义。
│   ├── BootScene           # 启动/加载入口点。
│   ├── MenuScene           # 地图选择 (Map A/B) 和游戏启动。
│   ├── GameScene           # 主战斗循环 (BattleLaunchParams)。
│   └── ResultsScene        # 战斗总结和结算。
│
└── UI/                     # [用户界面] UI 状态的纯逻辑模型。
    ├── UiStateModels.h     # HUD、选择和血条的确定性状态。
    └── README.md           # UI 状态快照文档。

Resources/                  # [资产]

```

### ---

### **职责与依赖 (Responsibility and Dependency)**

| 层级 | 负责人 | 职责 | 架构约束 |
| --- | --- | --- | --- |
| **App / UI** | **Dev C** (FullStack) | 菜单, HUD, 输入处理, 音频 | 可访问 Gameplay & Engine。 |
| **Gameplay** | **Dev B** (Gameplay) | 规则, AI, 单位数值, 地图逻辑 | 可访问 Engine。 **不可**访问 UI。 |
| **Core Engine** | **Dev A** (Engine) | 渲染, 物理数学, 资产加载 | **不可**访问 Gameplay 或 UI。 |

### ---

### **命名规范 (Naming Conventions)**

我们遵循 **Google Style** 命名规范。

| 类别 | 规范 | 示例 |
| --- | --- | --- |
| **Types** (类/结构体/枚举) | PascalCase (帕斯卡命名法) | GameConfig, UnitStats |
| **Methods** (方法) | PascalCase | GetTroopStats() |
| **Variables** (变量) | snake_case_ (成员变量)<br>

<br>snake_case (局部变量) | max_hp_, attack_speed<br>

<br>temp_damage |
| **Enum values** (枚举值) | kPascalCase | kTownHall, kGiant |
| **Compile-time constants** (编译时常量) | kPascalCase | kTileWidth, kMaxTroops |

#### **TODO 注释**

使用 Google 风格的 TODO 注解来分配任务或标记未完成的功能：

```cpp
// TODO(developer_name): 说明需要做什么。
// Example:
// TODO(DevB): 在此处实现 A* 寻路优化。

```

### ---

### **开发信息 (DEV INFO)**

**注意：** 目前 Windows 是主要支持的构建环境，但贡献者（包括你）可能使用其他操作系统（如 macOS）。**严禁**引入锁定特定操作系统的解决方案（如仅限 Windows 的 API），除非将它们显式隔离并标记。

#### **引擎 (Engine)**

* 引擎: **Cocos2d-x** (仓库内置/派生版本)
* 声明版本: **4.0.0** (`COCOS2D_VERSION = 0x00040000`)
* 提交哈希: `508fbe2cb50910bbc34e00d000cf700e67b38750`

#### **构建环境 (锁定)**

* 引擎目标: **Cocos2d-x v4.0** (CMake + Visual Studio 2022 工作流)
* C++ 标准: **C++17**
* Windows 工具链:
* Visual Studio: **Visual Studio 2022** (v17.x)
* MSVC 工具集: **v143** (CMake `-T v143`)
* 架构: **Win32** (CMake `-A win32`)


* CMake: 构建的**单一事实来源 (Source-of-truth)**。此代码库要求的最低 CMake 版本为 3.6。
* 注意：如果存在旧版 `.proj` 文件，请将其视为次要/过时文件，以 CMake 为准。


* 第三方依赖：通过 `download-deps.py` 获取（例如：openssl/curl/zlib 等）。
* Python: **Python 2.7.18** 是 cocos 设置工具的强制要求。请**不要**将其添加到 PATH（避免系统冲突）。

#### **仓库引导 (首次设置)**

1. 安装 **Python 2.7.18** (请**不要**添加到 PATH)。
2. 在 **cocos root** 目录下运行 `python setup.py`。 TODO(cocos_root_path): 填写本仓库中 `setup.py` 所在的路径。
3. 运行 `python download-deps.py`（需要联网）以获取第三方库。
4. 使用 CMake 配置项目（见下一节）。

#### **构建与运行 (Windows, 标准流程)**

打开 **Developer Command Prompt for VS 2022**，然后执行：

```bat
cd /d <Project_Root>
cmake -S . -B build -G "Visual Studio 17 2022" -A win32 -T v143
cmake --build build --config Debug

```

* 构建输出通常位于：`build/bin/Debug/`

#### 运行 (Windows)

* 运行的可执行文件：TODO(exe_name): 确认 `build/bin/Debug/` 下生成的 `.exe` 文件名。
* 可能（未确认）：`Clash_of_Clans_Combat_Test.exe`（基于 `AppDelegate.cpp` 中的窗口标题），或默认的 `cocos_test.exe`。


* 工作目录必须是包含 `Resources/` 的文件夹。
* 如果资产加载失败，请将工作目录设置为项目根目录，**或**确保 `Resources/` 已复制到构建输出文件夹中。



#### 平台支持

* Windows: 支持（主要开发环境）。
* iOS: 计划中（AudioEngine 钩子存在于 `AppDelegate` 中，但当前文档仅适用于 Windows）。
* Android: 超出范围（文档明确指出跳过 NDK 设置）。

#### 已知构建陷阱 (Windows)

* 缺少运行时 DLL，如 `MSVCR100.dll` / `MSVCR110.dll` / `MSVCR120.dll`：
* 原因：某些预构建的 cocos v4 库依赖于旧版 MSVC 运行时。
* 修复：安装 **VS2010 / VS2012 / VS2013** 的 Visual C++ Redistributables (**x86**)。



#### iOS 构建步骤

* 状态：待定 (TBD)。 TODO(iOS_build_details): 待可用时添加 Xcode 版本 + 签名 + 构建说明。