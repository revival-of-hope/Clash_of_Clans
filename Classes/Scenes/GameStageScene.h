#ifndef CLASSES_SCENES_GAMESTAGESCENE_H_
#define CLASSES_SCENES_GAMESTAGESCENE_H_

#include <cassert>
#include <string>
#include <unordered_map>

#include "Classes/Contract/Integration/SceneFlowService.h"
#include "Classes/Contract/Engine/InputRouter.h"
#include "Classes/Contract/Engine/MapLayer.h"
#include "Classes/Contract/Engine/TileHighlighter.h"
#include "Classes/Contract/Engine/TilePlacementController.h"
#include "Classes/Contract/Gameplay/TroopCommandService.h"
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
  void UpdateBuildPalette(const UiRenderPlan& plan);
  void UpdateSelectedBuilding(const UiRenderPlan& plan);
  void UpdateDeploymentBar(const UiRenderPlan& plan);
  void UpdateTroopInspect(const UiRenderPlan& plan);
  void UpdateMatchmakingPanel(const UiRenderPlan& plan);
  void UpdateHealthBars(const UiRenderPlan& plan);
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
  cocos2d::Rect GetMatchmakingButtonBounds() const;
  cocos2d::Rect GetTrainTroopButtonBounds() const;
  void TryShowResultsFromEvent();
  void RecordAction(const std::string& action);

  Integration::SceneFlowService* scene_flow_ = nullptr;
  Integration::BattleLaunchParams launch_params_{}; 
  Gameplay::GameEventManager* event_manager_ = nullptr;
  int local_owner_id_ = 0;
  UiStateStore ui_state_store_;
  UiPresentationBinding ui_binding_{};
  UiRenderPlan last_render_plan_{};

  cocos2d::Node* ui_root_ = nullptr;
  cocos2d::Label* gold_label_ = nullptr;
  cocos2d::Label* elixir_label_ = nullptr;
  cocos2d::Label* gem_label_ = nullptr;
  cocos2d::Label* player_label_ = nullptr;
  cocos2d::Label* countdown_label_ = nullptr;
  cocos2d::Label* build_panel_label_ = nullptr;
  cocos2d::Label* attack_panel_label_ = nullptr;
  cocos2d::Label* selected_building_label_ = nullptr;
  cocos2d::Label* troop_inspect_label_ = nullptr;
  cocos2d::Label* matchmaking_label_ = nullptr;
  cocos2d::Label* loot_label_ = nullptr;
  cocos2d::Label* train_troop_label_ = nullptr;
  std::vector<cocos2d::Label*> build_palette_labels_;
  std::vector<cocos2d::Label*> deployment_labels_;
  std::unordered_map<int, cocos2d::Label*> health_bar_labels_;
  std::unordered_map<int, cocos2d::Node*> progress_bars_;
  MapLayer* map_layer_ = nullptr;
  TileHighlighter* tile_highlighter_ = nullptr;
  InputRouter* input_router_ = nullptr;
  TilePlacementController* placement_controller_ = nullptr;
  Gameplay::TroopCommandService* troop_commands_ = nullptr;
  Core::TroopType selected_troop_type_ = Core::TroopType::kBarbarian;
  bool build_panel_open_ = false;
  bool attack_panel_open_ = false;
  std::string last_action_{};
};

#endif  // CLASSES_SCENES_GAMESTAGESCENE_H_
