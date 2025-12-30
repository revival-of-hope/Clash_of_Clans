#ifndef CLASSES_UI_UISTATEMODELS_H_
#define CLASSES_UI_UISTATEMODELS_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "Classes/Contract/Gameplay/CostQuery.h"
#include "Classes/Contract/Gameplay/GameEvents.h"
#include "Classes/Contract/Integration/PlayerIdentity.h"
#include "Core/GameConfig.h"

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
    int gems = 0;
    int gems_capacity = 0;
    int trophies = 0;
    int trophies_capacity = 0;
};

struct PlayerIdentitySnapshot {
    int player_id = 0;
    std::string name;
    std::string icon_id;
};

struct LootAvailabilitySnapshot {
    int gold_available = 0;
    int elixir_available = 0;
    int trophies_available = 0;
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
        } else if (evt.resource_type == "Gems") {
            gems_ = evt.current_amount;
            gems_capacity_ = evt.max_capacity;
        } else if (evt.resource_type == "Trophies") {
            trophies_ = evt.current_amount;
            trophies_capacity_ = evt.max_capacity;
        }
    }

    HudSnapshot GetSnapshot() const {
        HudSnapshot snapshot;
        snapshot.gold = gold_;
        snapshot.gold_capacity = gold_capacity_;
        snapshot.elixir = elixir_;
        snapshot.elixir_capacity = elixir_capacity_;
        snapshot.gems = gems_;
        snapshot.gems_capacity = gems_capacity_;
        snapshot.trophies = trophies_;
        snapshot.trophies_capacity = trophies_capacity_;
        return snapshot;
    }

private:
    int gold_ = 0;
    int gold_capacity_ = 0;
    int elixir_ = 0;
    int elixir_capacity_ = 0;
    int gems_ = 0;
    int gems_capacity_ = 0;
    int trophies_ = 0;
    int trophies_capacity_ = 0;
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

struct HealthBarRender {
    int entity_id = 0;
    float health_ratio = 0.0f;
    bool is_ally = false;
};

struct BuildingProgressStatus {
    int entity_id = 0;
    Core::BuildingType building_type = Core::BuildingType::kTownHall;
    Gameplay::BuildingState state = Gameplay::BuildingState::kIdle;
    float time_remaining = 0.0f;
    float total_build_time = 0.0f;
};

struct BuildPaletteEntry {
    Core::BuildingType type = Core::BuildingType::kTownHall;
    int level = 1;
    ResourceCost cost{};
    float build_time_seconds = 0.0f;
};

struct BuildPaletteSnapshot {
    std::vector<BuildPaletteEntry> entries;
    Core::BuildingType selected_type = Core::BuildingType::kTownHall;
    int selected_level = 1;
    ResourceCost selected_cost{};
    float selected_time_seconds = 0.0f;
};

struct SelectedBuildingSnapshot {
    bool has_selection = false;
    int entity_id = 0;
    Core::BuildingType type = Core::BuildingType::kTownHall;
    int level = 1;
    Core::BuildingStats stats{};
};

struct TroopInspectSnapshot {
    bool has_selection = false;
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    int level = 1;
    Core::UnitStats stats{};
};

struct MatchmakingSnapshot {
    ResourceCost cost{};
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

class LootAvailabilityState {
public:
    void OnLootAvailabilityUpdated(const Gameplay::LootAvailabilityEvent& evt) {
        snapshot_.gold_available = evt.gold_available;
        snapshot_.elixir_available = evt.elixir_available;
        snapshot_.trophies_available = evt.trophies_available;
    }

    LootAvailabilitySnapshot GetSnapshot() const { return snapshot_; }

private:
    LootAvailabilitySnapshot snapshot_{};
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
    PlayerIdentitySnapshot identity;
    HudSnapshot hud;
    LootAvailabilitySnapshot loot_availability;
    SelectionSnapshot selection;
    std::vector<HealthBarStatus> health_bars;
    std::vector<BuildingProgressStatus> building_progress;
    BuildPaletteSnapshot build_palette;
    SelectedBuildingSnapshot selected_building;
    BattleUiSnapshot battle;
    std::vector<DeploymentOwnerSnapshot> deployment;
    TroopInspectSnapshot troop_inspect;
    MatchmakingSnapshot matchmaking;
};

class UiStateStore : public Gameplay::IGameEventListener {
public:
    explicit UiStateStore(int local_owner_id) : local_owner_id_(local_owner_id) {
        CostQuery* cost_query = CostQuery::GetInstance();
        if (cost_query) {
            matchmaking_.cost = cost_query->GetMatchmakingCost();
        }
        InitializeBuildPaletteDefaults();
    }

