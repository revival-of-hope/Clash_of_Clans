#ifndef CLASSES_UI_UISTATEMODELS_H_
#define CLASSES_UI_UISTATEMODELS_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "Classes/Contract/Gameplay/GameEvents.h"

struct HudSnapshot {
    int gold = 0;
    int gold_capacity = 0;
    int elixir = 0;
    int elixir_capacity = 0;
};

// HUD state model driven by Gameplay::ResourceUpdateEvent signals.
class HudState {
public:
    void ApplyResourceUpdate(const Gameplay::ResourceUpdateEvent& evt) {
        if (evt.resource_type == "Gold") {
            gold_ = evt.current_amount;
            gold_capacity_ = evt.max_capacity;
        } else if (evt.resource_type == "Elixir") {
            elixir_ = evt.current_amount;
            elixir_capacity_ = evt.max_capacity;
        }
    }

    HudSnapshot GetSnapshot() const {
        HudSnapshot snapshot;
        snapshot.gold = gold_;
        snapshot.gold_capacity = gold_capacity_;
        snapshot.elixir = elixir_;
        snapshot.elixir_capacity = elixir_capacity_;
        return snapshot;
    }

private:
    int gold_ = 0;
    int gold_capacity_ = 0;
    int elixir_ = 0;
    int elixir_capacity_ = 0;
};

struct SelectionSnapshot {
    bool has_selection = false;
    int selected_entity_id = -1;
};

// Selection highlight state tracks the selected entity id and clears on destruction.
class SelectionState {
public:
    void SelectEntity(int entity_id) { selected_entity_id_ = entity_id; }

    void ClearSelection() { selected_entity_id_ = kNoSelection; }

    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) {
        if (selected_entity_id_ == evt.instance_id) {
            ClearSelection();
        }
    }

    SelectionSnapshot GetSnapshot() const {
        SelectionSnapshot snapshot;
        snapshot.has_selection = selected_entity_id_ != kNoSelection;
        snapshot.selected_entity_id = selected_entity_id_;
        return snapshot;
    }

private:
    static constexpr int kNoSelection = -1;
    int selected_entity_id_ = kNoSelection;
};

struct HealthBarStatus {
    int entity_id = 0;
    int current_hp = 0;
    int max_hp = 0;
    bool is_ally = false;
};

// Health bar tracker builds deterministic entries from spawn/damage/destroy events.
class HealthBarState {
public:
    void OnEntitySpawned(const Gameplay::EntitySpawnEvent& evt, bool is_ally) {
        HealthBarStatus status;
        status.entity_id = evt.instance_id;
        status.current_hp = evt.current_hp;
        status.max_hp = evt.max_hp;
        status.is_ally = is_ally;
        entries_[evt.instance_id] = status;
    }

