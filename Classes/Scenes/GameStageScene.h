#ifndef CLASSES_SCENES_GAMESTAGESCENE_H_
#define CLASSES_SCENES_GAMESTAGESCENE_H_

#include <cassert>

#include "Classes/Contract/Integration/SceneFlowService.h"
#include "cocos2d.h"

class GameStageScene : public cocos2d::Scene {
 public:
  static GameStageScene* Create(Integration::SceneFlowService* scene_flow,
                                const Integration::BattleLaunchParams& launch_params);

  bool init() override;

  const Integration::BattleLaunchParams& GetLaunchParams() const {
    return launch_params_;
  }

 private:
  GameStageScene(Integration::SceneFlowService* scene_flow,
                 const Integration::BattleLaunchParams& launch_params);

  void VerifyStage() const;

  Integration::SceneFlowService* scene_flow_ = nullptr;
  Integration::BattleLaunchParams launch_params_{};
};

#endif  // CLASSES_SCENES_GAMESTAGESCENE_H_
