#include <cassert>
#include <cmath>
#include <vector>

#include "Classes/Contract/Engine/InputRouter.h"
#include "Classes/Contract/Engine/MapLayer.h"
#include "Classes/Contract/Engine/TileHighlighter.h"
#include "Classes/Contract/Engine/TilePlacementController.h"
#include "Core/GameConstants.h"
#include "Classes/Contract/Gameplay/Building.h"
#include "Classes/Contract/Gameplay/CostQuery.h"
#include "Classes/Contract/Gameplay/EconomySystem.h"
#include "Classes/Contract/Gameplay/GameEvents.h"
#include "Classes/Contract/Gameplay/HealthComp.h"
#include "Classes/Contract/Gameplay/TroopCommandService.h"
#include "Classes/Contract/Gameplay/Unit.h"
#include "Classes/Contract/Integration/PlayerIdentityService.h"
#include "Classes/Contract/Integration/SceneFlowService.h"
#include "Classes/Integration/GameServices.h"
#include "Classes/Integration/SceneFlowServiceImpl.h"
#include "Classes/Managers/AudioManager/AudioManager.h"
#include "Classes/Managers/LevelManager.h"
#include "Classes/Mocks/Audio/AudioSinkMock.h"
#include "Classes/Mocks/IntegrationMock/SceneFlowService.h"
#include "Classes/Mocks/GameplayMock/TroopCommandServiceMock.h"
#include "Classes/Scenes/GameStageScene.h"
#include "Classes/Scenes/MenuScene.h"
#include "Classes/Scenes/ResultsScene.h"
#include "Classes/UI/UiStateModels.h"

int GetWorldToTileCallCountForTesting();
int GetTileToWorldCenterCallCountForTesting();
void ResetMapLayerCallCountsForTesting();
int GetTileHighlighterEnableCallCountForTesting();
void ResetTileHighlighterCallCountsForTesting();

void TestTilePlacementController() {
    cocos2d::Scene scene;
    MapLayer* map = MapLayer::create("mock_map.tmx");
    TilePlacementController controller(&scene);

    assert(!controller.isPlacing());
    controller.startPlacement(map, "unit.png", cocos2d::Vec2(10.0f, 20.0f));
    assert(controller.isPlacing());
    cocos2d::Vec2 snapped = controller.SnapToValidTile(cocos2d::Vec2(10.0f, 20.0f));
    assert(controller.CanPlaceAt(snapped));
    map->SetBlockedTilesForTesting({map->WorldToTile(snapped)});
    assert(!controller.CanPlaceAt(snapped));
    controller.cancelPlacement();
    assert(!controller.isPlacing());
}

void TestInputRouter() {
    MapLayer* map = MapLayer::create("mock_map.tmx");
    TileHighlighter highlighter(map->getMap());
    InputRouter router;
    router.AttachMap(map, &highlighter);

    router.SetUiConsumesInput(true);
    bool routed = router.RoutePointerMove(cocos2d::Vec2(12.0f, 8.0f));
    assert(!routed);
    auto state = router.GetLastRouteState();
    assert(state.ui_consumed);

    router.SetUiConsumesInput(false);
    const cocos2d::Vec2 screen_pos(static_cast<float>(Core::kTileWidth),
                                   static_cast<float>(Core::kTileWidth));
    routed = router.RoutePointerMove(screen_pos);
    assert(routed);
    state = router.GetLastRouteState();
    assert(!state.ui_consumed);
    const cocos2d::Vec2 expected_world = map->TileToWorldCenter(map->WorldToTile(screen_pos));
    // Policy (Input Routing Policy.md, Semantics): when UI ignores, events route to the map with snapped tile center.
    assert(std::fabs(state.world_pos.x - expected_world.x) < 0.001f);
    assert(std::fabs(state.world_pos.y - expected_world.y) < 0.001f);
}

void TestInputRouterUiConsumesSkipsMap() {
    ResetMapLayerCallCountsForTesting();
    ResetTileHighlighterCallCountsForTesting();
    MapLayer* map = MapLayer::create("mock_map_ui_first.tmx");
    map->SetTileSizeForTesting(10.0f);
    TileHighlighter highlighter(map->getMap());
    InputRouter router;
    router.AttachMap(map, &highlighter);

    router.SetUiConsumesInput(true);
    const cocos2d::Vec2 screen_pos(17.0f, 23.0f);
    bool routed = router.RoutePointerMove(screen_pos);
    assert(!routed);

    InputRouteState state = router.GetLastRouteState();
    // Policy (Input Routing Policy.md, Semantics): UI consume short-circuits map/highlighter and returns false.
    assert(state.ui_consumed);
    assert(GetWorldToTileCallCountForTesting() == 0);
    assert(GetTileToWorldCenterCallCountForTesting() == 0);
    assert(GetTileHighlighterEnableCallCountForTesting() == 0);
}

void TestInputRouterUiIgnoresRoutesToMap() {
    ResetMapLayerCallCountsForTesting();
    ResetTileHighlighterCallCountsForTesting();
    MapLayer* map = MapLayer::create("mock_map_ui_ignore.tmx");
    map->SetTileSizeForTesting(10.0f);
    TileHighlighter highlighter(map->getMap());
    InputRouter router;
    router.AttachMap(map, &highlighter);

    router.SetUiConsumesInput(false);
    const cocos2d::Vec2 screen_pos(12.0f, 8.0f);
    bool routed = router.RoutePointerMove(screen_pos);
    assert(routed);

    InputRouteState state = router.GetLastRouteState();
    assert(!state.ui_consumed);
    const cocos2d::Vec2 expected_world = map->TileToWorldCenter(map->WorldToTile(screen_pos));
    // Policy (Input Routing Policy.md, Semantics): when UI ignores, events route to the map with snapped tile center.
    assert(std::fabs(state.world_pos.x - expected_world.x) < 0.001f);
    assert(std::fabs(state.world_pos.y - expected_world.y) < 0.001f);
    assert(GetWorldToTileCallCountForTesting() > 0);
    assert(GetTileToWorldCenterCallCountForTesting() > 0);
    assert(GetTileHighlighterEnableCallCountForTesting() > 0);
}

void TestInputRouterWithoutMapIsSafe() {
    InputRouter router;
    router.SetUiConsumesInput(false);

    const cocos2d::Vec2 screen_pos(3.0f, 4.0f);
    bool routed = router.RoutePointerMove(screen_pos);
    assert(!routed);

    InputRouteState state = router.GetLastRouteState();
    // Policy (Input Routing Policy.md, Error handling): with no map attached, routing returns false and records ui_consumed.
    assert(state.ui_consumed);
}

void TestSceneFlowService() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    cocos2d::Scene* boot_scene = flow->CreateBootScene();
    assert(boot_scene != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kBoot);
    cocos2d::Scene* boot_scene_again = flow->CreateBootScene();
    assert(boot_scene_again != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kBoot);
    assert(boot_scene != boot_scene_again);

    cocos2d::Scene* menu_scene = flow->ShowMenuScene();
    assert(menu_scene != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kMenu);
    cocos2d::Scene* menu_scene_again = flow->ShowMenuScene();
    assert(menu_scene_again != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kMenu);
    assert(menu_scene != menu_scene_again);

    Integration::BattleLaunchParams launch{"maps/test.tmx", 42, false};
    cocos2d::Scene* game_scene = flow->StartGame(launch);
    assert(game_scene != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kGame);
    assert(flow->GetLastLaunchParams().map_path == "maps/test.tmx");
    assert(flow->GetLastLaunchParams().seed == 42);
    Integration::BattleLaunchParams second_launch{"maps/second.tmx", -1, true};
    cocos2d::Scene* game_scene_again = flow->StartGame(second_launch);
    assert(game_scene_again != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kGame);
    assert(flow->GetLastLaunchParams().map_path == "maps/second.tmx");
    assert(flow->GetLastLaunchParams().seed == -1);
    assert(flow->GetLastLaunchParams().use_random_base);

    Gameplay::BattleEndEvent summary;
    summary.stars_earned = 3;
    Integration::ResultsScreenData results{summary, 180, 5, 2};
    cocos2d::Scene* results_scene = flow->ShowResults(results);
    assert(results_scene != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kResults);
    Integration::ResultsScreenData cached_results = flow->GetLastResults();
    assert(cached_results.elapsed_seconds == 180);
    assert(cached_results.troops_deployed == 5);
    assert(cached_results.troops_remaining == 2);
    cocos2d::Scene* results_scene_again = flow->ShowResults(results);
    assert(results_scene_again != nullptr);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kResults);

    delete boot_scene;
    delete boot_scene_again;
    delete menu_scene;
    delete menu_scene_again;
    delete game_scene;
    delete game_scene_again;
    delete results_scene;
    delete results_scene_again;
    delete flow;
}

