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
