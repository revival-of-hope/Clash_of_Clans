# Contracts (Canonical Public API)

## Canonical API Statement
`Classes/Contracts/**` is the **only supported public API** for Dev C and integration code. All public interfaces must live here and be consumed from here.

### Stable include examples
Dev C must include Contracts via the canonical include root:

```cpp
#include "Classes/Contracts/Engine/MapLayer.h"
#include "Classes/Contracts/Engine/TilePlacementController.h"
#include "Classes/Contracts/Gameplay/EconomySystem.h"
#include "Classes/Contracts/Gameplay/Unit.h"
#include "Classes/Contracts/Gameplay/HealthComp.h"
```

## Migration Note (Legacy Public Headers)
The legacy public headers under `Classes/Engine/Public/**` and `Classes/Gameplay/Public/**` are **temporary**.  
Contracts may currently mirror/duplicate those headers to keep the build unblocked.  
**Final integration removes the legacy folders** and keeps `Classes/Contracts/**` as the sole public API.

## Traceability Requirement
Every contract header should include a brief comment at the top pointing to the authoritative doc source, e.g.:

```cpp
// Source: Classes/Integration/Docs/Gameplay Module API Reference.md.md (EconomySystem)
```

This keeps Contracts aligned with the documentation SOT.

## CONTRACT_GAPS Workflow
If the doc source is ambiguous, missing, or conflicting:
1. **Stop** and record the gap in `Classes/Contracts/CONTRACT_GAPS.md`.
2. Include:
   - Doc file path + heading
   - What’s missing/conflicting
   - Exact question(s) to resolve

## Cocos Type Policy (C1)
Contracts may reference cocos2d types **only** when required by the docs:
* Allowed as pointers/references.
* Prefer forward declarations where possible:
  `namespace cocos2d { class Sprite; class Scene; class TMXTiledMap; struct Vec2; }`
* If a doc requires a cocos2d type **by value** and it cannot be safely forward-declared,
  record a gap in `CONTRACT_GAPS.md` rather than guessing.
