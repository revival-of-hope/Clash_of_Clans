#include "Classes/Scenes/BootScene.h"

BootScene::BootScene(Integration::SceneFlowService* scene_flow)
    : scene_flow_(scene_flow) {}

BootScene* BootScene::Create(Integration::SceneFlowService* scene_flow) {
  auto* scene = new BootScene(scene_flow);
  if (scene && scene->init()) {
    scene->VerifyStage();
    return scene;
  }
  delete scene;
  return nullptr;
}

bool BootScene::init() { return cocos2d::Scene::init(); }

void BootScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kBoot);
}