    void SetLocalOwnerId(int owner_id) { local_owner_id_ = owner_id; }

    void SetPlayerIdentity(const Integration::PlayerIdentity& identity) {
        identity_.player_id = identity.player_id;
        identity_.name = identity.name;
        identity_.icon_id = identity.icon_id;
    }

    void InitializeBuildPalette(const std::vector<Core::BuildingType>& types, int level = 1) {
        BuildPaletteSnapshot snapshot;
        snapshot.selected_level = level;
        snapshot.selected_type = types.empty() ? Core::BuildingType::kTownHall : types.front();
        CostQuery* cost_query = CostQuery::GetInstance();
        for (Core::BuildingType type : types) {
            BuildPaletteEntry entry;
            entry.type = type;
            entry.level = level;
            if (cost_query) {
                entry.cost = cost_query->GetBuildingPlacementCost(type, level);
                entry.build_time_seconds = cost_query->GetBuildingConstructionTime(type, level);
            }
            snapshot.entries.push_back(entry);
        }
        UpdateBuildSelection(snapshot.selected_type, snapshot.selected_level, &snapshot);
        build_palette_ = snapshot;
    }

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

    void SetSelectedEntity(int entity_id) {
        selection_state_.SelectEntity(entity_id);
        UpdateSelectedBuilding(entity_id);
    }

    void ClearSelection() { selection_state_.ClearSelection(); }

    void SetBuildSelection(Core::BuildingType type, int level) {
        UpdateBuildSelection(type, level, &build_palette_);
    }

    UiStateSnapshot GetSnapshot() const {
        UiStateSnapshot snapshot;
        snapshot.mode = mode_;
        snapshot.identity = identity_;
        snapshot.hud = hud_state_.GetSnapshot();
        snapshot.loot_availability = loot_availability_state_.GetSnapshot();
        snapshot.selection = selection_state_.GetSnapshot();
        snapshot.health_bars = health_bar_state_.GetAllEntries();
        snapshot.building_progress = building_state_.GetAllEntries();
        snapshot.build_palette = build_palette_;
        snapshot.selected_building = selected_building_;
        snapshot.battle = battle_state_.GetSnapshot();
        snapshot.deployment = deployment_state_.GetAllOwnerSnapshots();
        snapshot.troop_inspect = troop_inspect_;
        snapshot.matchmaking = matchmaking_;
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
        if (evt.owner_id == local_owner_id_) {
            if (evt.has_selection) {
                troop_inspect_.has_selection = true;
                troop_inspect_.troop_type = evt.troop_type;
                troop_inspect_.level = 1;
                Core::GameConfig* config = Core::GameConfig::GetInstance();
                if (config) {
                    troop_inspect_.stats = config->GetTroopStats(evt.troop_type, troop_inspect_.level);
                }
            } else {
                troop_inspect_.has_selection = false;
            }
        }
    }