void TestSceneFlowMockDelegation() {
    Integration::SceneFlowService* mock =
        Integration::CreateMockSceneFlowService(Integration::CreateSceneFlowService());

    cocos2d::Scene* boot_scene = mock->CreateBootScene();
    assert(boot_scene != nullptr);
    assert(mock->GetCurrentStage() == Integration::SceneStage::kBoot);

    Integration::BattleLaunchParams launch{"delegated_map.tmx", 11};
    cocos2d::Scene* game_scene = mock->StartGame(launch);
    assert(game_scene != nullptr);
    Integration::BattleLaunchParams cached_launch = mock->GetLastLaunchParams();
    assert(cached_launch.map_path == "delegated_map.tmx");
    assert(cached_launch.seed == 11);
    assert(mock->GetCurrentStage() == Integration::SceneStage::kGame);

    Gameplay::BattleEndEvent summary{};
    summary.result = Gameplay::BattleResult::kVictory;
    Integration::ResultsScreenData results{summary, 60, 3, 2};
    cocos2d::Scene* results_scene = mock->ShowResults(results);
    assert(results_scene != nullptr);
    assert(mock->GetCurrentStage() == Integration::SceneStage::kResults);
    Integration::ResultsScreenData cached_results = mock->GetLastResults();
    assert(cached_results.elapsed_seconds == 60);
    assert(cached_results.troops_deployed == 3);
    assert(cached_results.troops_remaining == 2);

    delete boot_scene;
    delete game_scene;
    delete results_scene;
    delete mock;
}

void TestPlayerIdentityService() {
    Integration::PlayerIdentityService* service = Integration::PlayerIdentityService::GetInstance();

    Integration::PlayerIdentity created = service->CreateIdentity("Alice", "avatar_knight");
    assert(created.player_id >= 1);
    assert(created.name == "Alice");
    assert(created.icon_id == "avatar_knight");

    Integration::PlayerIdentity second = service->CreateIdentity("Bob", "avatar_wizard");
    assert(second.player_id == created.player_id + 1);
    assert(second.name == "Bob");

    Integration::PlayerIdentity stored = service->GetIdentity();
    assert(stored.player_id == second.player_id);
    assert(stored.name == second.name);
    assert(stored.icon_id == second.icon_id);

    Integration::PlayerIdentity override_identity;
    override_identity.player_id = 42;
    override_identity.name = "Override";
    override_identity.icon_id = "";
    service->SetIdentity(override_identity);

    Integration::PlayerIdentity updated = service->GetIdentity();
    assert(updated.player_id == 42);
    assert(updated.name == "Override");
    assert(updated.icon_id.empty());
}

void TestVerticalSliceSceneFlow() {
    Integration::SceneFlowService* scene_flow = Integration::CreateSceneFlowService();
    Gameplay::GameEventManager* event_manager = Integration::ResolveGameEventManager();

    assert(scene_flow != nullptr);
    assert(event_manager != nullptr);

    cocos2d::Scene* boot_scene = scene_flow->CreateBootScene();
    assert(boot_scene != nullptr);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kBoot);

    cocos2d::Scene* menu_scene = scene_flow->ShowMenuScene();
    assert(menu_scene != nullptr);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kMenu);

    Integration::BattleLaunchParams launch{"non_empty_map_path", 123};
    cocos2d::Scene* game_scene = scene_flow->StartGame(launch);
    assert(game_scene != nullptr);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kGame);
    Integration::BattleLaunchParams cached_launch = scene_flow->GetLastLaunchParams();
    assert(cached_launch.map_path == "non_empty_map_path");
    assert(cached_launch.seed == 123);

    Gameplay::BattleEndEvent battle_end{};
    battle_end.stars_earned = 2;
    battle_end.troops_deployed = 5;
    battle_end.troops_remaining = 1;
    event_manager->BroadcastBattleEnded(battle_end);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kGame);
    Gameplay::BattleEndEvent cached_end = event_manager->GetLastBattleEnded();
    assert(cached_end.troops_deployed == 5);
    assert(cached_end.troops_remaining == 1);

    Integration::ResultsScreenData results{battle_end, 90, 5, 1};
    // Stage 2 requires the explicit ShowResults call (not BattleEndEvent auto-transition).
    cocos2d::Scene* results_scene = scene_flow->ShowResults(results);
    assert(results_scene != nullptr);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kResults);
    Integration::ResultsScreenData cached_results = scene_flow->GetLastResults();
    assert(cached_results.summary.stars_earned == battle_end.stars_earned);
    assert(cached_results.elapsed_seconds == results.elapsed_seconds);
    assert(cached_results.troops_deployed == results.troops_deployed);
    assert(cached_results.troops_remaining == results.troops_remaining);

    // Reset cached battle-end state for isolation because the event manager is a singleton.
    Gameplay::BattleEndEvent baseline_end{};
    event_manager->BroadcastBattleEnded(baseline_end);

    delete boot_scene;
    delete menu_scene;
    delete game_scene;
    delete results_scene;
    delete scene_flow;
}

void TestMenuSceneMapSelectionPropagation() {
    Integration::SceneFlowService* scene_flow = Integration::ResolveSceneFlowService();
    LevelManager* level_manager = LevelManager::GetInstance();

    level_manager->ResetSelection();

    cocos2d::Scene* menu_scene_wrapper = scene_flow->ShowMenuScene();
    auto* menu_scene = static_cast<MenuScene*>(menu_scene_wrapper);
    level_manager->SelectMapA();
    level_manager->SetSeed(101);
    assert(level_manager->GetMapAPath() != level_manager->GetMapBPath());
    cocos2d::Scene* game_scene_a = menu_scene->StartSelectedMap();
    assert(game_scene_a != nullptr);
    Integration::BattleLaunchParams launch_a = scene_flow->GetLastLaunchParams();
    assert(launch_a.map_path == level_manager->GetMapAPath());
    assert(launch_a.seed == 101);

    cocos2d::Scene* menu_scene_wrapper_b = scene_flow->ShowMenuScene();
    menu_scene = static_cast<MenuScene*>(menu_scene_wrapper_b);
    level_manager->SelectMapB();
    level_manager->SetSeed(202);
    assert(launch_a.seed != level_manager->GetSeed());
    cocos2d::Scene* game_scene_b = menu_scene->StartSelectedMap();
    assert(game_scene_b != nullptr);
    Integration::BattleLaunchParams launch_b = scene_flow->GetLastLaunchParams();
    assert(launch_b.map_path == level_manager->GetMapBPath());
    assert(launch_a.map_path != launch_b.map_path);
    assert(launch_b.seed == 202);

    delete game_scene_a;
    delete game_scene_b;
    delete menu_scene_wrapper;
    delete menu_scene_wrapper_b;
    level_manager->ResetSelection();
    delete scene_flow;
}

void TestMenuSceneInputWiring() {
    Integration::SceneFlowService* scene_flow = Integration::CreateSceneFlowService();
    cocos2d::Scene* menu_scene_wrapper = scene_flow->ShowMenuScene();
    auto* menu_scene = static_cast<MenuScene*>(menu_scene_wrapper);
    LevelManager* level_manager = LevelManager::GetInstance();

    level_manager->ResetSelection();
    bool consumed = menu_scene->HandleTap(cocos2d::Vec2(540.0f, 200.0f));
    assert(consumed);
    assert(menu_scene->GetLastMenuActionForTest() == "map_b");
    assert(level_manager->GetSelectedMapPath() == level_manager->GetMapBPath());

    consumed = menu_scene->HandleTap(cocos2d::Vec2(260.0f, 100.0f));
    assert(consumed);
    assert(menu_scene->GetLastMenuActionForTest() == "leagues");

    consumed = menu_scene->HandleTap(cocos2d::Vec2(540.0f, 100.0f));
    assert(consumed);
    assert(menu_scene->GetLastMenuActionForTest() == "replays");

    consumed = menu_scene->HandleTap(cocos2d::Vec2(400.0f, 140.0f));
    assert(consumed);
    assert(menu_scene->GetLastMenuActionForTest() == "start");
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kGame);

    consumed = menu_scene->HandleTap(cocos2d::Vec2(10.0f, 10.0f));
    assert(!consumed);

    delete menu_scene_wrapper;
    delete scene_flow;
}

