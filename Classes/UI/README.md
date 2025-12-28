# UI State Models (Stage 4)

## What they are
Plain C++ models in `UiStateModels.h` that store deterministic UI-facing state derived only from `Classes/Contract/Gameplay/GameEvents.h`:
- HUD: gold/elixir values and capacities
- Selection highlight: current entity id (clears on destroy)
- Health bars: per-entity hp (current/max) and ally/enemy flag
- Deployment bar: selected troop + remaining counts per owner
- `UiStateStore`: aggregates all of the above and exposes a snapshot for rendering/presentation layers

## How UiStateStore attaches
`UiStateStore` implements `Gameplay::IGameEventListener`. Call `Attach(Gameplay::GameEventManager*)` to start receiving contract events and `Detach(...)` to stop. Attach/detach are idempotent; pre-attach and post-detach broadcasts are ignored by the store.

## Snapshots
`GetSnapshot()` returns a value type (`UiStateSnapshot`) containing HUD, selection, health bar, and deployment slices. No engine or Cocos types appear in the snapshot; rendering layers consume these values however they wish.

## Non-goals
- No rendering code or widget references
- No gameplay rules or predictions (state is driven only by contract events)
- No engine/Cocos dependencies beyond the gameplay contract listener interface

## Tests (Stage 4 coverage)
Run the mock-only suite to validate the state models:

```bash
cmake -S . -B build -DUSE_COCOS_ENGINE=OFF -DBUILD_APP=OFF -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```