    void OnEntitySpawned(const Gameplay::EntitySpawnEvent& evt) override {
        bool is_ally = evt.owner_id == local_owner_id_;
        health_bar_state_.OnEntitySpawned(evt, is_ally);
        building_state_.OnEntitySpawned(evt);
        if (evt.is_building) {
            BuildingInfo info;
            info.type = evt.building_type;
            info.level = evt.level;
            entity_buildings_[evt.instance_id] = info;
        }
    }

    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) override {
        selection_state_.OnEntityDestroyed(evt);
        health_bar_state_.OnEntityDestroyed(evt);
        building_state_.OnEntityDestroyed(evt);
        entity_buildings_.erase(evt.instance_id);
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

    void OnLootAvailabilityUpdated(const Gameplay::LootAvailabilityEvent& evt) override {
        loot_availability_state_.OnLootAvailabilityUpdated(evt);
    }

private:
    struct BuildingInfo {
        Core::BuildingType type = Core::BuildingType::kTownHall;
        int level = 1;
    };

    void UpdateBuildSelection(Core::BuildingType type, int level, BuildPaletteSnapshot* snapshot) {
        if (!snapshot) {
            return;
        }
        snapshot->selected_type = type;
        snapshot->selected_level = level;
        CostQuery* cost_query = CostQuery::GetInstance();
        if (cost_query) {
            snapshot->selected_cost = cost_query->GetBuildingPlacementCost(type, level);
            snapshot->selected_time_seconds = cost_query->GetBuildingConstructionTime(type, level);
        }
    }

    void UpdateSelectedBuilding(int entity_id) {
        auto it = entity_buildings_.find(entity_id);
        if (it == entity_buildings_.end()) {
            selected_building_.has_selection = false;
            return;
        }
        selected_building_.has_selection = true;
        selected_building_.entity_id = entity_id;
        selected_building_.type = it->second.type;
        selected_building_.level = it->second.level;
        Core::GameConfig* config = Core::GameConfig::GetInstance();
        if (config) {
            selected_building_.stats = config->GetBuildingStats(it->second.type, it->second.level);
        }
    }

    void InitializeBuildPaletteDefaults() {
        std::vector<Core::BuildingType> types = {
            Core::BuildingType::kTownHall,
            Core::BuildingType::kGoldMine,
            Core::BuildingType::kElixirCollector,
            Core::BuildingType::kGoldStorage,
            Core::BuildingType::kElixirStorage,
            Core::BuildingType::kBarracks,
            Core::BuildingType::kCannon,
            Core::BuildingType::kArcherTower,
            Core::BuildingType::kAirDefense,
            Core::BuildingType::kWall,
            Core::BuildingType::kArmyCamp,
        };
        InitializeBuildPalette(types, 1);
    }

    int local_owner_id_ = 0;
    Gameplay::GameEventManager* manager_ = nullptr;
    UiMode mode_ = UiMode::kMenu;
    PlayerIdentitySnapshot identity_{};
    HudState hud_state_;
    SelectionState selection_state_;
    HealthBarState health_bar_state_;
    BuildingState building_state_;
    BuildPaletteSnapshot build_palette_{};
    SelectedBuildingSnapshot selected_building_{};
    BattleState battle_state_;
    LootAvailabilityState loot_availability_state_;
    DeploymentBarState deployment_state_;
    TroopInspectSnapshot troop_inspect_{};
    MatchmakingSnapshot matchmaking_{};
    std::unordered_map<int, BuildingInfo> entity_buildings_;
};

// Stage 4.5 binding: converts UiStateSnapshot into presentation-ready props that
// a UI layer (Cocos or mock) can consume without querying Gameplay directly.
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
    std::string gem_icon = "Resources/UI/Icons/gem.png";

    // Menu
    std::string menu_background = "Resources/UI/Backgrounds/settings.png";
    std::string menu_start_button = "Resources/UI/Buttons/BattleSearchButton.png";

    // Build mode
    std::string build_progress_bar = "Resources/UI/Columns/resources num display column.png";
    std::string build_button = "Resources/UI/Buttons/UpgradeButton.png";
    std::string collect_button = "Resources/UI/Buttons/StoreButton.png";

    // Attack mode
    std::string countdown_icon = "Resources/UI/Icons/timeleft.png";
    std::string deployment_column = "Resources/UI/Columns/Troop Placeholder.png";
    std::string matchmaking_button = "Resources/UI/Buttons/BattleSearchButton.png";

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

struct LootRenderItem {
    std::string label_text;
};

struct BuildingOverlayRender {
    int entity_id = 0;
    Core::BuildingType building_type = Core::BuildingType::kTownHall;
    float progress_ratio = 0.0f;  // 0..1; 0 when idle or unknown
    Gameplay::BuildingState state = Gameplay::BuildingState::kIdle;
    std::string progress_bar_asset;
};

struct BuildPaletteRenderItem {
    Core::BuildingType type = Core::BuildingType::kTownHall;
    bool is_selected = false;
    std::string label_text;
    std::string cost_text;
    std::string time_text;
    std::string button_asset;
};

struct SelectedBuildingRender {
    bool show = false;
    std::string title;
    std::vector<std::string> stat_lines;
    std::string collect_button_asset;
};

struct TroopInspectRender {
    bool show = false;
    std::string title;
    std::vector<std::string> stat_lines;
};

struct DeploymentRenderItem {
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    int remaining_count = 0;
    bool is_selected = false;
    std::string icon_asset;
};

struct MatchmakingRender {
    std::string button_asset;
    std::string cost_text;
};

struct PlayerIdentityRender {
    int player_id = 0;
    std::string name;
    std::string icon_asset;
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
    PlayerIdentityRender player_identity;
    int trophies = 0;
    std::vector<HudRenderItem> hud_items;
    std::vector<LootRenderItem> loot_items;
    std::vector<HealthBarRender> health_bars;
    std::vector<BuildingOverlayRender> building_overlays;
    std::vector<BuildPaletteRenderItem> build_palette_items;
    SelectedBuildingRender selected_building;
    std::vector<DeploymentRenderItem> deployment_items;
    TroopInspectRender troop_inspect;
    MatchmakingRender matchmaking;
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
        plan.trophies = snapshot.hud.trophies;

