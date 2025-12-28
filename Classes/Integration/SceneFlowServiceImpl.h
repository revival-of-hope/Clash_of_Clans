#ifndef CLASSES_INTEGRATION_SCENEFLOWSERVICEIMPL_H_
#define CLASSES_INTEGRATION_SCENEFLOWSERVICEIMPL_H_

#include "Classes/Contract/Integration/SceneFlowService.h"

namespace Integration {

class SceneFlowServiceImpl : public SceneFlowService {
 public:
  SceneFlowServiceImpl() = default;
  ~SceneFlowServiceImpl() override = default;

  cocos2d::Scene* CreateBootScene() override;
  cocos2d::Scene* ShowMenuScene() override;
  cocos2d::Scene* StartGame(const BattleLaunchParams& params) override;
  cocos2d::Scene* ShowResults(const ResultsScreenData& results) override;
  SceneStage GetCurrentStage() const override;
  BattleLaunchParams GetLastLaunchParams() const override;
  ResultsScreenData GetLastResults() const override;

 private:
  SceneStage stage_ = SceneStage::kBoot;
  BattleLaunchParams last_launch_params_{};
  ResultsScreenData last_results_{};
};

// Production factory used by AppDelegate and non-mock builds.
SceneFlowService* CreateSceneFlowService();

}  // namespace Integration

#endif  // CLASSES_INTEGRATION_SCENEFLOWSERVICEIMPL_H_
