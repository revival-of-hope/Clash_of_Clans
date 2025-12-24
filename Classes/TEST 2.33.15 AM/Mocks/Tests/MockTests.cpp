#include <cassert>
#include <cmath>

#include "Contracts/Engine/InputRouter.h"
#include "Contracts/Engine/MapLayer.h"
#include "Contracts/Engine/TileHighlighter.h"
#include "Contracts/Engine/TilePlacementController.h"
#include "Core/GameConstants.h"
#include "Contracts/Gameplay/Building.h"
#include "Contracts/Gameplay/EconomySystem.h"
#include "Contracts/Gameplay/GameEvents.h"
#include "Contracts/Gameplay/HealthComp.h"
#include "Contracts/Gameplay/Unit.h"
#include "Contracts/Integration/SceneFlowService.h"

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
    const float tile_size = static_cast<float>(Core::kTileWidth);
    routed = router.RoutePointerMove(cocos2d::Vec2(tile_size, tile_size));
    assert(routed);
    state = router.GetLastRouteState();
    assert(!state.ui_consumed);
    const float expected_center = tile_size * 1.0f + tile_size / 2.0f;
    assert(std::fabs(state.world_pos.x - expected_center) < 0.001f);
    assert(std::fabs(state.world_pos.y - expected_center) < 0.001f);
}

void TestSceneFlowService() {
    Integration::SceneFlowService* flow = Integration::CreateSceneFlowService();
    flow->CreateBootScene();
    assert(flow->GetCurrentStage() == Integration::SceneStage::kBoot);
    flow->ShowMenuScene();
    assert(flow->GetCurrentStage() == Integration::SceneStage::kMenu);
    Integration::BattleLaunchParams launch{"maps/test.tmx", 42};
    flow->StartGame(launch);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kGame);
    assert(flow->GetLastLaunchParams().map_path == "maps/test.tmx");
    assert(flow->GetLastLaunchParams().seed == 42);

    Gameplay::BattleEndEvent summary;
    summary.stars_earned = 3;
    Integration::ResultsScreenData results{summary, 180, 5, 2};
    flow->ShowResults(results);
    assert(flow->GetCurrentStage() == Integration::SceneStage::kResults);
    assert(flow->GetLastResults().elapsed_seconds == 180);
    delete flow;
}

void TestBattleEndPayload() {
    Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
    Gameplay::BattleEndEvent evt;
    evt.result = Gameplay::BattleResult::kVictory;
    evt.stars_earned = 2;
    evt.destruction_percent = 75;
    evt.gold_stolen = 500;
    evt.elixir_stolen = 250;
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
}

void TestHealthComp() {
    HealthComp health;
    health.InitStats(100);
    assert(std::fabs(health.GetHealthPercentage() - 1.0f) < 0.0001f);
    assert(!health.IsDead());

    bool died = health.TakeDamage(40);
    assert(!died);
    assert(std::fabs(health.GetHealthPercentage() - 0.6f) < 0.0001f);

    died = health.TakeDamage(60);
    assert(died);
    assert(health.IsDead());
    assert(std::fabs(health.GetHealthPercentage() - 0.0f) < 0.0001f);
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

    Building* collector = Building::create(Core::BuildingType::kGoldMine, 1, 0);
    assert(collector->GetStoredResource() > 0);
}

int main() {
    TestTilePlacementController();
    TestInputRouter();
    TestEconomySystem();
    TestHealthComp();
    TestUnitAndBuilding();
    TestSceneFlowService();
    TestBattleEndPayload();
    return 0;
}