void TestGameServicesSmoke() {
    Integration::SceneFlowService* scene_flow = Integration::ResolveSceneFlowService();
    InputRouter* router = Integration::ResolveInputRouter();
    Gameplay::GameEventManager* event_manager = Integration::ResolveGameEventManager();

    assert(Integration::IsUsingMocksForTest());
    assert(scene_flow != nullptr);
    assert(router != nullptr);
    assert(event_manager != nullptr);

    Gameplay::BattleEndEvent baseline_event;
    event_manager->BroadcastBattleEnded(baseline_event);
    Gameplay::BattleEndEvent cached_baseline = event_manager->GetLastBattleEnded();
    assert(cached_baseline.stars_earned == 0);
    assert(cached_baseline.troops_deployed == 0);

    cocos2d::Scene* boot_scene = scene_flow->CreateBootScene();
    assert(boot_scene != nullptr);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kBoot);

    MapLayer* map = MapLayer::create("smoke_test_map.tmx");
    TileHighlighter highlighter(map->getMap());
    router->AttachMap(map, &highlighter);

    bool routed = router->RoutePointerMove(cocos2d::Vec2(5.0f, 5.0f));
    assert(!routed);
    InputRouteState route_state = router->GetLastRouteState();
    assert(route_state.ui_consumed);
    router->SetUiConsumesInput(false);
    routed = router->RoutePointerMove(cocos2d::Vec2(10.0f, 10.0f));
    assert(routed);
    route_state = router->GetLastRouteState();
    assert(!route_state.ui_consumed);

    Integration::BattleLaunchParams launch{"maps/smoke_test.tmx", 7};
    cocos2d::Scene* game_scene = scene_flow->StartGame(launch);
    assert(game_scene != nullptr);
    Integration::BattleLaunchParams cached_launch = scene_flow->GetLastLaunchParams();
    assert(cached_launch.map_path == "maps/smoke_test.tmx");
    assert(cached_launch.seed == 7);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kGame);

    cocos2d::Scene* menu_scene = scene_flow->ShowMenuScene();
    assert(menu_scene != nullptr);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kMenu);

    Gameplay::BattleEndEvent battle_event;
    battle_event.stars_earned = 1;
    battle_event.troops_deployed = 4;
    battle_event.troops_remaining = 2;

    class TestListener : public Gameplay::IGameEventListener {
    public:
        void OnBattleEnded(const Gameplay::BattleEndEvent& evt) override {
            ++call_count_;
            cached_ = evt;
        }

        int call_count_ = 0;
        Gameplay::BattleEndEvent cached_{};
    } listener;

    event_manager->AddListener(&listener);
    event_manager->BroadcastBattleEnded(battle_event);
    assert(listener.call_count_ == 1);
    Gameplay::BattleEndEvent cached_event = event_manager->GetLastBattleEnded();
    assert(cached_event.troops_deployed == 4);
    assert(cached_event.troops_remaining == 2);
    event_manager->RemoveListener(&listener);

    event_manager->BroadcastBattleEnded(baseline_event);
    Gameplay::BattleEndEvent cached_after_removal = event_manager->GetLastBattleEnded();
    assert(cached_after_removal.stars_earned == 0);
    assert(listener.call_count_ == 1);

    Integration::ResultsScreenData results{battle_event, 30, 3, 1};
    cocos2d::Scene* results_scene = scene_flow->ShowResults(results);
    assert(results_scene != nullptr);
    Integration::ResultsScreenData cached_results = scene_flow->GetLastResults();
    assert(cached_results.elapsed_seconds == 30);
    assert(cached_results.troops_deployed == 3);
    assert(cached_results.troops_remaining == 1);
    assert(scene_flow->GetCurrentStage() == Integration::SceneStage::kResults);

    delete boot_scene;
    delete game_scene;
    delete scene_flow;
    delete router;
    delete menu_scene;
    delete results_scene;
}

void TestBattleEndPayload() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    Gameplay::BattleEndEvent evt;
    evt.result = Gameplay::BattleResult::kVictory;
    evt.stars_earned = 2;
    evt.destruction_percent = 75;
    evt.gold_stolen = 500;
    evt.elixir_stolen = 250;
    evt.trophies_earned = 12;
    evt.trophies_total = 1400;
    evt.battle_duration_seconds = 140;
    evt.troops_deployed = 6;
    evt.troops_remaining = 3;
    evt.spells_used = 1;
    manager->BroadcastBattleEnded(evt);

    Gameplay::BattleEndEvent cached = manager->GetLastBattleEnded();
    assert(cached.battle_duration_seconds == 140);
    assert(cached.troops_deployed == 6);
    assert(cached.troops_remaining == 3);
    assert(cached.spells_used == 1);
    assert(cached.trophies_earned == 12);
    assert(cached.trophies_total == 1400);
}

void TestLootAvailabilityPayload() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    Gameplay::LootAvailabilityEvent evt{};
    evt.gold_available = 700;
    evt.elixir_available = 500;
    evt.trophies_available = 15;
    manager->BroadcastLootAvailabilityUpdated(evt);

    Gameplay::LootAvailabilityEvent cached = manager->GetLastLootAvailability();
    assert(cached.gold_available == 700);
    assert(cached.elixir_available == 500);
    assert(cached.trophies_available == 15);
}

void TestDeploymentContractExtensions() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();

    class DeploymentListener : public Gameplay::IGameEventListener {
    public:
        void OnTroopCountUpdated(const Gameplay::TroopCountUpdateEvent& evt) override {
            count_events.push_back(evt);
        }

        void OnDeploymentSelectionChanged(const Gameplay::DeploymentSelectionEvent& evt) override {
            selection_events.push_back(evt);
        }

        std::vector<Gameplay::TroopCountUpdateEvent> count_events;
        std::vector<Gameplay::DeploymentSelectionEvent> selection_events;
    } listener;

    // Before registration, events should not reach the listener.
    Gameplay::TroopCountUpdateEvent pre_add_evt{};
    pre_add_evt.remaining_count = 99;
    manager->BroadcastTroopCountUpdated(pre_add_evt);
    assert(listener.count_events.empty());

    manager->AddListener(&listener);

    Gameplay::TroopCountUpdateEvent player_counts{};
    player_counts.owner_id = 0;
    player_counts.troop_type = Core::TroopType::kBarbarian;
    player_counts.remaining_count = 7;
    manager->BroadcastTroopCountUpdated(player_counts);

    Gameplay::TroopCountUpdateEvent enemy_counts{};
    enemy_counts.owner_id = 1;
    enemy_counts.troop_type = Core::TroopType::kArcher;
    enemy_counts.remaining_count = 2;
    manager->BroadcastTroopCountUpdated(enemy_counts);

    Gameplay::DeploymentSelectionEvent select_evt{};
    select_evt.owner_id = 0;
    select_evt.has_selection = true;
    select_evt.troop_type = Core::TroopType::kGiant;
    manager->BroadcastDeploymentSelectionChanged(select_evt);

    Gameplay::DeploymentSelectionEvent clear_evt{};
    clear_evt.owner_id = 0;
    clear_evt.has_selection = false;
    manager->BroadcastDeploymentSelectionChanged(clear_evt);

    assert(listener.count_events.size() == 2);
    assert(listener.count_events[0].owner_id == 0);
    assert(listener.count_events[0].troop_type == Core::TroopType::kBarbarian);
    assert(listener.count_events[0].remaining_count == 7);
    assert(listener.count_events[1].owner_id == 1);
    assert(listener.count_events[1].troop_type == Core::TroopType::kArcher);
    assert(listener.count_events[1].remaining_count == 2);

    assert(listener.selection_events.size() == 2);
    assert(listener.selection_events[0].owner_id == 0);
    assert(listener.selection_events[0].has_selection);
    assert(listener.selection_events[0].troop_type == Core::TroopType::kGiant);
    assert(listener.selection_events[1].owner_id == 0);
    assert(!listener.selection_events[1].has_selection);

    manager->RemoveListener(&listener);
}

