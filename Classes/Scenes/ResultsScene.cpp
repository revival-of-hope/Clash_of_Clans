#include "Classes/Scenes/ResultsScene.h"

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
  return screen_pos.x >= bounds.x && screen_pos.x <= bounds.x + bounds.width &&
         screen_pos.y >= bounds.y && screen_pos.y <= bounds.y + bounds.height;
}

cocos2d::Rect ResultsScene::GetReplayButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(400.0f, 140.0f), 220.0f, 60.0f);
}

void ResultsScene::RecordAction(const std::string& action) {
  last_action_ = action;
}
