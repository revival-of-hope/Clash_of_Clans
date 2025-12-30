#ifndef CLASSES_SCENES_MENUSCENE_H_
#define CLASSES_SCENES_MENUSCENE_H_

#include <cassert>
#include <string>

#include "Classes/Integration/GameServices.h"
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
  bool HandleTap(const cocos2d::Vec2& screen_pos);
  const std::string& GetLastMenuActionForTest() const { return last_action_; }

 private:
  explicit MenuScene(Integration::SceneFlowService* scene_flow);

  void VerifyStage() const;
  bool HitTest(const cocos2d::Rect& bounds, const cocos2d::Vec2& screen_pos) const;
  void RecordAction(const std::string& action);
  cocos2d::Rect GetStartButtonBounds() const;
  cocos2d::Rect GetMapAButtonBounds() const;
  cocos2d::Rect GetMapBButtonBounds() const;
  cocos2d::Rect GetLeaguesButtonBounds() const;
  cocos2d::Rect GetReplaysButtonBounds() const;

  Integration::SceneFlowService* scene_flow_ = nullptr;
  LevelManager* level_manager_ = nullptr;
  AudioManager* audio_manager_ = nullptr;
  std::string last_action_{};
};

#endif  // CLASSES_SCENES_MENUSCENE_H_
