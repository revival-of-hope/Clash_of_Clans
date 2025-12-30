#include "Classes/Scenes/ResultsScene.h"

#include <string>

namespace {

cocos2d::Rect MakeButtonRect(const cocos2d::Vec2& center, float width, float height) {
  return cocos2d::Rect(center.x - width * 0.5f, center.y - height * 0.5f, width,
                       height);
}

}  // namespace

ResultsScene::ResultsScene(Integration::SceneFlowService* scene_flow,
                           const Integration::ResultsScreenData& results)
    : scene_flow_(scene_flow), results_(results) {}

ResultsScene* ResultsScene::Create(Integration::SceneFlowService* scene_flow,
                                   const Integration::ResultsScreenData& results) {
  auto* scene = new ResultsScene(scene_flow, results);
  if (scene && scene->init()) {
    scene->VerifyStage();
    return scene;
  }
  delete scene;
  return nullptr;
}

bool ResultsScene::init() {
  if (!cocos2d::Scene::init()) {
    return false;
  }

  std::string result_text = "Result";
  switch (results_.summary.result) {
    case Gameplay::BattleResult::kVictory:
      result_text = "Victory";
      break;
    case Gameplay::BattleResult::kDefeat:
      result_text = "Defeat";
      break;
    case Gameplay::BattleResult::kTimeOut:
      result_text = "Time Out";
      break;
  }

  auto* result_label =
      cocos2d::Label::createWithSystemFont(result_text, "Arial", 24);
  if (result_label) {
    result_label->setPosition(cocos2d::Vec2(400.0f, 420.0f));
    addChild(result_label);
  }

  auto* stars_label =
      cocos2d::Label::createWithSystemFont(
          "Stars: " + std::to_string(results_.summary.stars_earned), "Arial", 18);
  if (stars_label) {
    stars_label->setPosition(cocos2d::Vec2(400.0f, 380.0f));
    addChild(stars_label);
  }

  auto* loot_label =
      cocos2d::Label::createWithSystemFont(
          "Loot: " + std::to_string(results_.summary.gold_stolen) + "g " +
              std::to_string(results_.summary.elixir_stolen) + "e",
          "Arial", 18);
  if (loot_label) {
    loot_label->setPosition(cocos2d::Vec2(400.0f, 350.0f));
    addChild(loot_label);
  }

  auto* trophies_label =
      cocos2d::Label::createWithSystemFont(
          "Trophies: " + std::to_string(results_.summary.trophies_earned) +
              " (Total " + std::to_string(results_.summary.trophies_total) + ")",
          "Arial", 18);
  if (trophies_label) {
    trophies_label->setPosition(cocos2d::Vec2(400.0f, 320.0f));
    addChild(trophies_label);
  }

  auto* replay_label =
      cocos2d::Label::createWithSystemFont("Replay", "Arial", 20);
  if (replay_label) {
    replay_label->setPosition(cocos2d::Vec2(400.0f, 140.0f));
    addChild(replay_label);
  }

  return true;
}

void ResultsScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kResults);
}

bool ResultsScene::HandleTap(const cocos2d::Vec2& screen_pos) {
  if (HitTest(GetReplayButtonBounds(), screen_pos)) {
    RecordAction("replay");
    return true;
  }
  return false;
}

bool ResultsScene::HitTest(const cocos2d::Rect& bounds,
                           const cocos2d::Vec2& screen_pos) const {
#if USE_COCOS_ENGINE
  return screen_pos.x >= bounds.origin.x &&
         screen_pos.x <= bounds.origin.x + bounds.size.width &&
         screen_pos.y >= bounds.origin.y &&
         screen_pos.y <= bounds.origin.y + bounds.size.height;
#else
  return screen_pos.x >= bounds.x && screen_pos.x <= bounds.x + bounds.width &&
         screen_pos.y >= bounds.y && screen_pos.y <= bounds.y + bounds.height;
#endif
}

cocos2d::Rect ResultsScene::GetReplayButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(400.0f, 140.0f), 220.0f, 60.0f);
}

void ResultsScene::RecordAction(const std::string& action) {
  last_action_ = action;
}
