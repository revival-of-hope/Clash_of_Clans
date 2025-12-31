# AudioManager Sound Catalog

Authoritative list of audio identifiers and their intended usage for AudioManager mappings. Audio files are stored under `Resources/music`. Paths below are relative to the repository root and should be treated as stable identifiers for playback.

## Background Music
- `Resources/music/Background Music/Home music 1.mp3` — Home screen background loop.
- `Resources/music/Background Music/Home music 2.mp3` — Alternate home screen background loop.
- `Resources/music/Background Music/shorter part1.mp3` — Shorter background intro.
- `Resources/music/Background Music/shorter part2.mp3` — Battle background loop (shorter background loop/continuation).

## Building Effects
- `Resources/music/Building effects/bad-placement.mp3` — Invalid placement warning.
- `Resources/music/Building effects/cash-register.mp3` — Resource collection acknowledgment.
- `Resources/music/Building effects/collecting-gems.mp3` — Gem collection.
- `Resources/music/Building effects/placement.mp3` — Successful placement confirmation.
- `Resources/music/Building effects/upgrade-sound.mp3` — Upgrade initiated/completed.
- `Resources/music/Building effects/upgrade.mp3` — Upgrade in progress.

## Combat Effects
- `Resources/music/Combat effects/archer tower.mp3` — Archer tower firing.
- `Resources/music/Combat effects/arrow-hit.mp3` — Projectile impact.
- `Resources/music/Combat effects/barbarian-death-cry.mp3` — Barbarian death.
- `Resources/music/Combat effects/building destroyed.mp3` — Structure destroyed.
- `Resources/music/Combat effects/cannon fire.mp3` — Cannon firing.

## UI Effects
- `Resources/music/UI effects/builder-base-combat-end.mp3` — Combat finished.
- `Resources/music/UI effects/start_up.mp3` — App start/login.
- `Resources/music/UI effects/ui_click.mp3` — UI interaction click.
- `Resources/music/UI effects/victory.mp3` — Victory fanfare.

## Integrating with real Cocos2d-x AudioEngine (when available)

- **Build flags:** Enable `USE_COCOS_ENGINE=ON` to compile the Cocos-backed sink. Keep `BUILD_TESTS=ON` with `USE_COCOS_ENGINE=OFF` for CI/mock runs.
- **Cocos-dependent code location:** All Cocos includes and calls are isolated to `Classes/Integration/Engine/CocosAudioSink.{h,cpp}` and guarded by `USE_COCOS_ENGINE`. `AudioManager` and `IAudioSink` must not include Cocos headers.
- **Wiring (single switch):** The composition root chooses the sink implementation. Mock builds/tests instantiate `AudioSinkMock` (record-only). Real builds instantiate `CocosAudioSink` and pass it to `AudioManager` while leaving the manager logic unchanged.
- **Dependencies when Cocos is present:** `CocosAudioSink.cpp` expects the standard Cocos2d-x AudioEngine header (e.g., `<audio/include/AudioEngine.h>`) and links against the corresponding Cocos audio libraries per your platform toolchain/CMake setup.
- **Runtime identifiers:** `AudioManager` plays string IDs exactly as listed above (relative paths under `Resources/music`). Assets are not required in git but must be packaged so the runtime working directory can resolve these paths.
- **Integration checklist:**
  1. Enable the Cocos build flag (`-DUSE_COCOS_ENGINE=ON`).
  2. Ensure `CocosAudioSink` compiles and links against the Cocos AudioEngine library/headers.
  3. Perform a manual runtime smoke check to confirm a clip plays through the adapter.
  4. Keep mock tests running with `USE_COCOS_ENGINE=OFF` to verify event-to-sound mappings remain deterministic.


### Audio Assets

clash_of_clans_local/Resources/music
├── Background Music
│   ├── Home music 1.mp3
│   ├── Home music 2.mp3
│   ├── shorter part1.mp3
│   └── shorter part2.mp3
├── Building effects
│   ├── bad-placement.mp3
│   ├── cash-register.mp3
│   ├── collecting-gems.mp3
│   ├── placement.mp3
│   ├── upgrade-sound.mp3
│   └── upgrade.mp3
├── Combat effects
│   ├── archer tower.mp3
│   ├── arrow-hit.mp3
│   ├── barbarian-death-cry.mp3
│   ├── building destroyed.mp3
│   └── cannon fire.mp3
└── UI effects
    ├── builder-base-combat-end.mp3
    ├── start_up.mp3
    ├── ui_click.mp3
    └── victory.mp3
