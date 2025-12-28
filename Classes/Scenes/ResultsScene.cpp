#include "Classes/Scenes/ResultsScene.h"

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

bool ResultsScene::init() { return cocos2d::Scene::init(); }

void ResultsScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kResults);
}
