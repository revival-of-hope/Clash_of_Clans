#include "Classes/Scenes/MenuScene.h"

MenuScene::MenuScene(Integration::SceneFlowService* scene_flow)
    : scene_flow_(scene_flow), level_manager_(LevelManager::GetInstance()) {}

MenuScene* MenuScene::Create(Integration::SceneFlowService* scene_flow) {
  auto* scene = new MenuScene(scene_flow);
  if (scene && scene->init()) {
    scene->VerifyStage();
    return scene;
  }
  delete scene;
  return nullptr;
}

bool MenuScene::init() {
  if (level_manager_) {
    level_manager_->SelectMapA();
  }
  return cocos2d::Scene::init();
}

void MenuScene::SelectMapA() {
  if (!level_manager_) {
    return;
  }
  level_manager_->SelectMapA();
}

void MenuScene::SelectMapB() {
  if (!level_manager_) {
    return;
  }
  level_manager_->SelectMapB();
}

cocos2d::Scene* MenuScene::StartSelectedMap() {
  if (!scene_flow_ || !level_manager_) {
    return nullptr;
  }
  Integration::BattleLaunchParams params{level_manager_->GetSelectedMapPath(),
                                         level_manager_->GetSeed()};
  return scene_flow_->StartGame(params);
}

void MenuScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kMenu);
}
