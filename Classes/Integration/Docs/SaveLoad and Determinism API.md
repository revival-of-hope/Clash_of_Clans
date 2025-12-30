# Save/Load and Determinism API (Dev C)

This document defines the canonical API for save/load snapshots and determinism inspection. It is **opaque** and does not expose Gameplay rules.

## 1. SaveLoadService

**Header:** `Classes/Contract/Integration/SaveLoadService.h`

### 1.1 Interface

```cpp
class SaveLoadService {
public:
    static SaveLoadService* GetInstance();

    std::string SaveSnapshot() const;
    bool LoadSnapshot(const std::string& blob);

    uint64_t GetDeterminismTick() const;
    uint64_t GetStateHash() const;
};
```

### 1.2 Semantics

* **Snapshot format is opaque.** No JSON requirement is imposed.
* `SaveSnapshot()` returns a serialized snapshot blob representing current game state.
* `LoadSnapshot(blob)` restores a snapshot:
  * Returns `false` if `blob` is empty.
  * Returns `true` otherwise.
* `GetDeterminismTick()` returns the current simulation tick counter.
* `GetStateHash()` returns a stable hash of the current simulation state.

### 1.3 Usage Example

```cpp
auto save_load = SaveLoadService::GetInstance();
auto blob = save_load->SaveSnapshot();

if (!blob.empty()) {
    save_load->LoadSnapshot(blob);
}

auto tick = save_load->GetDeterminismTick();
auto hash = save_load->GetStateHash();
```

