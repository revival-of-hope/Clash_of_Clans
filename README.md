# **CLASH OF CLANS (DRAFT)**

### **Overview**

This project is a real-time strategy (RTS) and simulation game. The game blends resource gathering, base building, troop training, and combat simulation, referencing the gameplay mechanics of "Clash of Clans".

Team Size: 3 Developers  
Timeline: 1 Month

### **Table of Contents**

1. [Core Features](https://www.google.com/search?q=%23core-features)  
2. [Game Content & Assets](https://www.google.com/search?q=%23game-content--assets)  
3. [Architecture Philosophy](https://www.google.com/search?q=%23architecture-philosophy)  
4. [Project Structure](https://www.google.com/search?q=%23project-structure)  
5. [Responsibility & Dependency](https://www.google.com/search?q=%23responsibility-and-dependency)  
6. [Coding Conventions](https://www.google.com/search?q=%23coding-conventions)
7. [Chinese Version(Machine Translated)]()

### ---

**Core Features**

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

**Game Content & Assets**

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

**Architecture Philosophy**

* **Unidirectional Data Flow:** Avoid circular dependencies and minimize coupling.  
* **Clear Structure:** Maintain a logical, consistent hierarchy.  
* **Self-Contained Modules:** Keep modules independent where possible.  
* **Centralized Shared Definitions:** Place shared constants and variables in a core contract/module.

### ---

**Project Structure**

The directory structure is strictly divided by developer responsibility to minimize merge conflicts.

```text
Classes/
├── Core/                       <-- SHARED TYPES (No Logic)
│   ├── GameConstants.h         (Team Colors, Screen Dims, UI Constants)
│   ├── GameEvents.h            (Observer Pattern: "OnUnitDied", "OnGoldChanged")
│   └── ResourceStructs.h       (Stats for Troops/Buildings loaded from JSON)
│
├── Engine/                     <-- DEV A (Rendering & Physics ONLY)
│   ├── RenderSystem/
│   │   ├── SpriteBatch.cpp
│   │   └── AnimationController.cpp
│   ├── MapSystem/
│   │   ├── IsometricGrid.cpp   (Math: Screen <-> Iso coords)
│   │   └── MapLayer.cpp        (Draws the tiles)
│   └── Input/
│       └── TouchHandler.cpp    (Raw input detection)
│
├── Gameplay/                   <-- DEV B (Logic & Rules ONLY)
│   ├── Entities/
│   │   ├── BaseEntity.h        (ID, Position, Owner)
│   │   ├── Unit.cpp            (Handles Move/Attack logic for ALL troops)
│   │   └── Building.cpp        (Handles Production/Defense logic)
│   ├── Components/             (Composition over Inheritance)
│   │   ├── HealthComp.cpp
│   │   ├── AttackComp.cpp      (Range, Damage, Rate)
│   │   └── PathAgent.cpp       (A* implementation)
│   └── Logic/
│       ├── CombatResolver.cpp  (Who hit whom?)
│       └── EconomySystem.cpp   (Resource ticks)
│
├── Managers/                   <-- DEV C (Orchestration)
│   ├── LevelManager.cpp        (Loads Map + Entities)
│   ├── AudioManager.cpp        (Listens to GameEvents)
│   └── UIManager.cpp           (The bridge between Logic and Screen)
│
└── UI/                         <-- DEV C (Visuals)
    ├── Widgets/                (Reusable buttons/bars)
    ├── HUDScene.cpp            (In-game overlay)
    └── Menus/                  (Main Menu, Shop)
```

### ---

**Responsibility and Dependency**

| Layer | Owner | Responsibility | Architectural Constraint |
| :---- | :---- | :---- | :---- |
| **App / UI** | **Dev C** (FullStack) | Menus, HUD, Input Handling, Audio | Can access Gameplay & Engine. |
| **Gameplay** | **Dev B** (Gameplay) | Rules, AI, Unit Stats, Map Logic | Can access Engine. **Cannot** access UI. |
| **Core Engine** | **Dev A** (Engine) | Rendering, Physics Math, Asset Loading | **Cannot** access Gameplay or UI. |

### ---

**Coding Conventions**

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


# **部落冲突 (草案)**

### **概览**

本项目是一个即时战略 (RTS) 与模拟游戏。游戏融合了资源采集、基地建设、部队训练和战斗模拟，参考了《部落冲突》(Clash of Clans) 的游戏机制。

团队规模：3 名开发人员
时间表：1 个月

### **目录**

1.  [核心功能](https://www.google.com/search?q=%23core-features)
2.  [游戏内容与资产](https://www.google.com/search?q=%23game-content--assets)
3.  [架构理念](https://www.google.com/search?q=%23architecture-philosophy)
4.  [项目结构](https://www.google.com/search?q=%23project-structure)
5.  [职责与依赖](https://www.google.com/search?q=%23responsibility-and-dependency)
6.  [编码规范](https://www.google.com/search?q=%23coding-conventions)

### \---

**核心功能**

游戏主要围绕涉及经济管理和自动战斗的“主村庄”中心系统展开。

  * **经济与资源：** 管理金币、圣水和人口/容量。
  * **建筑系统：** 包括资源生成（矿井）、兵营、防御塔和仓库。所有建筑至少支持 3 个升级等级。
  * **战斗系统：**
      * **部队：** 弓箭手、野蛮人、炸弹人 (Wall Breakers) 和巨人。
      * **AI：** 寻路 (A\*)、攻击判定和建筑优先级（例如巨人优先攻击防御设施）。
      * **流程：** 部署 -\> 自动战斗 -\> 胜负判定。
  * **地图：** 支持至少 2 张不同的地图。
  * **音频：** 背景音乐和动态战斗音效。

#### **可选 / 扩展功能**

  * 空中部队（如飞龙宝宝）和防空设施。
  * 陷阱与炸弹系统。
  * 回放系统。
  * 多人游戏/联盟系统。

### \---

**游戏内容与资产**

#### **1. 部队 (基类: Troop)**

*规范：* 每个单位需要四个状态序列帧：**待机 (Idle)、移动 (Move)、攻击 (Attack)、死亡 (Die)**。

  * **野蛮人 (Barbarian)：** 近战单位。
  * **弓箭手 (Archer)：** 远程单位（投射物：箭矢）。
  * **巨人 (Giant)：** 高生命值坦克（优先攻击防御设施）。
  * **炸弹人 (Wall Breaker)：** 携带炸弹（自杀式单位）。
  * **飞龙宝宝 (Baby Dragon) (可选)：** 空中单位（需要阴影渲染）。

#### **2. 建筑 (基类: Building)**

*规范：* 每个建筑必须提供 3 个等级的视觉外观 (lvl1, lvl2, lvl3)。

  * **经济：** 金矿、圣水收集器、储金罐、圣水瓶。
  * **军事：** 训练营（生产部队）、兵营。
  * **防御：** 加农炮（旋转炮塔）、箭塔、防空火箭（可选）、城墙。
  * **核心：** 大本营。

#### **3. 特效 (VFX) & 投射物**

  * **投射物：** 箭矢、加农炮弹、火箭、火球。
  * **特效：** 爆炸（炸弹人/炮弹）、火焰冲击、生成烟雾（部署时）。

#### **4. UI 元素**

  * **HUD：** 单位头顶血条（背景/友方/敌方）。
  * **图标：** 金币/圣水资源。
  * **控制：** 部署卡牌（屏幕底部）、选择高亮框。

### \---

**架构理念**

  * **单向数据流：** 避免循环依赖并最大限度地减少耦合。
  * **清晰的结构：** 保持逻辑清晰、一致的层级结构。
  * **自包含模块：** 尽可能保持模块独立。
  * **集中化共享定义：** 将共享常量和变量放置在核心契约/模块中。

### \---

**项目结构**

目录结构严格按开发人员职责划分，以最大限度地减少合并冲突。

```text
Classes/
├── Core/                       <-- 共享类型 (无逻辑)
│   ├── GameConstants.h         (团队颜色, 屏幕尺寸, UI 常量)
│   ├── GameEvents.h            (观察者模式: "OnUnitDied", "OnGoldChanged")
│   └── ResourceStructs.h       (从 JSON 加载的部队/建筑属性)
├── Engine/                     <-- 开发人员 A (仅限渲染与物理)
│   ├── RenderSystem/
│   │   ├── SpriteBatch.cpp
│   │   └── AnimationController.cpp
│   ├── MapSystem/
│   │   ├── IsometricGrid.cpp   (数学: 屏幕坐标 <-> 等轴坐标)
│   │   └── MapLayer.cpp        (绘制瓦片)
│   └── Input/
│       └── TouchHandler.cpp    (原始输入检测)
├── Gameplay/                   <-- 开发人员 B (仅限逻辑与规则)
│   ├── Entities/
│   │   ├── BaseEntity.h        (ID, 位置, 所有者)
│   │   ├── Unit.cpp            (处理所有部队的移动/攻击逻辑)
│   │   └── Building.cpp        (处理生产/防御逻辑)
│   ├── Components/             (组合优于继承)
│   │   ├── HealthComp.cpp
│   │   ├── AttackComp.cpp      (范围, 伤害, 速率)
│   │   └── PathAgent.cpp       (A* 实现)
│   └── Logic/
│       ├── CombatResolver.cpp  (判定谁击中了谁?)
│       └── EconomySystem.cpp   (资源Tick)
├── Managers/                   <-- 开发人员 C (编排)
│   ├── LevelManager.cpp        (加载地图 + 实体)
│   ├── AudioManager.cpp        (监听游戏事件)
│   └── UIManager.cpp           (逻辑与屏幕之间的桥梁)
└── UI/                         <-- 开发人员 C (视觉)
    ├── Widgets/                (可复用的按钮/条)
    ├── HUDScene.cpp            (游戏内覆盖层)
    └── Menus/                  (主菜单, 商店)
```

### \---

**职责与依赖**

| 层级 | 负责人 | 职责 | 架构约束 |
| :---- | :---- | :---- | :---- |
| **App / UI** | **Dev C** (全栈) | 菜单, HUD, 输入处理, 音频 | 可以访问 Gameplay 和 Engine。 |
| **Gameplay** | **Dev B** (玩法) | 规则, AI, 单位属性, 地图逻辑 | 可以访问 Engine。**不能**访问 UI。 |
| **Core Engine** | **Dev A** (引擎) | 渲染, 物理数学, 资产加载 | **不能**访问 Gameplay 或 UI。 |

### \---

**编码规范**

我们遵循 **Google Style** 命名约定。

| 类别 | 约定 | 示例 |
| :---- | :---- | :---- |
| **类型** (class/struct/enums) | PascalCase (帕斯卡命名法) | GameConfig, UnitStats |
| **方法** | PascalCase (帕斯卡命名法) | GetTroopStats() |
| **变量** | snake\_case\_ (成员变量) snake\_case (局部变量) | max\_hp\_, attack\_speed temp\_damage |
| **枚举值** | kPascalCase | kTownHall, kGiant |
| **编译时常量** | kPascalCase | kTileWidth, kMaxTroops |

#### **TODO 注释**

使用 Google 风格的 TODO 注解来分配任务或标记未完成的功能：

C++

// TODO(developer\_name): Explanation of what needs to be done.  
// Example:  
// TODO(DevB): Implement A\* pathfinding optimization here.
