#ifndef CLASSES_SCENES_GAMESTAGESCENE_H_
#define CLASSES_SCENES_GAMESTAGESCENE_H_

#include <cassert>
#include <string>
#include <unordered_map>

#include "Classes/Contract/Integration/SceneFlowService.h"
#include "Classes/UI/UiStateModels.h"
#include "cocos2d.h"

class GameStageScene : public cocos2d::Scene {
 public:
  static GameStageScene* Create(Integration::SceneFlowService* scene_flow,
                                const Integration::BattleLaunchParams& launch_params);

  bool init() override;
  ~GameStageScene() override;
  void onExit() override;

  const Integration::BattleLaunchParams& GetLaunchParams() const {
    return launch_params_;
  }

  bool HandleTap(const cocos2d::Vec2& screen_pos);
  void SetModeForTest(UiMode mode) { ui_state_store_.SetMode(mode); }
  bool IsBuildPanelOpenForTest() const { return build_panel_open_; }
  bool IsAttackPanelOpenForTest() const { return attack_panel_open_; }
  const std::string& GetLastActionForTest() const { return last_action_; }
  size_t GetProgressBarCountForTest() const { return progress_bars_.size(); }
  bool HasProgressBarForEntityForTest(int entity_id) const;

  // Testing hooks to validate binding without a real cocos runtime.
  void ForceRenderForTest();
  UiRenderPlan GetLastRenderPlanForTest() const { return last_render_plan_; }

 private:
  GameStageScene(Integration::SceneFlowService* scene_flow,
                 const Integration::BattleLaunchParams& launch_params);

  void VerifyStage() const;
  void AttachEvents();
  void DetachEvents();
  void BuildStaticUi();
  void RefreshUi();
  void UpdateHud(const UiRenderPlan& plan);
  void UpdateBuildingOverlays(const UiRenderPlan& plan);
  void UpdateBattlePanel(const UiRenderPlan& plan);
  void UpdatePanelsVisibility();
  bool HitTest(const cocos2d::Rect& bounds, const cocos2d::Vec2& screen_pos) const;
  cocos2d::Rect GetBuildPanelBounds() const;
  cocos2d::Rect GetAttackPanelBounds() const;
  cocos2d::Rect GetSaveBaseButtonBounds() const;
  cocos2d::Rect GetLoadBaseButtonBounds() const;
  cocos2d::Rect GetDemoBuildButtonBounds() const;
  cocos2d::Rect GetDemoAttackButtonBounds() const;
  cocos2d::Rect GetDemoResultsButtonBounds() const;
  void TryShowResultsFromEvent();
  void RecordAction(const std::string& action);

  Integration::SceneFlowService* scene_flow_ = nullptr;
  Integration::BattleLaunchParams launch_params_{}; 
  Gameplay::GameEventManager* event_manager_ = nullptr;
  UiStateStore ui_state_store_{/*local_owner_id=*/0};
  UiPresentationBinding ui_binding_{};
  UiRenderPlan last_render_plan_{};

  cocos2d::Node* ui_root_ = nullptr;
  cocos2d::Label* gold_label_ = nullptr;
  cocos2d::Label* elixir_label_ = nullptr;
  cocos2d::Label* countdown_label_ = nullptr;
  cocos2d::Label* build_panel_label_ = nullptr;
  cocos2d::Label* attack_panel_label_ = nullptr;
  std::unordered_map<int, cocos2d::Node*> progress_bars_;
  bool build_panel_open_ = false;
  bool attack_panel_open_ = false;
  std::string last_action_{};
};

#endif  // CLASSES_SCENES_GAMESTAGESCENE_H_