void TestEconomySystem() {
    EconomySystem* economy = EconomySystem::GetInstance();
    economy->Reset();

    Building* gold_mine = Building::create(Core::BuildingType::kGoldMine, 1, 0);
    Building* elixir_collector = Building::create(Core::BuildingType::kElixirCollector, 1, 0);

    cocos2d::Vector<Building*> buildings;
    buildings.pushBack(gold_mine);
    buildings.pushBack(elixir_collector);
    economy->RecalculateLimits(buildings);

    assert(economy->GetCurrentGold() == 0);
    assert(economy->GetCurrentElixir() == 0);
    assert(economy->GetMaxGold() > 0);
    assert(economy->GetMaxElixir() > 0);

    ResourceCost building_cost = CostQuery::GetInstance()->GetBuildingPlacementCost(Core::BuildingType::kCannon, 1);
    assert(building_cost.gold > 0);
    assert(economy->CanAffordCost(building_cost) == false);

    int collected_gold = economy->TryCollectResource(gold_mine);
    assert(collected_gold > 0);
    assert(economy->GetCurrentGold() == collected_gold);

    int collected_elixir = economy->TryCollectResource(elixir_collector);
    assert(collected_elixir > 0);
    assert(economy->GetCurrentElixir() == collected_elixir);

    int previous_gold = economy->GetCurrentGold();
    bool spent = economy->SpendGold(previous_gold + 1);
    assert(!spent);
    assert(economy->GetCurrentGold() == previous_gold);

    economy->AddGold(50);
    assert(economy->GetCurrentGold() >= previous_gold);

    ResourceCost troop_cost = CostQuery::GetInstance()->GetTroopTrainingCost(Core::TroopType::kBarbarian, 1);
    economy->AddElixir(troop_cost.elixir);
    bool afford_troop = economy->CanAffordTroop(Core::TroopType::kBarbarian, 1);
    assert(afford_troop);
}

void TestMatchmakingCost() {
    CostQuery* cost_query = CostQuery::GetInstance();
    ResourceCost matchmaking_cost = cost_query->GetMatchmakingCost();
    assert(matchmaking_cost.gold == 50);
    assert(matchmaking_cost.elixir == 0);
    assert(matchmaking_cost.population == 0);

    EconomySystem* economy = EconomySystem::GetInstance();
    economy->Reset();
    economy->AddGold(matchmaking_cost.gold);
    assert(economy->CanAffordCost(matchmaking_cost));
    bool spent = economy->SpendGold(matchmaking_cost.gold);
    assert(spent);
    assert(economy->GetCurrentGold() == 0);
}

void TestTroopCommandServiceMock() {
    Gameplay::TroopCommandServiceMock::Reset();
    Gameplay::TroopCommandService* service = Gameplay::TroopCommandService::GetInstance();

    Gameplay::TroopTrainRequest train_request{};
    train_request.troop_type = Core::TroopType::kArcher;
    train_request.level = 2;
    train_request.count = 3;
    train_request.owner_id = 0;
    bool trained = service->RequestTrainTroop(train_request);
    assert(trained);
    assert(Gameplay::TroopCommandServiceMock::GetTrainRequestCount() == 1);
    Gameplay::TroopTrainRequest cached_train = Gameplay::TroopCommandServiceMock::GetLastTrainRequest();
    assert(cached_train.troop_type == Core::TroopType::kArcher);
    assert(cached_train.level == 2);
    assert(cached_train.count == 3);

    Gameplay::TroopDeployRequest deploy_request{};
    deploy_request.troop_type = Core::TroopType::kGiant;
    deploy_request.level = 1;
    deploy_request.owner_id = 0;
    deploy_request.world_x = 10.0f;
    deploy_request.world_y = 20.0f;
    bool deployed = service->RequestDeployTroop(deploy_request);
    assert(deployed);
    assert(Gameplay::TroopCommandServiceMock::GetDeployRequestCount() == 1);
    Gameplay::TroopDeployRequest cached_deploy = Gameplay::TroopCommandServiceMock::GetLastDeployRequest();
    assert(cached_deploy.troop_type == Core::TroopType::kGiant);
    assert(cached_deploy.world_x == 10.0f);
    assert(cached_deploy.world_y == 20.0f);
}

void TestHealthComp() {
    HealthComp health;
    health.InitStats(100);
    assert(std::fabs(health.GetHealthPercentage() - 1.0f) < 0.0001f);
    assert(std::fabs(health.GetCurrentHealth() - 100.0f) < 0.0001f);
    assert(std::fabs(health.GetMaxHealth() - 100.0f) < 0.0001f);
    assert(!health.IsDead());

    bool died = health.TakeDamage(40);
    assert(!died);
    assert(std::fabs(health.GetHealthPercentage() - 0.6f) < 0.0001f);
    assert(std::fabs(health.GetCurrentHealth() - 60.0f) < 0.0001f);

    died = health.TakeDamage(60);
    assert(died);
    assert(health.IsDead());
    assert(std::fabs(health.GetHealthPercentage() - 0.0f) < 0.0001f);

    health.InitStats(50);
    health.Heal(10);
    assert(std::fabs(health.GetCurrentHealth() - 50.0f) < 0.0001f);
}

void TestUnitAndBuilding() {
    Unit* air_unit = Unit::create(Core::TroopType::kBabyDragon, 2, 0);
    assert(air_unit->GetGeneralType() == Core::GeneralType::kAir);
    assert(air_unit->GetState() == Core::UnitAnimationState::kIdle);
    assert(air_unit->GetDamage() > 0);

    Building* cannon = Building::create(Core::BuildingType::kCannon, 2, 0);
    assert(cannon->GetLevel() == 2);
    assert(cannon->GetBuildingType() == Core::BuildingType::kCannon);
    assert(!cannon->IsConstructing());
    cannon->StartConstruction(3.0f);
    assert(cannon->IsConstructing());
    assert(cannon->GetConstructionProgress() <= 1.0f);

    Building* collector = Building::create(Core::BuildingType::kGoldMine, 1, 0);
    assert(collector->GetStoredResource() > 0);
    assert(collector->GetStoragePercentage() > 0.0f);
    int collected = collector->CollectResource(10);
    assert(collected >= 0);
    assert(collector->GetStoredResource() <= static_cast<int>(collector->GetStoragePercentage() * 200));

    cannon->PlayAttackAnimation();
    cannon->PlayHurtEffect();
    cannon->PlayDestroyedAnimation();
}

void TestHudStateUpdates() {
    HudState hud_state;

    Gameplay::ResourceUpdateEvent gold_evt;
    gold_evt.resource_type = "Gold";
    gold_evt.current_amount = 500;
    gold_evt.max_capacity = 1000;
    hud_state.ApplyResourceUpdate(gold_evt);
    HudSnapshot hud_snapshot = hud_state.GetSnapshot();
    assert(hud_snapshot.gold == 500);
    assert(hud_snapshot.gold_capacity == 1000);

    Gameplay::ResourceUpdateEvent elixir_evt;
    elixir_evt.resource_type = "Elixir";
    elixir_evt.current_amount = 250;
    elixir_evt.max_capacity = 750;
    hud_state.ApplyResourceUpdate(elixir_evt);
    hud_snapshot = hud_state.GetSnapshot();
    assert(hud_snapshot.elixir == 250);
    assert(hud_snapshot.elixir_capacity == 750);

    Gameplay::ResourceUpdateEvent population_evt;
    population_evt.resource_type = "Population";
    population_evt.current_amount = 10;
    population_evt.max_capacity = 20;
    hud_state.ApplyResourceUpdate(population_evt);
    // Non-HUD resource types do not modify gold/elixir state.
    hud_snapshot = hud_state.GetSnapshot();
    assert(hud_snapshot.gold == 500);
    assert(hud_snapshot.elixir == 250);

    Gameplay::ResourceUpdateEvent gem_evt;
    gem_evt.resource_type = "Gems";
    gem_evt.current_amount = 3;
    gem_evt.max_capacity = 0;
    hud_state.ApplyResourceUpdate(gem_evt);
    hud_snapshot = hud_state.GetSnapshot();
    assert(hud_snapshot.gems == 3);

    Gameplay::ResourceUpdateEvent trophy_evt;
    trophy_evt.resource_type = "Trophies";
    trophy_evt.current_amount = 1200;
    trophy_evt.max_capacity = 0;
    hud_state.ApplyResourceUpdate(trophy_evt);
    hud_snapshot = hud_state.GetSnapshot();
    assert(hud_snapshot.trophies == 1200);
}

