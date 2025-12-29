#ifndef CLASSES_UI_UISTATEMODELS_H_
#define CLASSES_UI_UISTATEMODELS_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "Classes/Contract/Gameplay/GameEvents.h"

enum class UiMode {
    kMenu,
    kBuild,
    kAttack,
};

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

struct BuildingProgressStatus {
    int entity_id = 0;
    Core::BuildingType building_type = Core::BuildingType::kTownHall;
    Gameplay::BuildingState state = Gameplay::BuildingState::kIdle;
    float time_remaining = 0.0f;
    float total_build_time = 0.0f;
};

struct BattleUiSnapshot {
    bool in_battle = false;
    int time_limit_seconds = 0;
    bool has_battle_end = false;
    Gameplay::BattleEndEvent last_battle_end;
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

// Tracks construction/upgrade progress for building entities.
class BuildingState {
public:
    void OnEntitySpawned(const Gameplay::EntitySpawnEvent& evt) {
        if (!evt.is_building) {
            return;
        }
        BuildingProgressStatus status;
        status.entity_id = evt.instance_id;
        status.building_type = evt.building_type;
        status.state = Gameplay::BuildingState::kIdle;
        status.time_remaining = 0.0f;
        status.total_build_time = 0.0f;
        entries_[evt.instance_id] = status;
    }

    void OnBuildingStateChanged(const Gameplay::BuildingStateEvent& evt) {
        auto it = entries_.find(evt.instance_id);
        if (it == entries_.end()) {
            BuildingProgressStatus status;
            status.entity_id = evt.instance_id;
            status.building_type = evt.type;
            entries_[evt.instance_id] = status;
            it = entries_.find(evt.instance_id);
        }
        it->second.state = evt.new_state;
        it->second.time_remaining = evt.time_remaining;
        it->second.total_build_time = evt.total_build_time;
    }

    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) {
        entries_.erase(evt.instance_id);
    }

    std::vector<BuildingProgressStatus> GetAllEntries() const {
        std::vector<BuildingProgressStatus> results;
        results.reserve(entries_.size());
        for (const auto& pair : entries_) {
            results.push_back(pair.second);
        }
        std::sort(results.begin(), results.end(),
                  [](const BuildingProgressStatus& a, const BuildingProgressStatus& b) {
                      return a.entity_id < b.entity_id;
                  });
        return results;
    }

private:
    std::unordered_map<int, BuildingProgressStatus> entries_;
};

// Battle HUD state tracks battle lifecycle and cached settlement payload.
class BattleState {
public:
    void OnBattleStarted(const Gameplay::BattleStartEvent& evt) {
        in_battle_ = true;
        time_limit_seconds_ = evt.time_limit_seconds;
        has_battle_end_ = false;
    }

    void OnBattleEnded(const Gameplay::BattleEndEvent& evt) {
        in_battle_ = false;
        has_battle_end_ = true;
        last_battle_end_ = evt;
    }

    BattleUiSnapshot GetSnapshot() const {
        BattleUiSnapshot snapshot;
        snapshot.in_battle = in_battle_;
        snapshot.time_limit_seconds = time_limit_seconds_;
        snapshot.has_battle_end = has_battle_end_;
        snapshot.last_battle_end = last_battle_end_;
        return snapshot;
    }

private:
    bool in_battle_ = false;
    int time_limit_seconds_ = 0;
    bool has_battle_end_ = false;
    Gameplay::BattleEndEvent last_battle_end_{};
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
    UiMode mode = UiMode::kMenu;
    HudSnapshot hud;
    SelectionSnapshot selection;
    std::vector<HealthBarStatus> health_bars;
    std::vector<BuildingProgressStatus> building_progress;
    BattleUiSnapshot battle;
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

    void SetMode(UiMode mode) { mode_ = mode; }

    void SetSelectedEntity(int entity_id) { selection_state_.SelectEntity(entity_id); }