        AppendHud(snapshot.hud, &plan);
        AppendLoot(snapshot.loot_availability, &plan);
        AppendHealthBars(snapshot.health_bars, &plan);
        AppendBuildingOverlays(snapshot.building_progress, &plan);
        AppendBuildPalette(snapshot.build_palette, &plan);
        AppendSelectedBuilding(snapshot.selected_building, &plan);
        AppendDeployment(snapshot.deployment, snapshot.identity.player_id, &plan);
        AppendTroopInspect(snapshot.troop_inspect, &plan);
        AppendMatchmaking(snapshot.matchmaking, &plan);
        AppendBattle(snapshot.battle, &plan);
        AppendMenu(&plan);
        AppendIdentity(snapshot.identity, &plan);
        return plan;
    }

private:
    void AppendHud(const HudSnapshot& hud, UiRenderPlan* plan) const {
        plan->hud_items.push_back({assets_.gold_icon, FormatResource(hud.gold, hud.gold_capacity)});
        plan->hud_items.push_back({assets_.elixir_icon, FormatResource(hud.elixir, hud.elixir_capacity)});
        plan->hud_items.push_back({assets_.gem_icon, FormatResource(hud.gems, hud.gems_capacity)});
    }

    void AppendHealthBars(const std::vector<HealthBarStatus>& health_bars,
                          UiRenderPlan* plan) const {
        for (const HealthBarStatus& status : health_bars) {
            HealthBarRender render;
            render.entity_id = status.entity_id;
            render.is_ally = status.is_ally;
            if (status.max_hp > 0) {
                render.health_ratio =
                    std::max(0.0f, std::min(1.0f, static_cast<float>(status.current_hp) /
                                                       static_cast<float>(status.max_hp)));
            }
            plan->health_bars.push_back(render);
        }
        std::sort(plan->health_bars.begin(), plan->health_bars.end(),
                  [](const HealthBarRender& a, const HealthBarRender& b) {
                      return a.entity_id < b.entity_id;
                  });
    }

