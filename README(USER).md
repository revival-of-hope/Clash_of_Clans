# Clash of Clans (RTS 模拟)

**devC note: Please take the English version as the standard/authoritative one.**
## 概览 (Overview)

《部落冲突》是一款专注于村庄建设、资源经济、军队训练和自动战斗的实时战略 (RTS) 与模拟游戏。本项目提供了一套完整的玩法循环，并采用健壮的架构将引擎、玩法逻辑和 UI 关注点分离，从而支持快速迭代和可维护的集成。

## 核心功能 (Core Features)

* **经济与资源：** 金币、圣水以及人口/容量管理。
* **建筑系统：** 资源生产、存储、军事建筑、防御设施，以及具有多个升级等级的大本营进阶体系。
* **战斗系统：** 单位部署、自动战斗、寻路以及目标选择逻辑。
* **军队：** 野蛮人、弓箭手、巨人、炸弹人，以及可选的空中单位（如飞龙宝宝）。
* **地图：** 支持确定性启动的多张战斗地图。
* **音频：** 音乐、UI 反馈和战斗音效。

## 游戏内容 (Game Content)

### 军队 (Troops)

每个单位都支持核心动画状态（待机、移动、攻击、死亡）和独特的战斗定位。

### 建筑 (Buildings)

包含经济、军事、防御和核心建筑，具备多个升级等级和不同阶段的视觉表现。

### 特效与投射物 (VFX & Projectiles)

投射物类型和命中特效旨在提升战斗的可读性与反馈感。

### UI 元素 (UI Elements)

HUD、资源条、部署卡牌、选中高亮和结算界面。

## 架构 (Architecture)

本项目围绕严格的职责分离和单向依赖进行组织：

* **引擎 (Engine)：** 渲染、物理、资产加载和平台集成。
* **玩法 (Gameplay)：** 规则、AI、实体、战斗逻辑和确定性游戏状态。
* **应用/UI (App/UI)：** 场景、菜单、HUD、输入处理和表现逻辑。

这种分离保持了架构的清晰度，防止了循环依赖，并允许每个子系统独立演进。

## 项目结构 (当前)

```
Classes/
├── Contract/            # [规范] 公共 API 表面 (Canonical public API surface)
├── Core/                # 共享常量、结构体和配置
├── Engine/              # 渲染、输入、地图系统
├── Gameplay/            # 实体、逻辑、规则、AI
├── Integration/         # 应用级集成服务
├── Main/                # 应用入口点 (AppDelegate, bootstrap)
├── Managers/            # 编排层 (音频, 关卡, UI)
├── Mocks/               # 确定性模拟 (Mocks) 和测试
├── Scenes/              # 启动/菜单/游戏/结算 场景流
└── UI/                  # HUD 和表现组件

```

## 场景流 (Scene Flow)

规范的流程为：
**启动 (Boot) → 菜单 (Menu) → 游戏 (Game) → 结算 (Results)**

这保持了用户旅程的清晰和集成层的稳定，同时支持 UI 和玩法的快速迭代。

## 开发工作流优势 (Development Workflow Advantages)

本项目专为可维护性和开发速度而设计：

* **契约优先 API (Contract-first API)：** 系统之间清晰的公共接口表面减少了集成阻力。
* **模拟优先测试 (Mock-first testing)：** 确定性测试实现了可靠的 UI 和流程验证，而无需与引擎运行时耦合。
* **单向数据流 (Unidirectional data flow)：** 可预测的状态更新使得调试和扩展更加安全。
* **文档驱动集成 (Documentation-driven integration)：** 集中化的规范和模块边界最小化了歧义。
* **分层所有权 (Layered ownership)：** 团队职责明确，防止意外的跨层耦合。

## 构建与运行 (Build & Run)

有关构建和运行说明，请参阅代码仓库中的项目脚本和特定平台的设置指南。

# Clash of Clans (RTS Simulation)

## Overview
Clash of Clans is a real-time strategy and simulation game focused on village building, resource economy, troop training, and automated combat. The project provides a complete gameplay loop with a robust architecture that separates engine, gameplay, and UI concerns while enabling fast iteration and maintainable integration.

## Core Features
- **Economy & Resources:** Gold, Elixir, and population/capacity management.
- **Building System:** Resource production, storage, military buildings, defenses, and Town Hall progression with multiple upgrade levels.
- **Combat System:** Unit deployment, automated combat, pathfinding, and target selection logic.
- **Troops:** Barbarian, Archer, Giant, Wall Breaker, and optional air units (e.g., Baby Dragon).
- **Maps:** Multiple battle maps with deterministic launches.
- **Audio:** Music, UI feedback, and combat sound effects.

## Game Content
### Troops
Each unit supports core animation states (idle, move, attack, die) and distinct combat roles.

### Buildings
Includes economy, military, defense, and core structures with multiple upgrade levels and visual stages.

### VFX & Projectiles
Projectile types and impact effects support combat readability and feedback.

### UI Elements
HUD, resource bars, deployment cards, selection highlighting, and results screens.

## Architecture
The project is organized around strict separation of responsibilities with unidirectional dependencies:

- **Engine:** Rendering, physics, asset loading, and platform integration.
- **Gameplay:** Rules, AI, entities, combat logic, and deterministic game state.
- **App/UI:** Scenes, menus, HUD, input handling, and presentation logic.

This separation preserves clarity, prevents circular dependencies, and allows each subsystem to evolve independently.

## Project Structure (Current)
```
Classes/
├── Contract/            # Canonical public API surface
├── Core/                # Shared constants, structs, and config
├── Engine/              # Rendering, input, map systems
├── Gameplay/            # Entities, logic, rules, AI
├── Integration/         # App-level integration services
├── Main/                # App entry points (AppDelegate, bootstrap)
├── Managers/            # Orchestration layer (audio, level, UI)
├── Mocks/               # Deterministic mocks and tests
├── Scenes/              # Boot/Menu/Game/Results scene flow
└── UI/                  # HUD and presentation components
```

## Scene Flow
The canonical flow is:
**Boot → Menu → Game → Results**

This keeps the user journey clear and the integration layer stable while supporting fast iteration on UI and gameplay.

## Development Workflow Advantages
This project is designed for maintainability and speed:

- **Contract-first API:** A clear public surface between systems reduces integration friction.
- **Mock-first testing:** Deterministic tests enable reliable UI and flow validation without engine runtime coupling.
- **Unidirectional data flow:** Predictable state updates make debugging and extension safer.
- **Documentation-driven integration:** Centralized specs and module boundaries minimize ambiguity.
- **Layered ownership:** Team responsibilities are explicit, preventing accidental cross-layer coupling.

## Build & Run
See project scripts and platform-specific setup guides in the repository for build and runtime instructions.