void TestSelectionState() {
    SelectionState selection_state;
    SelectionSnapshot selection_snapshot = selection_state.GetSnapshot();
    assert(!selection_snapshot.has_selection);

    selection_state.SelectEntity(7);
    selection_snapshot = selection_state.GetSnapshot();
    assert(selection_snapshot.has_selection);
    assert(selection_snapshot.selected_entity_id == 7);

    selection_state.SelectEntity(9);
    selection_snapshot = selection_state.GetSnapshot();
    assert(selection_snapshot.selected_entity_id == 9);

    Gameplay::EntityDestroyEvent other_destroy{};
    other_destroy.instance_id = 3;
    selection_state.OnEntityDestroyed(other_destroy);
    selection_snapshot = selection_state.GetSnapshot();
    assert(selection_snapshot.selected_entity_id == 9);

    Gameplay::EntityDestroyEvent destroy_evt{};
    destroy_evt.instance_id = 9;
    selection_state.OnEntityDestroyed(destroy_evt);
    selection_snapshot = selection_state.GetSnapshot();
    assert(!selection_snapshot.has_selection);
}

void TestHealthBarState() {
    HealthBarState health_state;

    Gameplay::EntitySpawnEvent ally_spawn{};
    ally_spawn.instance_id = 1;
    ally_spawn.owner_id = 0;
    ally_spawn.current_hp = 120;
    ally_spawn.max_hp = 150;
    health_state.OnEntitySpawned(ally_spawn, /*is_ally=*/true);

    Gameplay::EntitySpawnEvent enemy_spawn{};
    enemy_spawn.instance_id = 2;
    enemy_spawn.owner_id = 1;
    enemy_spawn.current_hp = 200;
    enemy_spawn.max_hp = 250;
    health_state.OnEntitySpawned(enemy_spawn, /*is_ally=*/false);

    const HealthBarStatus* ally_status = health_state.GetEntry(1);
    assert(ally_status != nullptr);
    assert(ally_status->current_hp == 120);
    assert(ally_status->max_hp == 150);
    assert(ally_status->is_ally);

    const HealthBarStatus* enemy_status = health_state.GetEntry(2);
    assert(enemy_status != nullptr);
    assert(!enemy_status->is_ally);

    Gameplay::DamageEvent dmg{};
    dmg.target_instance_id = 1;
    dmg.current_hp = 80;
    dmg.max_hp = 150;
    health_state.OnEntityDamaged(dmg);
    ally_status = health_state.GetEntry(1);
    assert(ally_status != nullptr);
    assert(ally_status->current_hp == 80);
    assert(ally_status->max_hp == 150);

    Gameplay::DamageEvent unknown_damage{};
    unknown_damage.target_instance_id = 99;
    unknown_damage.current_hp = 5;
    unknown_damage.max_hp = 10;
    health_state.OnEntityDamaged(unknown_damage);
    assert(health_state.GetEntry(99) == nullptr);

    Gameplay::EntityDestroyEvent destroy_evt{};
    destroy_evt.instance_id = 2;
    health_state.OnEntityDestroyed(destroy_evt);
    assert(health_state.GetEntry(2) == nullptr);
}

void TestUiStateStoreSnapshot() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();

    UiStateStore store(/*local_owner_id=*/0);

    // Before attaching, broadcasts should not update the store.
    Gameplay::ResourceUpdateEvent pre_attach_resource{};
    pre_attach_resource.resource_type = "Gold";
    pre_attach_resource.current_amount = 99;
    manager->BroadcastResourceChange(pre_attach_resource);
    UiStateSnapshot pre_attach_snapshot = store.GetSnapshot();
    assert(pre_attach_snapshot.hud.gold == 0);

    store.Attach(manager);

    store.SetSelectedEntity(123);

    Gameplay::ResourceUpdateEvent gold_evt{};
    gold_evt.resource_type = "Gold";
    gold_evt.current_amount = 400;
    gold_evt.max_capacity = 500;
    manager->BroadcastResourceChange(gold_evt);

    Gameplay::LootAvailabilityEvent loot_evt{};
    loot_evt.gold_available = 1000;
    loot_evt.elixir_available = 800;
    loot_evt.trophies_available = 20;
    manager->BroadcastLootAvailabilityUpdated(loot_evt);

    Gameplay::EntitySpawnEvent spawn_evt{};
    spawn_evt.instance_id = 10;
    spawn_evt.owner_id = 0;
    spawn_evt.current_hp = 50;
    spawn_evt.max_hp = 75;
    manager->BroadcastEntitySpawned(spawn_evt);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.hud.gold == 400);
    assert(snapshot.hud.gold_capacity == 500);
    assert(snapshot.loot_availability.gold_available == 1000);
    assert(snapshot.loot_availability.elixir_available == 800);
    assert(snapshot.loot_availability.trophies_available == 20);
    assert(snapshot.selection.has_selection);
    assert(snapshot.selection.selected_entity_id == 123);
    assert(snapshot.health_bars.size() == 1);
    assert(snapshot.health_bars[0].entity_id == 10);

    store.Detach(manager);

    Gameplay::ResourceUpdateEvent post_detach_evt{};
    post_detach_evt.resource_type = "Gold";
    post_detach_evt.current_amount = 1;
    manager->BroadcastResourceChange(post_detach_evt);
    UiStateSnapshot post_detach_snapshot = store.GetSnapshot();
    assert(post_detach_snapshot.hud.gold == 400);
}

void TestUiStateStoreSelectionAndDestroy() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    store.SetSelectedEntity(55);
    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.selection.has_selection);

    Gameplay::EntityDestroyEvent unrelated_destroy{};
    unrelated_destroy.instance_id = 99;
    manager->BroadcastEntityDestroyed(unrelated_destroy);
    snapshot = store.GetSnapshot();
    assert(snapshot.selection.has_selection);

    Gameplay::EntityDestroyEvent destroy_evt{};
    destroy_evt.instance_id = 55;
    manager->BroadcastEntityDestroyed(destroy_evt);
    snapshot = store.GetSnapshot();
    assert(!snapshot.selection.has_selection);

    store.Detach(manager);
}

void TestUiStateStoreHealthBars() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/1);
    store.Attach(manager);

    Gameplay::EntitySpawnEvent ally{};
    ally.instance_id = 5;
    ally.owner_id = 1;
    ally.current_hp = 60;
    ally.max_hp = 80;
    manager->BroadcastEntitySpawned(ally);

    Gameplay::EntitySpawnEvent enemy{};
    enemy.instance_id = 6;
    enemy.owner_id = 2;
    enemy.current_hp = 120;
    enemy.max_hp = 150;
    manager->BroadcastEntitySpawned(enemy);

    Gameplay::DamageEvent dmg{};
    dmg.target_instance_id = 6;
    dmg.current_hp = 90;
    dmg.max_hp = 150;
    manager->BroadcastEntityDamaged(dmg);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.health_bars.size() == 2);
    assert(snapshot.health_bars[0].entity_id == 5);
    assert(snapshot.health_bars[0].is_ally);
    assert(snapshot.health_bars[1].entity_id == 6);
    assert(!snapshot.health_bars[1].is_ally);
    assert(snapshot.health_bars[1].current_hp == 90);

    Gameplay::EntityDestroyEvent destroy_enemy{};
    destroy_enemy.instance_id = 6;
    manager->BroadcastEntityDestroyed(destroy_enemy);
    snapshot = store.GetSnapshot();
    assert(snapshot.health_bars.size() == 1);
    assert(snapshot.health_bars[0].entity_id == 5);

    store.Detach(manager);
}