    void ClearSelection() { selection_state_.ClearSelection(); }

    UiStateSnapshot GetSnapshot() const {
        UiStateSnapshot snapshot;
        snapshot.mode = mode_;
        snapshot.hud = hud_state_.GetSnapshot();
        snapshot.selection = selection_state_.GetSnapshot();
        snapshot.health_bars = health_bar_state_.GetAllEntries();
        snapshot.building_progress = building_state_.GetAllEntries();
        snapshot.battle = battle_state_.GetSnapshot();
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
        building_state_.OnEntitySpawned(evt);
    }

    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) override {
        selection_state_.OnEntityDestroyed(evt);
        health_bar_state_.OnEntityDestroyed(evt);
        building_state_.OnEntityDestroyed(evt);
    }

    void OnEntityDamaged(const Gameplay::DamageEvent& evt) override {
        health_bar_state_.OnEntityDamaged(evt);
    }

    void OnBuildingStateChanged(const Gameplay::BuildingStateEvent& evt) override {
        building_state_.OnBuildingStateChanged(evt);
    }

    void OnBattleStarted(const Gameplay::BattleStartEvent& evt) override {
        battle_state_.OnBattleStarted(evt);
    }

    void OnBattleEnded(const Gameplay::BattleEndEvent& evt) override {
        battle_state_.OnBattleEnded(evt);
    }

private:
    int local_owner_id_ = 0;
    Gameplay::GameEventManager* manager_ = nullptr;
    UiMode mode_ = UiMode::kMenu;
    HudState hud_state_;
    SelectionState selection_state_;
    HealthBarState health_bar_state_;
    BuildingState building_state_;
    BattleState battle_state_;
    DeploymentBarState deployment_state_;
};

// Stage 4.5 binding: converts UiStateSnapshot into presentation-ready props that
// a UI layer (Cocos or mock) can consume without querying gameplay directly.
struct UiLayoutConstants {
    float hud_padding_px = 8.0f;
    float hud_icon_size_px = 32.0f;
    float hud_font_size_pt = 18.0f;
    float countdown_font_size_pt = 24.0f;
    float progress_bar_height_px = 14.0f;
    float results_panel_padding_px = 12.0f;
};

struct UiAssetCatalog {
    // HUD
    std::string gold_icon = "Resources/UI/Icons/coin.png";
    std::string elixir_icon = "Resources/UI/Icons/elixir.png";

    // Menu
    std::string menu_background = "Resources/UI/Backgrounds/settings.png";
    std::string menu_start_button = "Resources/UI/Buttons/BattleSearchButton.png";

    // Build mode
    std::string build_progress_bar = "Resources/UI/Columns/resources num display column.png";

    // Attack mode
    std::string countdown_icon = "Resources/UI/Icons/timeleft.png";
    std::string deployment_column = "Resources/UI/Columns/Troop Placeholder.png";

    // Results
    std::string victory_background = "Resources/UI/Backgrounds/Real victory.png";
    std::string defeat_background = "Resources/UI/Backgrounds/shop.png";
    std::string star_zero = "Resources/UI/Backgrounds/0 stars.png";
    std::string star_one = "Resources/UI/Backgrounds/1 stars.png";
    std::string star_two = "Resources/UI/Backgrounds/2 stars.png";
    std::string star_three = "Resources/UI/Backgrounds/3 stars.png";
};

struct HudRenderItem {
    std::string icon_asset;
    std::string label_text;
};

struct BuildingOverlayRender {
    int entity_id = 0;
    Core::BuildingType building_type = Core::BuildingType::kTownHall;
    float progress_ratio = 0.0f;  // 0..1; 0 when idle or unknown
    Gameplay::BuildingState state = Gameplay::BuildingState::kIdle;
    std::string progress_bar_asset;
};

