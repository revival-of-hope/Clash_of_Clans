#include "Scenes/GameScene.h"

#include "Classes/Contract/Engine/MapLayer.h"
#include "Classes/Contract/Gameplay/Building.h"
#include "Classes/Contract/Gameplay/CombatResolver.h"
#include "Classes/Contract/Gameplay/EconomySystem.h"
#include "Classes/Contract/Gameplay/Unit.h"
#include "Core/GameConstants.h"

namespace {

cocos2d::Vec2 GridToPixel(int x, int y) {
    return cocos2d::Vec2(x * Core::kTileWidth + (Core::kTileWidth / 2.0f),
                         y * Core::kTileHeight + (Core::kTileHeight / 2.0f));
}

}  // namespace

cocos2d::Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto* event_manager = Gameplay::GameEventManager::GetInstance();
    if (event_manager) {
        event_manager->AddListener(this);
    }

    auto* game_layer = cocos2d::Node::create();
    addChild(game_layer, static_cast<int>(Core::ZOrder::kGround));

    auto* ui_layer = cocos2d::Node::create();
    addChild(ui_layer, static_cast<int>(Core::ZOrder::kUiHud));

    auto* map_layer = MapLayer::create("maps/test2.tmx");
    if (map_layer) {
        game_layer->addChild(map_layer);
    }

    auto* combat = CombatResolver::GetInstance();
    if (combat) {
        combat->Initialize(game_layer);
    }

    auto* economy = EconomySystem::GetInstance();
    if (economy) {
        economy->Reset();
    }

    auto* town_hall = Building::create(Core::BuildingType::kTownHall, 1, 0);
    if (town_hall) {
        town_hall->setPosition(GridToPixel(10, 10));
        game_layer->addChild(town_hall, static_cast<int>(Core::ZOrder::kBuildingBase));
    }

    auto* cannon = Building::create(Core::BuildingType::kCannon, 1, 0);
    if (cannon) {
        cannon->setPosition(GridToPixel(14, 10));
        game_layer->addChild(cannon, static_cast<int>(Core::ZOrder::kBuildingBase));
    }

    if (economy && town_hall) {
        cocos2d::Vector<Building*> buildings;
        buildings.pushBack(town_hall);
        if (cannon) {
            buildings.pushBack(cannon);
        }
        economy->RecalculateLimits(buildings);
    }

    if (economy && economy->AddTroopPopulation(1)) {
        auto* unit = Unit::create(Core::TroopType::kBarbarian, 1, 0);
        if (unit) {
            unit->setPosition(GridToPixel(8, 8));
            game_layer->addChild(unit, static_cast<int>(Core::ZOrder::kUnits));
        }
    }

    status_label_ = cocos2d::Label::createWithSystemFont(
        "Battle running... (waiting for OnBattleEnded)", "Arial", 18);
    if (status_label_) {
        status_label_->setPosition(cocos2d::Vec2(200.0f, 560.0f));
        ui_layer->addChild(status_label_);
    }

    // TODO(DevC): Replace label update with actual battle results once Gameplay emits events.
    return true;
}

void GameScene::onExit() {
    auto* event_manager = Gameplay::GameEventManager::GetInstance();
    if (event_manager) {
        event_manager->RemoveListener(this);
    }
    Scene::onExit();
}

void GameScene::OnBattleEnded(const Gameplay::BattleEndEvent& evt) {
    if (!status_label_) {
        return;
    }

    const char* result_text = "Victory";
    if (evt.result == Gameplay::BattleResult::kDefeat) {
        result_text = "Defeat";
    } else if (evt.result == Gameplay::BattleResult::kTimeOut) {
        result_text = "Time Out";
    }

    status_label_->setString(cocos2d::StringUtils::format(
        "Battle ended: %s (%d%%, %d stars)", result_text, evt.destruction_percent,
        evt.stars_earned));
}
