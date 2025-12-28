#ifndef CLASSES_SCENES_BOOTSCENE_H_
#define CLASSES_SCENES_BOOTSCENE_H_

#include <cassert>

#include "Classes/Contract/Integration/SceneFlowService.h"
#include "cocos2d.h"

class BootScene : public cocos2d::Scene {
 public:
  static BootScene* Create(Integration::SceneFlowService* scene_flow);

  bool init() override;

 private:
  explicit BootScene(Integration::SceneFlowService* scene_flow);

  void VerifyStage() const;

  Integration::SceneFlowService* scene_flow_ = nullptr;
};

#endif  // CLASSES_SCENES_BOOTSCENE_H_
