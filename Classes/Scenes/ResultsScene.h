#ifndef CLASSES_SCENES_RESULTSSCENE_H_
#define CLASSES_SCENES_RESULTSSCENE_H_

#include <cassert>
#include <string>

#include "Classes/Contract/Integration/SceneFlowService.h"
#include "cocos2d.h"

class ResultsScene : public cocos2d::Scene {
 public:
  static ResultsScene* Create(Integration::SceneFlowService* scene_flow,
                              const Integration::ResultsScreenData& results);

  bool init() override;

  const Integration::ResultsScreenData& GetResults() const { return results_; }
  bool HandleTap(const cocos2d::Vec2& screen_pos);
  const std::string& GetLastResultsActionForTest() const { return last_action_; }

 private:
  ResultsScene(Integration::SceneFlowService* scene_flow,
               const Integration::ResultsScreenData& results);

  void VerifyStage() const;
  bool HitTest(const cocos2d::Rect& bounds, const cocos2d::Vec2& screen_pos) const;
  cocos2d::Rect GetReplayButtonBounds() const;
  void RecordAction(const std::string& action);

  Integration::SceneFlowService* scene_flow_ = nullptr;
  Integration::ResultsScreenData results_{};
  std::string last_action_{};
};

#endif  // CLASSES_SCENES_RESULTSSCENE_H_