void TestUiStateStoreDeploymentBar() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);

    // Pre-attach broadcasts should be ignored by the store.
    Gameplay::TroopCountUpdateEvent pre_attach_count{};
    pre_attach_count.owner_id = 0;
    pre_attach_count.troop_type = Core::TroopType::kBarbarian;
    pre_attach_count.remaining_count = 9;
    manager->BroadcastTroopCountUpdated(pre_attach_count);
    UiStateSnapshot initial_snapshot = store.GetSnapshot();
    assert(initial_snapshot.deployment.empty());

    store.Attach(manager);

    Gameplay::TroopCountUpdateEvent count_evt{};
    count_evt.owner_id = 0;
    count_evt.troop_type = Core::TroopType::kBarbarian;
    count_evt.remaining_count = 5;
    manager->BroadcastTroopCountUpdated(count_evt);

    count_evt.remaining_count = 3;
    manager->BroadcastTroopCountUpdated(count_evt);

    Gameplay::TroopCountUpdateEvent enemy_count{};
    enemy_count.owner_id = 2;
    enemy_count.troop_type = Core::TroopType::kArcher;
    enemy_count.remaining_count = 7;
    manager->BroadcastTroopCountUpdated(enemy_count);

    Gameplay::DeploymentSelectionEvent select_evt{};
    select_evt.owner_id = 0;
    select_evt.has_selection = true;
    select_evt.troop_type = Core::TroopType::kGiant;
    manager->BroadcastDeploymentSelectionChanged(select_evt);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.deployment.size() == 2);
    const DeploymentOwnerSnapshot& player = snapshot.deployment[0];
    assert(player.owner_id == 0);
    assert(player.has_selection);
    assert(player.selected_troop == Core::TroopType::kGiant);
    assert(player.troop_counts.size() == 1);
    assert(player.troop_counts[0].remaining_count == 3);

    const DeploymentOwnerSnapshot& enemy = snapshot.deployment[1];
    assert(enemy.owner_id == 2);
    assert(!enemy.has_selection);
    assert(enemy.troop_counts.size() == 1);
    assert(enemy.troop_counts[0].troop_type == Core::TroopType::kArcher);
    assert(enemy.troop_counts[0].remaining_count == 7);

    store.Detach(manager);

    count_evt.remaining_count = 1;
    manager->BroadcastTroopCountUpdated(count_evt);
    snapshot = store.GetSnapshot();
    assert(snapshot.deployment.size() == 2);
    assert(snapshot.deployment[0].troop_counts[0].remaining_count == 3);
}

void TestUiStateStoreBuildingProgress() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::EntitySpawnEvent building_spawn{};
    building_spawn.instance_id = 77;
    building_spawn.is_building = true;
    building_spawn.building_type = Core::BuildingType::kCannon;
    manager->BroadcastEntitySpawned(building_spawn);

    Gameplay::BuildingStateEvent state_evt{};
    state_evt.instance_id = 77;
    state_evt.type = Core::BuildingType::kCannon;
    state_evt.new_state = Gameplay::BuildingState::kConstructing;
    state_evt.time_remaining = 5.0f;
    state_evt.total_build_time = 10.0f;
    manager->BroadcastBuildingStateChanged(state_evt);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.building_progress.size() == 1);
    const BuildingProgressStatus& progress = snapshot.building_progress[0];
    assert(progress.entity_id == 77);
    assert(progress.building_type == Core::BuildingType::kCannon);
    assert(progress.state == Gameplay::BuildingState::kConstructing);
    assert(progress.time_remaining == 5.0f);
    assert(progress.total_build_time == 10.0f);

    Gameplay::EntityDestroyEvent destroy{};
    destroy.instance_id = 77;
    manager->BroadcastEntityDestroyed(destroy);

    snapshot = store.GetSnapshot();
    assert(snapshot.building_progress.empty());

    store.Detach(manager);
}

void TestUiStateStoreBuildingProgressOutOfOrder() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::BuildingStateEvent state_evt{};
    state_evt.instance_id = 21;
    state_evt.type = Core::BuildingType::kBarracks;
    state_evt.new_state = Gameplay::BuildingState::kConstructing;
    state_evt.time_remaining = 4.0f;
    state_evt.total_build_time = 8.0f;
    manager->BroadcastBuildingStateChanged(state_evt);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.building_progress.size() == 1);
    assert(snapshot.building_progress[0].entity_id == 21);

    Gameplay::EntityDestroyEvent destroy{};
    destroy.instance_id = 21;
    manager->BroadcastEntityDestroyed(destroy);
    snapshot = store.GetSnapshot();
    assert(snapshot.building_progress.empty());

    store.Detach(manager);
}

void TestUiStateStoreBattleStateAndMode() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    store.SetMode(UiMode::kAttack);

    Gameplay::BattleStartEvent start_evt{};
    start_evt.time_limit_seconds = 180;
    manager->BroadcastBattleStarted(start_evt);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.mode == UiMode::kAttack);
    assert(snapshot.battle.in_battle);
    assert(snapshot.battle.time_limit_seconds == 180);
    assert(!snapshot.battle.has_battle_end);

    Gameplay::BattleEndEvent end_evt{};
    end_evt.result = Gameplay::BattleResult::kDefeat;
    end_evt.stars_earned = 1;
    end_evt.gold_stolen = 50;
    manager->BroadcastBattleEnded(end_evt);

    snapshot = store.GetSnapshot();
    assert(!snapshot.battle.in_battle);
    assert(snapshot.battle.has_battle_end);
    assert(snapshot.battle.last_battle_end.result == Gameplay::BattleResult::kDefeat);
    assert(snapshot.battle.last_battle_end.stars_earned == 1);
    assert(snapshot.battle.last_battle_end.gold_stolen == 50);

    store.Detach(manager);
}

void TestUiStateStoreBattleRepeatedStart() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::BattleStartEvent start_evt{};
    start_evt.time_limit_seconds = 120;
    manager->BroadcastBattleStarted(start_evt);
    start_evt.time_limit_seconds = 90;
    manager->BroadcastBattleStarted(start_evt);

    UiStateSnapshot snapshot = store.GetSnapshot();
    assert(snapshot.battle.in_battle);
    assert(snapshot.battle.time_limit_seconds == 90);

    store.Detach(manager);
}

void TestUiPresentationBindingRenderPlan() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::ResourceUpdateEvent gold{};
    gold.resource_type = "Gold";
    gold.current_amount = 150;
    gold.max_capacity = 300;
    manager->BroadcastResourceChange(gold);

    Gameplay::ResourceUpdateEvent elixir{};
    elixir.resource_type = "Elixir";
    elixir.current_amount = 90;
    elixir.max_capacity = 200;
    manager->BroadcastResourceChange(elixir);

    Gameplay::BuildingStateEvent progress_evt{};
    progress_evt.instance_id = 9;
    progress_evt.type = Core::BuildingType::kBarracks;
    progress_evt.new_state = Gameplay::BuildingState::kConstructing;
    progress_evt.time_remaining = 3.0f;
    progress_evt.total_build_time = 10.0f;
    manager->BroadcastBuildingStateChanged(progress_evt);

    UiPresentationBinding binding;
    UiRenderPlan plan = binding.BuildRenderPlan(store.GetSnapshot());
    assert(plan.mode == UiMode::kMenu);
    assert(plan.hud_items.size() == 3);
    assert(plan.hud_items[0].icon_asset == plan.assets.gold_icon);
    assert(plan.hud_items[0].label_text == "150/300");
    assert(plan.hud_items[1].icon_asset == plan.assets.elixir_icon);
    assert(plan.hud_items[1].label_text == "90/200");
    assert(plan.hud_items[2].icon_asset == plan.assets.gem_icon);
    assert(plan.hud_items[2].label_text == "0/0");
    assert(plan.menu_panel.start_button_asset == plan.assets.menu_start_button);
    assert(!plan.battle_panel.show_countdown);
    assert(plan.battle_panel.star_strip_asset.empty());

    assert(plan.building_overlays.size() == 1);
    const BuildingOverlayRender& overlay = plan.building_overlays[0];
    assert(overlay.entity_id == 9);
    assert(overlay.progress_bar_asset == plan.assets.build_progress_bar);
    assert(std::fabs(overlay.progress_ratio - 0.7f) < 0.0001f);

    Gameplay::BattleStartEvent start_evt{};
    start_evt.time_limit_seconds = 150;
    manager->BroadcastBattleStarted(start_evt);
    plan = binding.BuildRenderPlan(store.GetSnapshot());
    assert(plan.battle_panel.show_countdown);
    assert(plan.battle_panel.countdown_seconds == 150);
    assert(!plan.battle_panel.show_results);

    Gameplay::BattleEndEvent end_evt{};
    end_evt.result = Gameplay::BattleResult::kVictory;
    end_evt.stars_earned = 2;
    manager->BroadcastBattleEnded(end_evt);
    plan = binding.BuildRenderPlan(store.GetSnapshot());
    assert(plan.battle_panel.show_results);
    assert(plan.battle_panel.background_asset == plan.assets.victory_background);
    assert(plan.battle_panel.star_strip_asset == plan.assets.star_two);
    assert(!plan.build_palette_items.empty());
    assert(!plan.matchmaking.cost_text.empty());

    store.Detach(manager);
}

