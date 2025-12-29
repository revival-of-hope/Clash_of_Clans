# Contract Gaps

This file tracks missing, ambiguous, or conflicting API/behavior Contract found in the current SOT docs.

| ID | Scope | Description | Source Docs | Resolution Need |
| --- | --- | --- | --- | --- |
| G1 | SceneFlowService | Ownership/lifetime of returned `cocos2d::Scene*` is unspecified; current implementation returns freshly created scenes per call. | `Classes/Integration/Docs/Scene Flow and Entrypoint.md` (no ownership guidance) | Confirm who owns/destroys scenes and any Director transition hooks required. |
| G2 | Gameplay → UI events | RESOLVED in contract: added `TroopCountUpdateEvent` + `DeploymentSelectionEvent` via `GameEvents.h` and broadcasts so deployment bar can track counts/selection without guessing. | `Classes/Integration/Docs/Gameplay_Public_GameEvents_Documentation.md`, `Classes/Contract/Gameplay/GameEvents.h` | Keep docs in sync when gameplay docs are next revised. |
| G3 | Audio resources | RESOLVED: Audio catalog documented at `Classes/Managers/AudioManager/README.md` with identifiers for background, building, combat, and UI sounds. | `Classes/Managers/AudioManager/README.md` | Use documented paths as mapping keys. |
| G4 | Audio sink contract | RESOLVED: Added engine-agnostic `IAudioSink` under `Classes/Contract/Engine/AudioSink.h` with mock and cocos adapter hooks. | `Classes/Contract/Engine/AudioSink.h`; `Classes/Mocks/Audio/AudioSinkMock.h`; `Classes/Integration/Engine/CocosAudioSink.{h,cpp}` | Wire AudioManager through the contract. |
| G5 | Gameplay contract duplication | RESOLVED: Promoted the real gameplay headers into `Classes/Contract/Gameplay` and archived the simplified legacy headers under `Classes/Mocks/LegacyContracts/Gameplay` to remove dual contract surfaces. | `Classes/Contract/Gameplay/*.h`; `Classes/Mocks/LegacyContracts/Gameplay/*.h` | Keep canonical includes pointed at `Classes/Contract/Gameplay/...`; legacy headers remain only for reference. |
