#include "Classes/Scenes/GameStageScene.h"

GameStageScene::GameStageScene(Integration::SceneFlowService* scene_flow,
                               const Integration::BattleLaunchParams& launch_params)
    : scene_flow_(scene_flow), launch_params_(launch_params) {}

GameStageScene* GameStageScene::Create(
    Integration::SceneFlowService* scene_flow,
    const Integration::BattleLaunchParams& launch_params) {
  auto* scene = new GameStageScene(scene_flow, launch_params);
  if (scene && scene->init()) {
    scene->VerifyStage();
    return scene;
  }
  delete scene;
  return nullptr;
}

bool GameStageScene::init() { return cocos2d::Scene::init(); }

void GameStageScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kGame);
}
