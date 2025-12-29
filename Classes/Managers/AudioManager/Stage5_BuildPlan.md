# Stage 5 Prep — AudioManager (Event → Sound Mapping)

## Repo Anatomy (inspected sources)
- DevC roadmap: `DevC_roadmap.md`.
- Repository structure and planned Managers folder: `README.md` (Managers draft section).
- Game event Contract: `Classes/Contract/Gameplay/GameEvents.h`.
- Gameplay event usage guide: `Classes/Integration/Docs/Gameplay_Public_GameEvents_Documentation.md`.
- GameServices wiring (composition root): `Classes/Integration/GameServices.h` and `Classes/Integration/GameServices.cpp`.
- Scene bootstrap usage of GameServices: `Classes/Main/AppDelegate.cpp`.
- Current scene listener example: `Classes/Scenes/GameScene.cpp`.
- Mock event bus implementation: `Classes/Mocks/GameplayMock/GameEvents.cpp`.
- Existing mock/test harness style: `Classes/Mocks/Tests/MockTests.cpp`.
- Mock policy and defaults: `Classes/Mocks/README.md`.

## AudioManager placement and dependencies
- **Placement:** create `Classes/Managers/AudioManager/AudioManager.h` and `AudioManager.cpp` (matches repository draft layout in `README.md`).
- **Contract to depend on:**
  - `Gameplay::IGameEventListener` and `Gameplay::GameEventManager` from `Classes/Contract/Gameplay/GameEvents.h` for event subscription.
  - Audio sink contract `Classes/Contract/Engine/AudioSink.h` (minimal `IAudioSink` defined below) rather than direct engine headers.
- **Avoid:** linking to gameplay/engine internals beyond contract headers; avoid pulling cocos audio engine directly until a contract surface exists; do not depend on asset scanning.

## Build plan (step-by-step)
1. **Define audio sink contract (if provided):** add a minimal interface (e.g., `PlaySound(id, loop=false)`, `StopAll()`), or wire to existing contract once specified; keep include path under `Classes/Contract/Engine` to align with Dev C boundaries.
2. **Implement AudioManager class:**
   - Inherit from `Gameplay::IGameEventListener`.
   - Accept pointers to `Gameplay::GameEventManager` and the audio sink in ctor/setup.
   - Register with `GameEventManager` on `Start()`/construction and deregister on `Stop()`/destruction (idempotent safety like UI store).
   - Maintain event→sound ID map loaded from documented resource table (pending SOT); expose injection hook for tests.
   - Handle relevant callbacks (`OnProjectileFired`, `OnProjectileHit`, `OnEntityDestroyed`, `OnBattleStarted`, `OnBattleEnded`, etc.) and invoke audio sink accordingly; ensure no-op when sink or map missing.
3. **Wire through GameServices/composition root:**
   - Extend `Integration::ResolveGameEventManager()` usage to construct/configure `AudioManager` alongside UI wiring (likely in scene creation or a new manager factory once audio sink contract exists).
   - Keep mocks enabled (`kUseMocks = true`) and use mock sink implementation in tests.
4. **Create mock audio sink:**
   - Implement under `Classes/Mocks/EngineMock` (or new `AudioMock`) a deterministic sink that records `PlaySound` calls with IDs/loop flags and exposes counters for assertions.
5. **Author tests (see below) in `Classes/Mocks/Tests` ensuring they only depend on Contract + mocks.**
6. **Document resource mapping:** once the missing `Classes/Managers/AudioManager/README.md` (resource inventory) is available, encode mappings as constants and cite the source; until then, block on gap or accept injected mapping in tests.

## Test plan
- **AudioManager registers/unregisters:** create an instance with mock sink and mock `GameEventManager`; assert listener count changes and deregistration on teardown.
- **Event→sound mapping:** broadcast specific gameplay events (e.g., `OnProjectileFired`, `OnProjectileHit`, `OnBattleStarted/Ended`, `OnEntityDestroyed`) and assert the mock sink recorded the expected sound IDs according to the mapping table.
- **No stray sounds:** after initialization without events, verify no sink calls; also test unrelated events (e.g., `OnResourceChanged`) produce zero audio calls.
- **No sink/no map safety:** construct AudioManager without an injected sink or mapping to confirm callbacks do not crash and do not record plays.
- **Idempotent attach/detach:** calling `Start()` twice or `Stop()` twice should not double-register or throw; verify mock sink count unchanged.

## Test practicality & rigidity
- Tests run inside existing `mock_tests` target (uses `<cassert>` style); they only rely on contract-defined event structures and the mock sink, so they are deterministic and engine-free.
- Event broadcasts are synchronous in `GameEvents.cpp`, letting tests broadcast and immediately assert captured calls without timers.
- Mock sink exposes explicit counters and last-played IDs, enabling precise assertions for “no sound without event” and “event X → sound Y”.
- Idempotency checks leverage `GameEventManager`’s listener list semantics (no duplicates) and can be observed via mock sink state and listener count introspection within the mock if needed.

## Outstanding gaps (blockers)
- Audio resource inventory now lives in `Classes/Managers/AudioManager/README.md`; mappings should cite those identifiers.
- Audio sink contract is defined (`Classes/Contract/Engine/AudioSink.h`); remaining work is wiring AudioManager to use it.

## Audio sink contract (Stage 5)
- **Interface:** `Classes/Contract/Engine/AudioSink.h`
  - `class IAudioSink { int Play(const std::string& clip_id, bool loop=false); void Stop(int handle); void StopAll(); };`
- **Mock backend:** `Classes/Mocks/Audio/AudioSinkMock.h` (record-only; deterministic handles, exposes play/stop history for assertions).
- **Production adapter:** `Classes/Integration/Engine/CocosAudioSink.{h,cpp}` wrapping `cocos2d::experimental::AudioEngine` when `USE_COCOS_ENGINE=ON`; stubbed no-op when off. AudioManager calls only through `IAudioSink`.