    void OnEntityDamaged(const Gameplay::DamageEvent& evt) {
        auto it = entries_.find(evt.target_instance_id);
        if (it == entries_.end()) {
            return;
        }
        it->second.current_hp = evt.current_hp;
        it->second.max_hp = evt.max_hp;
    }

    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) {
        entries_.erase(evt.instance_id);
    }

    const HealthBarStatus* GetEntry(int entity_id) const {
        auto it = entries_.find(entity_id);
        if (it == entries_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    std::vector<HealthBarStatus> GetAllEntries() const {
        std::vector<HealthBarStatus> results;
        results.reserve(entries_.size());
        for (const auto& pair : entries_) {
            results.push_back(pair.second);
        }
        std::sort(results.begin(), results.end(),
                  [](const HealthBarStatus& a, const HealthBarStatus& b) {
                      return a.entity_id < b.entity_id;
                  });
        return results;
    }

private:
    std::unordered_map<int, HealthBarStatus> entries_;
};

struct TroopCountState {
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    int remaining_count = 0;
};

struct DeploymentOwnerSnapshot {
    int owner_id = 0;
    bool has_selection = false;
    Core::TroopType selected_troop = Core::TroopType::kBarbarian;
    std::vector<TroopCountState> troop_counts;
};

class DeploymentBarState {
public:
    void OnTroopCountUpdated(const Gameplay::TroopCountUpdateEvent& evt) {
        counts_[evt.owner_id][evt.troop_type] = evt.remaining_count;
    }

    void OnSelectionChanged(const Gameplay::DeploymentSelectionEvent& evt) {
        DeploymentSelection selection;
        selection.has_selection = evt.has_selection;
        selection.selected_troop = evt.troop_type;
        selection_by_owner_[evt.owner_id] = selection;
    }

    DeploymentOwnerSnapshot GetOwnerSnapshot(int owner_id) const {
        DeploymentOwnerSnapshot snapshot;
        snapshot.owner_id = owner_id;

        auto selection_it = selection_by_owner_.find(owner_id);
        if (selection_it != selection_by_owner_.end()) {
            snapshot.has_selection = selection_it->second.has_selection;
            snapshot.selected_troop = selection_it->second.selected_troop;
        }

        auto counts_it = counts_.find(owner_id);
        if (counts_it != counts_.end()) {
            for (const auto& pair : counts_it->second) {
                TroopCountState state;
                state.troop_type = pair.first;
                state.remaining_count = pair.second;
                snapshot.troop_counts.push_back(state);
            }
            std::sort(snapshot.troop_counts.begin(), snapshot.troop_counts.end(),
                      [](const TroopCountState& a, const TroopCountState& b) {
                          return static_cast<int>(a.troop_type) < static_cast<int>(b.troop_type);
                      });
        }

        return snapshot;
    }

    std::vector<DeploymentOwnerSnapshot> GetAllOwnerSnapshots() const {
        std::vector<DeploymentOwnerSnapshot> snapshots;
        std::vector<int> owner_ids;
        owner_ids.reserve(counts_.size());
        for (const auto& pair : counts_) {
            owner_ids.push_back(pair.first);
        }
        for (const auto& pair : selection_by_owner_) {
            if (std::find(owner_ids.begin(), owner_ids.end(), pair.first) == owner_ids.end()) {
                owner_ids.push_back(pair.first);
            }
        }
        std::sort(owner_ids.begin(), owner_ids.end());

        for (int owner_id : owner_ids) {
            snapshots.push_back(GetOwnerSnapshot(owner_id));
        }
        return snapshots;
    }

private:
    struct DeploymentSelection {
        bool has_selection = false;
        Core::TroopType selected_troop = Core::TroopType::kBarbarian;
    };

    std::unordered_map<int, std::unordered_map<Core::TroopType, int>> counts_;
    std::unordered_map<int, DeploymentSelection> selection_by_owner_;
};

struct UiStateSnapshot {
    HudSnapshot hud;
    SelectionSnapshot selection;
    std::vector<HealthBarStatus> health_bars;
    std::vector<DeploymentOwnerSnapshot> deployment;
};

class UiStateStore : public Gameplay::IGameEventListener {
public:
    explicit UiStateStore(int local_owner_id) : local_owner_id_(local_owner_id) {}

    void Attach(Gameplay::GameEventManager* manager) {
        if (manager_ == manager) {
            return;
        }
        manager_ = manager;
        if (manager_ != nullptr) {
            manager_->AddListener(this);
        }
    }

    void Detach(Gameplay::GameEventManager* manager) {
        if (manager_ == nullptr || manager_ != manager) {
            return;
        }
        manager_->RemoveListener(this);
        manager_ = nullptr;
    }

    void SetSelectedEntity(int entity_id) { selection_state_.SelectEntity(entity_id); }

    void ClearSelection() { selection_state_.ClearSelection(); }

    UiStateSnapshot GetSnapshot() const {
        UiStateSnapshot snapshot;
        snapshot.hud = hud_state_.GetSnapshot();
        snapshot.selection = selection_state_.GetSnapshot();
        snapshot.health_bars = health_bar_state_.GetAllEntries();
        snapshot.deployment = deployment_state_.GetAllOwnerSnapshots();
        return snapshot;
    }

    void OnResourceChanged(const Gameplay::ResourceUpdateEvent& evt) override {
        hud_state_.ApplyResourceUpdate(evt);
    }

    void OnTroopCountUpdated(const Gameplay::TroopCountUpdateEvent& evt) override {
        deployment_state_.OnTroopCountUpdated(evt);
    }

    void OnDeploymentSelectionChanged(const Gameplay::DeploymentSelectionEvent& evt) override {
        deployment_state_.OnSelectionChanged(evt);
    }

    void OnEntitySpawned(const Gameplay::EntitySpawnEvent& evt) override {
        bool is_ally = evt.owner_id == local_owner_id_;
        health_bar_state_.OnEntitySpawned(evt, is_ally);
    }

    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) override {
        selection_state_.OnEntityDestroyed(evt);
        health_bar_state_.OnEntityDestroyed(evt);
    }

    void OnEntityDamaged(const Gameplay::DamageEvent& evt) override {
        health_bar_state_.OnEntityDamaged(evt);
    }

private:
    int local_owner_id_ = 0;
    Gameplay::GameEventManager* manager_ = nullptr;
    HudState hud_state_;
    SelectionState selection_state_;
    HealthBarState health_bar_state_;
    DeploymentBarState deployment_state_;
};

#endif  // CLASSES_UI_UISTATEMODELS_H_
