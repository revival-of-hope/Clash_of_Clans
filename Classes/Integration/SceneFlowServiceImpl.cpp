#include "Classes/Integration/SceneFlowServiceImpl.h"

#include "Classes/Scenes/BootScene.h"
#include "Classes/Scenes/GameStageScene.h"
#include "Classes/Scenes/MenuScene.h"
#include "Classes/Scenes/ResultsScene.h"

namespace Integration {

cocos2d::Scene* SceneFlowServiceImpl::CreateBootScene() {
  stage_ = SceneStage::kBoot;
  return BootScene::Create(this);
}

cocos2d::Scene* SceneFlowServiceImpl::ShowMenuScene() {
  stage_ = SceneStage::kMenu;
  return MenuScene::Create(this);
}

cocos2d::Scene* SceneFlowServiceImpl::StartGame(
    const BattleLaunchParams& params) {
  last_launch_params_ = params;
  stage_ = SceneStage::kGame;
  return GameStageScene::Create(this, params);
}

cocos2d::Scene* SceneFlowServiceImpl::ShowResults(
    const ResultsScreenData& results) {
  last_results_ = results;
  stage_ = SceneStage::kResults;
  return ResultsScene::Create(this, results);
}

SceneStage SceneFlowServiceImpl::GetCurrentStage() const { return stage_; }

BattleLaunchParams SceneFlowServiceImpl::GetLastLaunchParams() const {
  return last_launch_params_;
}

ResultsScreenData SceneFlowServiceImpl::GetLastResults() const {
  return last_results_;
}

SceneFlowService* CreateSceneFlowService() {
  return new SceneFlowServiceImpl();
}

}  // namespace Integration