void TestUiPresentationBindingDefaultHudAndMenu() {
    UiPresentationBinding binding;
    UiStateSnapshot snapshot{};
    UiRenderPlan plan = binding.BuildRenderPlan(snapshot);
    assert(plan.hud_items.size() == 3);
    assert(plan.hud_items[0].icon_asset == plan.assets.gold_icon);
    assert(plan.hud_items[0].label_text == "0/0");
    assert(plan.hud_items[1].icon_asset == plan.assets.elixir_icon);
    assert(plan.hud_items[1].label_text == "0/0");
    assert(plan.hud_items[2].icon_asset == plan.assets.gem_icon);
    assert(plan.hud_items[2].label_text == "0/0");
    assert(plan.menu_panel.background_asset == plan.assets.menu_background);
    assert(plan.menu_panel.start_button_asset == plan.assets.menu_start_button);
}

void TestUiPresentationBindingLootAvailability() {
    UiPresentationBinding binding;
    UiStateSnapshot snapshot{};
    snapshot.loot_availability.gold_available = 250;
    snapshot.loot_availability.elixir_available = 150;
    snapshot.loot_availability.trophies_available = 12;
    UiRenderPlan plan = binding.BuildRenderPlan(snapshot);
    assert(plan.loot_items.size() == 1);
    assert(plan.loot_items[0].label_text == "Loot 250g 150e 12t");
}

void TestUiPresentationBindingBuildingOverlayOrdering() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::BuildingStateEvent progress_a{};
    progress_a.instance_id = 5;
    progress_a.type = Core::BuildingType::kBarracks;
    progress_a.new_state = Gameplay::BuildingState::kConstructing;
    progress_a.time_remaining = 3.0f;
    progress_a.total_build_time = 6.0f;
    manager->BroadcastBuildingStateChanged(progress_a);

    Gameplay::BuildingStateEvent progress_b{};
    progress_b.instance_id = 3;
    progress_b.type = Core::BuildingType::kCannon;
    progress_b.new_state = Gameplay::BuildingState::kConstructing;
    progress_b.time_remaining = 2.0f;
    progress_b.total_build_time = 4.0f;
    manager->BroadcastBuildingStateChanged(progress_b);

    UiPresentationBinding binding;
    UiRenderPlan plan = binding.BuildRenderPlan(store.GetSnapshot());
    assert(plan.building_overlays.size() == 2);
    assert(plan.building_overlays[0].entity_id == 3);
    assert(plan.building_overlays[1].entity_id == 5);

    store.Detach(manager);
}

void TestUiPresentationBindingBattleDefeatAssets() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::BattleEndEvent end_evt{};
    end_evt.result = Gameplay::BattleResult::kDefeat;
    end_evt.stars_earned = 0;
    manager->BroadcastBattleEnded(end_evt);

    UiPresentationBinding binding;
    UiRenderPlan plan = binding.BuildRenderPlan(store.GetSnapshot());
    assert(plan.battle_panel.show_results);
    assert(plan.battle_panel.background_asset == plan.assets.defeat_background);
    assert(plan.battle_panel.star_strip_asset == plan.assets.star_zero);

    store.Detach(manager);
}

void TestUiPresentationBindingDeploymentRender() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    UiStateStore store(/*local_owner_id=*/0);
    store.Attach(manager);

    Gameplay::TroopCountUpdateEvent count_evt{};
    count_evt.owner_id = 0;
    count_evt.troop_type = Core::TroopType::kArcher;
    count_evt.remaining_count = 4;
    manager->BroadcastTroopCountUpdated(count_evt);

    Gameplay::DeploymentSelectionEvent selection_evt{};
    selection_evt.owner_id = 0;
    selection_evt.has_selection = true;
    selection_evt.troop_type = Core::TroopType::kArcher;
    manager->BroadcastDeploymentSelectionChanged(selection_evt);

    UiPresentationBinding binding;
    UiRenderPlan plan = binding.BuildRenderPlan(store.GetSnapshot());
    assert(plan.deployment_items.size() == 1);
    assert(plan.deployment_items[0].troop_type == Core::TroopType::kArcher);
    assert(plan.deployment_items[0].remaining_count == 4);
    assert(plan.deployment_items[0].is_selected);
    assert(plan.troop_inspect.show);

    store.Detach(manager);
}

void TestGameStageSceneUiBinding() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    Integration::BattleLaunchParams params{"maps/test_ui.tmx", 99};
    cocos2d::Scene* scene = flow->StartGame(params);
    auto* game_scene = dynamic_cast<GameStageScene*>(scene);
    assert(game_scene != nullptr);

    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();

    Gameplay::ResourceUpdateEvent gold{};
    gold.resource_type = "Gold";
    gold.current_amount = 200;
    gold.max_capacity = 400;
    manager->BroadcastResourceChange(gold);

    Gameplay::BuildingStateEvent build{};
    build.instance_id = 31;
    build.type = Core::BuildingType::kBarracks;
    build.new_state = Gameplay::BuildingState::kConstructing;
    build.time_remaining = 6.0f;
    build.total_build_time = 12.0f;
    manager->BroadcastBuildingStateChanged(build);

    game_scene->ForceRenderForTest();
    UiRenderPlan plan = game_scene->GetLastRenderPlanForTest();
    assert(plan.hud_items.size() >= 2);
    assert(plan.hud_items[0].label_text == "200/400");
    assert(plan.building_overlays.size() == 1);

    Gameplay::TroopCountUpdateEvent irrelevant{};
    irrelevant.owner_id = 7;
    irrelevant.troop_type = Core::TroopType::kArcher;
    irrelevant.remaining_count = 5;
    manager->BroadcastTroopCountUpdated(irrelevant);

    game_scene->ForceRenderForTest();
    plan = game_scene->GetLastRenderPlanForTest();
    assert(plan.building_overlays.size() == 1);

    Gameplay::BattleStartEvent start{};
    start.time_limit_seconds = 120;
    manager->BroadcastBattleStarted(start);

    game_scene->ForceRenderForTest();
    plan = game_scene->GetLastRenderPlanForTest();
    assert(plan.battle_panel.show_countdown);
    assert(plan.battle_panel.countdown_seconds == 120);

    Gameplay::BattleEndEvent end{};
    end.result = Gameplay::BattleResult::kVictory;
    end.stars_earned = 3;
    manager->BroadcastBattleEnded(end);

    game_scene->ForceRenderForTest();
    plan = game_scene->GetLastRenderPlanForTest();
    assert(plan.battle_panel.show_results);
    assert(plan.battle_panel.star_strip_asset == plan.assets.star_three);
    assert(!plan.build_palette_items.empty());

    scene->onExit();
    delete scene;
    delete flow;
}