struct BattlePanelRender {
    bool show_countdown = false;
    int countdown_seconds = 0;
    bool show_results = false;
    Gameplay::BattleResult result = Gameplay::BattleResult::kDefeat;
    int stars_earned = 0;
    std::string background_asset;
    std::string star_strip_asset;
};

struct MenuPanelRender {
    std::string background_asset;
    std::string start_button_asset;
};

struct UiRenderPlan {
    UiMode mode = UiMode::kMenu;
    UiAssetCatalog assets;
    UiLayoutConstants layout;
    std::vector<HudRenderItem> hud_items;
    std::vector<BuildingOverlayRender> building_overlays;
    BattlePanelRender battle_panel;
    MenuPanelRender menu_panel;
};

class UiPresentationBinding {
public:
    UiRenderPlan BuildRenderPlan(const UiStateSnapshot& snapshot) const {
        UiRenderPlan plan;
        plan.mode = snapshot.mode;
        plan.assets = assets_;
        plan.layout = layout_;

        AppendHud(snapshot.hud, &plan);
        AppendBuildingOverlays(snapshot.building_progress, &plan);
        AppendBattle(snapshot.battle, &plan);
        AppendMenu(&plan);
        return plan;
    }

private:
    void AppendHud(const HudSnapshot& hud, UiRenderPlan* plan) const {
        plan->hud_items.push_back({assets_.gold_icon, FormatResource(hud.gold, hud.gold_capacity)});
        plan->hud_items.push_back({assets_.elixir_icon, FormatResource(hud.elixir, hud.elixir_capacity)});
    }

    void AppendBuildingOverlays(const std::vector<BuildingProgressStatus>& progress,
                                UiRenderPlan* plan) const {
        for (const BuildingProgressStatus& status : progress) {
            BuildingOverlayRender render;
            render.entity_id = status.entity_id;
            render.building_type = status.building_type;
            render.state = status.state;
            render.progress_bar_asset = assets_.build_progress_bar;
            if (status.total_build_time > 0.0f && status.time_remaining >= 0.0f) {
                float spent = status.total_build_time - status.time_remaining;
                render.progress_ratio = std::max(0.0f, std::min(1.0f, spent / status.total_build_time));
            }
            plan->building_overlays.push_back(render);
        }
        std::sort(plan->building_overlays.begin(), plan->building_overlays.end(),
                  [](const BuildingOverlayRender& a, const BuildingOverlayRender& b) {
                      return a.entity_id < b.entity_id;
                  });
    }

    void AppendBattle(const BattleUiSnapshot& battle, UiRenderPlan* plan) const {
        plan->battle_panel.show_countdown = battle.in_battle;
        plan->battle_panel.countdown_seconds = battle.time_limit_seconds;
        if (battle.has_battle_end) {
            plan->battle_panel.show_results = true;
            plan->battle_panel.result = battle.last_battle_end.result;
            plan->battle_panel.stars_earned = battle.last_battle_end.stars_earned;
            plan->battle_panel.background_asset = battle.last_battle_end.result == Gameplay::BattleResult::kVictory
                                                      ? assets_.victory_background
                                                      : assets_.defeat_background;
            plan->battle_panel.star_strip_asset = SelectStarStrip(battle.last_battle_end.stars_earned);
        }
    }

    void AppendMenu(UiRenderPlan* plan) const {
        plan->menu_panel.background_asset = assets_.menu_background;
        plan->menu_panel.start_button_asset = assets_.menu_start_button;
    }

    std::string FormatResource(int current, int capacity) const {
        std::ostringstream ss;
        ss << current << "/" << capacity;
        return ss.str();
    }

    std::string SelectStarStrip(int stars) const {
        switch (stars) {
            case 0:
                return assets_.star_zero;
            case 1:
                return assets_.star_one;
            case 2:
                return assets_.star_two;
            default:
                return assets_.star_three;
        }
    }

    UiAssetCatalog assets_{};
    UiLayoutConstants layout_{};
};

#endif  // CLASSES_UI_UISTATEMODELS_H_
