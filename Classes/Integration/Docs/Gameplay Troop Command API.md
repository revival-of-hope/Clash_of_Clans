# Gameplay Troop Command API (Dev C)

This document defines the canonical UI → Gameplay command surface for troop training and deployment.
The API is command-only; it does not expose Gameplay state beyond success/failure.

## 1. TroopCommandService

**Header:** `Classes/Contract/Gameplay/TroopCommandService.h`

### 1.1 TroopTrainRequest

```cpp
struct TroopTrainRequest {
    Core::TroopType troop_type;
    int level = 1;
    int count = 1;
    int owner_id = 0;
};
```

### 1.2 TroopDeployRequest

```cpp
struct TroopDeployRequest {
    Core::TroopType troop_type;
    int level = 1;
    int owner_id = 0;
    float world_x = 0.0f;
    float world_y = 0.0f;
};
```

### 1.3 Interface

```cpp
class TroopCommandService {
public:
    static TroopCommandService* GetInstance();

    bool RequestTrainTroop(const TroopTrainRequest& request);
    bool RequestDeployTroop(const TroopDeployRequest& request);
};
```

**Semantics:**
* `RequestTrainTroop(...)` enqueues a training request for the specified troop type/level and count.
* `RequestDeployTroop(...)` requests deployment at a world position (world coordinates are UI-derived).
* Both calls return `true` if accepted for processing; `false` if rejected (e.g., insufficient resources).
* Commands are **not** guaranteed to complete; resulting changes must be observed via `GameEvents.h`.

