#ifndef CLASSES_SCENES_MENUSCENE_H_
#define CLASSES_SCENES_MENUSCENE_H_

#include <cassert>

#include "Classes/Contract/Integration/SceneFlowService.h"
#include "Classes/Managers/LevelManager.h"
#include "cocos2d.h"

class MenuScene : public cocos2d::Scene {
 public:
  static MenuScene* Create(Integration::SceneFlowService* scene_flow);

  bool init() override;

  void SelectMapA();
  void SelectMapB();

  cocos2d::Scene* StartSelectedMap();

 private:
  explicit MenuScene(Integration::SceneFlowService* scene_flow);

  void VerifyStage() const;

  Integration::SceneFlowService* scene_flow_ = nullptr;
  LevelManager* level_manager_ = nullptr;
};

#endif  // CLASSES_SCENES_MENUSCENE_H_