    void AppendLoot(const LootAvailabilitySnapshot& loot, UiRenderPlan* plan) const {
        if (loot.gold_available == 0 && loot.elixir_available == 0 &&
            loot.trophies_available == 0) {
            return;
        }
        LootRenderItem item;
        std::ostringstream ss;
        ss << "Loot " << loot.gold_available << "g "
           << loot.elixir_available << "e "
           << loot.trophies_available << "t";
        item.label_text = ss.str();
        plan->loot_items.push_back(item);
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

    void AppendBuildPalette(const BuildPaletteSnapshot& palette, UiRenderPlan* plan) const {
        for (const BuildPaletteEntry& entry : palette.entries) {
            BuildPaletteRenderItem render;
            render.type = entry.type;
            render.is_selected = entry.type == palette.selected_type;
            render.label_text = BuildingTypeToString(entry.type);
            render.cost_text = FormatCost(entry.cost);
            render.time_text = FormatTime(entry.build_time_seconds);
            render.button_asset = assets_.build_button;
            plan->build_palette_items.push_back(render);
        }
    }

    void AppendSelectedBuilding(const SelectedBuildingSnapshot& selected, UiRenderPlan* plan) const {
        if (!selected.has_selection) {
            plan->selected_building.show = false;
            return;
        }
        plan->selected_building.show = true;
        plan->selected_building.title = "Building";
        plan->selected_building.stat_lines.push_back(BuildingTypeToString(selected.type));
        plan->selected_building.stat_lines.push_back("Level " + std::to_string(selected.level));
        plan->selected_building.stat_lines.push_back("HP " + std::to_string(selected.stats.max_hp_));
        plan->selected_building.stat_lines.push_back(
            "Size " + std::to_string(selected.stats.width_) + "x" +
            std::to_string(selected.stats.height_));
        plan->selected_building.collect_button_asset = assets_.collect_button;
    }

    void AppendDeployment(const std::vector<DeploymentOwnerSnapshot>& deployment,
                          int local_owner_id,
                          UiRenderPlan* plan) const {
        for (const DeploymentOwnerSnapshot& owner : deployment) {
            if (owner.owner_id != local_owner_id) {
                continue;
            }
            for (const TroopCountState& troop : owner.troop_counts) {
                DeploymentRenderItem render;
                render.troop_type = troop.troop_type;
                render.remaining_count = troop.remaining_count;
                render.is_selected = owner.has_selection && owner.selected_troop == troop.troop_type;
                render.icon_asset = assets_.deployment_column;
                plan->deployment_items.push_back(render);
            }
        }
    }

    void AppendTroopInspect(const TroopInspectSnapshot& inspect, UiRenderPlan* plan) const {
        if (!inspect.has_selection) {
            plan->troop_inspect.show = false;
            return;
        }
        plan->troop_inspect.show = true;
        plan->troop_inspect.title = "Troop";
        plan->troop_inspect.stat_lines.push_back(TroopTypeToString(inspect.troop_type));
        plan->troop_inspect.stat_lines.push_back("Level " + std::to_string(inspect.level));
        plan->troop_inspect.stat_lines.push_back("HP " + std::to_string(inspect.stats.max_hp_));
        plan->troop_inspect.stat_lines.push_back(
            "DPS " + std::to_string(inspect.stats.damage_per_shot_));
        plan->troop_inspect.stat_lines.push_back(
            "Range " + std::to_string(static_cast<int>(inspect.stats.range_)));
    }

    void AppendMatchmaking(const MatchmakingSnapshot& matchmaking, UiRenderPlan* plan) const {
        plan->matchmaking.button_asset = assets_.matchmaking_button;
        plan->matchmaking.cost_text = FormatCost(matchmaking.cost);
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

    void AppendIdentity(const PlayerIdentitySnapshot& identity, UiRenderPlan* plan) const {
        plan->player_identity.player_id = identity.player_id;
        plan->player_identity.name = identity.name;
        plan->player_identity.icon_asset = identity.icon_id;
    }

    std::string FormatResource(int current, int capacity) const {
        std::ostringstream ss;
        ss << current << "/" << capacity;
        return ss.str();
    }

    std::string FormatCost(const ResourceCost& cost) const {
        std::ostringstream ss;
        ss << cost.gold << "g " << cost.elixir << "e";
        return ss.str();
    }

    std::string FormatTime(float seconds) const {
        std::ostringstream ss;
        ss << static_cast<int>(seconds) << "s";
        return ss.str();
    }

    std::string BuildingTypeToString(Core::BuildingType type) const {
        switch (type) {
            case Core::BuildingType::kTownHall:
                return "Town Hall";
            case Core::BuildingType::kGoldMine:
                return "Gold Mine";
            case Core::BuildingType::kElixirCollector:
                return "Elixir Collector";
            case Core::BuildingType::kGoldStorage:
                return "Gold Storage";
            case Core::BuildingType::kElixirStorage:
                return "Elixir Storage";
            case Core::BuildingType::kBarracks:
                return "Barracks";
            case Core::BuildingType::kCannon:
                return "Cannon";
            case Core::BuildingType::kArcherTower:
                return "Archer Tower";
            case Core::BuildingType::kAirDefense:
                return "Air Defense";
            case Core::BuildingType::kWall:
                return "Wall";
            case Core::BuildingType::kArmyCamp:
                return "Army Camp";
            case Core::BuildingType::kNone:
            default:
                return "None";
        }
    }

    std::string TroopTypeToString(Core::TroopType type) const {
        switch (type) {
            case Core::TroopType::kBarbarian:
                return "Barbarian";
            case Core::TroopType::kArcher:
                return "Archer";
            case Core::TroopType::kGiant:
                return "Giant";
            case Core::TroopType::kWallBreaker:
                return "Wall Breaker";
            case Core::TroopType::kBabyDragon:
                return "Baby Dragon";
            default:
                return "Unknown";
        }
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