void TestGameStageSceneRenderPlanStability() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    Integration::BattleLaunchParams params{"maps/test_ui.tmx", 99};
    cocos2d::Scene* scene = flow->StartGame(params);
    auto* game_scene = dynamic_cast<GameStageScene*>(scene);
    assert(game_scene != nullptr);

    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();

    Gameplay::BuildingStateEvent build{};
    build.instance_id = 31;
    build.type = Core::BuildingType::kBarracks;
    build.new_state = Gameplay::BuildingState::kConstructing;
    build.time_remaining = 6.0f;
    build.total_build_time = 12.0f;
    manager->BroadcastBuildingStateChanged(build);

    game_scene->ForceRenderForTest();
    assert(game_scene->GetProgressBarCountForTest() == 1);
    assert(game_scene->HasProgressBarForEntityForTest(31));

    game_scene->ForceRenderForTest();
    assert(game_scene->GetProgressBarCountForTest() == 1);

    Gameplay::EntityDestroyEvent destroy{};
    destroy.instance_id = 31;
    manager->BroadcastEntityDestroyed(destroy);
    game_scene->ForceRenderForTest();
    assert(game_scene->GetProgressBarCountForTest() == 0);

    scene->onExit();
    delete scene;
    delete flow;
}

void TestGameStageSceneInputWiring() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    Integration::BattleLaunchParams params{"maps/test_ui.tmx", 99};
    cocos2d::Scene* scene = flow->StartGame(params);
    auto* game_scene = dynamic_cast<GameStageScene*>(scene);
    assert(game_scene != nullptr);

    game_scene->SetModeForTest(UiMode::kBuild);
    bool consumed = game_scene->HandleTap(cocos2d::Vec2(120.0f, 320.0f));
    assert(consumed);
    assert(game_scene->IsBuildPanelOpenForTest());

    consumed = game_scene->HandleTap(cocos2d::Vec2(120.0f, 280.0f));
    assert(consumed);
    assert(game_scene->GetLastActionForTest() == "save_base");

    consumed = game_scene->HandleTap(cocos2d::Vec2(120.0f, 250.0f));
    assert(consumed);
    assert(game_scene->GetLastActionForTest() == "load_base");

    game_scene->SetModeForTest(UiMode::kAttack);
    consumed = game_scene->HandleTap(cocos2d::Vec2(680.0f, 320.0f));
    assert(consumed);
    assert(game_scene->IsAttackPanelOpenForTest());

    scene->onExit();
    delete scene;
    delete flow;
}

void TestGameStageSceneMatchmakingButton() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    Integration::BattleLaunchParams params{"maps/test_ui.tmx", 99};
    cocos2d::Scene* scene = flow->StartGame(params);
    auto* game_scene = dynamic_cast<GameStageScene*>(scene);
    assert(game_scene != nullptr);

    EconomySystem* economy = EconomySystem::GetInstance();
    economy->Reset();
    ResourceCost matchmaking_cost = CostQuery::GetInstance()->GetMatchmakingCost();
    economy->AddGold(matchmaking_cost.gold);

    game_scene->SetModeForTest(UiMode::kAttack);
    bool consumed = game_scene->HandleTap(cocos2d::Vec2(680.0f, 280.0f));
    assert(consumed);
    assert(game_scene->GetLastActionForTest() == "find_match");
    assert(economy->GetCurrentGold() == 0);

    scene->onExit();
    delete scene;
    delete flow;
}

void TestGameStageSceneDetachStopsUpdates() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    Integration::BattleLaunchParams params{"maps/test_ui.tmx", 99};
    cocos2d::Scene* scene = flow->StartGame(params);
    auto* game_scene = dynamic_cast<GameStageScene*>(scene);
    assert(game_scene != nullptr);

    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    Gameplay::ResourceUpdateEvent gold{};
    gold.resource_type = "Gold";
    gold.current_amount = 100;
    gold.max_capacity = 200;
    manager->BroadcastResourceChange(gold);
    game_scene->ForceRenderForTest();
    UiRenderPlan before = game_scene->GetLastRenderPlanForTest();
    std::string before_label = before.hud_items.empty() ? "" : before.hud_items[0].label_text;

    scene->onExit();
    gold.current_amount = 200;
    gold.max_capacity = 400;
    manager->BroadcastResourceChange(gold);
    game_scene->ForceRenderForTest();
    UiRenderPlan after = game_scene->GetLastRenderPlanForTest();
    std::string after_label = after.hud_items.empty() ? "" : after.hud_items[0].label_text;
    assert(before_label == after_label);

    delete scene;
    delete flow;
}

void TestResultsSceneReplayButton() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    Gameplay::BattleEndEvent summary{};
    Integration::ResultsScreenData results{summary, 10, 1, 1};
    cocos2d::Scene* scene = flow->ShowResults(results);
    auto* results_scene = static_cast<ResultsScene*>(scene);

    bool consumed = results_scene->HandleTap(cocos2d::Vec2(400.0f, 140.0f));
    assert(consumed);
    assert(results_scene->GetLastResultsActionForTest() == "replay");

    delete scene;
    delete flow;
}

void TestAudioManagerPlaysBattleStartMusic() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    AudioSinkMock sink;
    sink.Reset();

    AudioManager audio_manager(manager, &sink);

    Gameplay::BattleStartEvent battle_start{};
    manager->BroadcastBattleStarted(battle_start);

    assert(!sink.GetPlays().empty());
    const RecordedClip& play = sink.GetPlays().back();
    assert(play.clip_id == "Resources/music/UI effects/start_up.mp3");
}

void TestAudioManagerNoSoundWithoutEvents() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    AudioSinkMock sink;
    sink.Reset();

    AudioManager audio_manager(manager, &sink);

    assert(sink.GetPlays().empty());
}

void TestAudioManagerIgnoresUnmappedEvents() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    AudioSinkMock sink;
    sink.Reset();

    AudioManager audio_manager(manager, &sink);

    Gameplay::ResourceUpdateEvent resource_evt{};
    resource_evt.resource_type = "gold";
    manager->BroadcastResourceChange(resource_evt);

    assert(sink.GetPlays().empty());
}

void TestAudioManagerMenuMusicAndClick() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    AudioSinkMock sink;
    sink.Reset();

    AudioManager audio_manager(manager, &sink);
    audio_manager.PlayMenuMusic();
    audio_manager.PlayUiClick();

    const std::vector<RecordedClip>& plays = sink.GetPlays();
    assert(plays.size() == 2);
    assert(plays[0].clip_id == "Resources/music/Background Music/Home music 1.mp3");
    assert(plays[0].loop);
    assert(plays[1].clip_id == "Resources/music/UI effects/ui_click.mp3");
}

int main() {
    TestTilePlacementController();
    TestInputRouter();
    TestInputRouterUiConsumesSkipsMap();
    TestInputRouterUiIgnoresRoutesToMap();
    TestInputRouterWithoutMapIsSafe();
    TestEconomySystem();
    TestMatchmakingCost();
    TestTroopCommandServiceMock();
    TestHealthComp();
    TestUnitAndBuilding();
    TestSceneFlowService();
    TestSceneFlowMockDelegation();
    TestPlayerIdentityService();
    TestVerticalSliceSceneFlow();
    TestMenuSceneMapSelectionPropagation();
    TestMenuSceneInputWiring();
    TestBattleEndPayload();
    TestLootAvailabilityPayload();
    TestDeploymentContractExtensions();
    TestGameServicesSmoke();
    TestHudStateUpdates();
    TestSelectionState();
    TestHealthBarState();
    TestUiStateStoreSnapshot();
    TestUiStateStoreSelectionAndDestroy();
    TestUiStateStoreHealthBars();
    TestUiStateStoreDeploymentBar();
    TestUiStateStoreBuildingProgress();
    TestUiStateStoreBuildingProgressOutOfOrder();
    TestUiStateStoreBattleStateAndMode();
    TestUiStateStoreBattleRepeatedStart();
    TestUiPresentationBindingRenderPlan();
    TestUiPresentationBindingDefaultHudAndMenu();
    TestUiPresentationBindingLootAvailability();
    TestUiPresentationBindingBuildingOverlayOrdering();
    TestUiPresentationBindingBattleDefeatAssets();
    TestUiPresentationBindingDeploymentRender();
    TestGameStageSceneUiBinding();
    TestGameStageSceneRenderPlanStability();
    TestGameStageSceneInputWiring();
    TestGameStageSceneMatchmakingButton();
    TestGameStageSceneDetachStopsUpdates();
    TestResultsSceneReplayButton();
    TestAudioManagerPlaysBattleStartMusic();
    TestAudioManagerNoSoundWithoutEvents();
    TestAudioManagerIgnoresUnmappedEvents();
    TestAudioManagerMenuMusicAndClick();
    return 0;
}
