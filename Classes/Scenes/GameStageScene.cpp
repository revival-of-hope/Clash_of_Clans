#include "Classes/Scenes/GameStageScene.h"

#include <string>

#include "Core/GameConstants.h"

namespace {

cocos2d::Rect MakePanelRect(const cocos2d::Vec2& center, float width, float height) {
  return cocos2d::Rect(center.x - width * 0.5f, center.y - height * 0.5f, width,
                       height);
}

}  // namespace

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

bool GameStageScene::init() {
  if (!cocos2d::Scene::init()) {
    return false;
  }

#if defined(USE_COCOS_ENGINE) && USE_COCOS_ENGINE
  auto* listener = cocos2d::EventListenerTouchOneByOne::create();
  if (listener) {
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event*) {
      if (!touch) {
        return false;
      }
      return HandleTap(touch->getLocation());
    };
    cocos2d::Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(listener, this);
  }
#endif

  ui_root_ = cocos2d::Node::create();
  addChild(ui_root_, static_cast<int>(Core::ZOrder::kUiHud));

  AttachEvents();
  ui_state_store_.SetMode(UiMode::kBuild);
  BuildStaticUi();

  RefreshUi();
  return true;
}

GameStageScene::~GameStageScene() { DetachEvents(); }

void GameStageScene::onExit() {
  DetachEvents();
  Scene::onExit();
}

void GameStageScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kGame);
}

void GameStageScene::AttachEvents() {
  event_manager_ = Gameplay::GameEventManager::GetInstance();
  if (event_manager_) {
    ui_state_store_.Attach(event_manager_);
  }
}

void GameStageScene::DetachEvents() {
  if (event_manager_) {
    ui_state_store_.Detach(event_manager_);
    event_manager_ = nullptr;
  }
}

void GameStageScene::BuildStaticUi() {
  auto* hud_layer = cocos2d::Node::create();
  hud_layer->setPosition(cocos2d::Vec2(12.0f, 560.0f));
  ui_root_->addChild(hud_layer);

  gold_label_ = cocos2d::Label::createWithSystemFont("0/0", "Arial", 18);
  if (gold_label_) {
    gold_label_->setPosition(cocos2d::Vec2(24.0f, 0.0f));
    hud_layer->addChild(gold_label_);
  }

  elixir_label_ = cocos2d::Label::createWithSystemFont("0/0", "Arial", 18);
  if (elixir_label_) {
    elixir_label_->setPosition(cocos2d::Vec2(144.0f, 0.0f));
    hud_layer->addChild(elixir_label_);
  }

  countdown_label_ = cocos2d::Label::createWithSystemFont("", "Arial", 24);
  if (countdown_label_) {
    countdown_label_->setPosition(cocos2d::Vec2(660.0f, 560.0f));
    ui_root_->addChild(countdown_label_);
  }

  build_panel_label_ =
      cocos2d::Label::createWithSystemFont("Build Panel", "Arial", 18);
  if (build_panel_label_) {
    build_panel_label_->setPosition(cocos2d::Vec2(120.0f, 320.0f));
    ui_root_->addChild(build_panel_label_);
  }

  auto* save_label = cocos2d::Label::createWithSystemFont("Save Base", "Arial", 16);
  if (save_label) {
    save_label->setPosition(cocos2d::Vec2(120.0f, 280.0f));
    ui_root_->addChild(save_label);
  }

  auto* load_label = cocos2d::Label::createWithSystemFont("Load Base", "Arial", 16);
  if (load_label) {
    load_label->setPosition(cocos2d::Vec2(120.0f, 250.0f));
    ui_root_->addChild(load_label);
  }

  attack_panel_label_ =
      cocos2d::Label::createWithSystemFont("Attack Panel", "Arial", 18);
  if (attack_panel_label_) {
    attack_panel_label_->setPosition(cocos2d::Vec2(680.0f, 320.0f));
    ui_root_->addChild(attack_panel_label_);
  }

  auto* demo_label =
      cocos2d::Label::createWithSystemFont("Demo Controls", "Arial", 16);
  if (demo_label) {
    demo_label->setPosition(cocos2d::Vec2(640.0f, 520.0f));
    ui_root_->addChild(demo_label);
  }

  auto* demo_build = cocos2d::Label::createWithSystemFont("Build", "Arial", 14);
  if (demo_build) {
    demo_build->setPosition(cocos2d::Vec2(600.0f, 490.0f));
    ui_root_->addChild(demo_build);
  }

  auto* demo_attack = cocos2d::Label::createWithSystemFont("Attack", "Arial", 14);
  if (demo_attack) {
    demo_attack->setPosition(cocos2d::Vec2(680.0f, 490.0f));
    ui_root_->addChild(demo_attack);
  }

  auto* demo_results =
      cocos2d::Label::createWithSystemFont("Results", "Arial", 14);
  if (demo_results) {
    demo_results->setPosition(cocos2d::Vec2(640.0f, 460.0f));
    ui_root_->addChild(demo_results);
  }
}

void GameStageScene::RefreshUi() {
  UiStateSnapshot snapshot = ui_state_store_.GetSnapshot();
  if (snapshot.battle.in_battle) {
    snapshot.mode = UiMode::kAttack;
  }
  last_render_plan_ = ui_binding_.BuildRenderPlan(snapshot);
  UpdateHud(last_render_plan_);
  UpdateBuildingOverlays(last_render_plan_);
  UpdateBattlePanel(last_render_plan_);
  UpdatePanelsVisibility();
}

void GameStageScene::UpdateHud(const UiRenderPlan& plan) {
  if (plan.hud_items.size() >= 2) {
    if (gold_label_) {
      gold_label_->setString(plan.hud_items[0].label_text);
    }
    if (elixir_label_) {
      elixir_label_->setString(plan.hud_items[1].label_text);
    }
  }
}

void GameStageScene::UpdateBuildingOverlays(const UiRenderPlan& plan) {
  std::unordered_map<int, cocos2d::Node*> next_overlays;
  float y_offset = 520.0f;
  for (const auto& overlay : plan.building_overlays) {
    cocos2d::Node* bar = nullptr;
    auto existing = progress_bars_.find(overlay.entity_id);
    if (existing != progress_bars_.end()) {
      bar = existing->second;
    } else {
      bar = cocos2d::Node::create();
      ui_root_->addChild(bar);
    }
    bar->setPosition(cocos2d::Vec2(12.0f, y_offset));
    next_overlays[overlay.entity_id] = bar;
    y_offset -= 22.0f;
  }
  progress_bars_.swap(next_overlays);
}

void GameStageScene::UpdateBattlePanel(const UiRenderPlan& plan) {
  if (countdown_label_) {
    if (plan.battle_panel.show_countdown) {
      countdown_label_->setString(std::to_string(plan.battle_panel.countdown_seconds));
    } else {
      countdown_label_->setString("");
    }
  }
}

void GameStageScene::UpdatePanelsVisibility() {
  (void)build_panel_label_;
  (void)attack_panel_label_;
}

void GameStageScene::ForceRenderForTest() { RefreshUi(); }

bool GameStageScene::HandleTap(const cocos2d::Vec2& screen_pos) {
  if (HitTest(GetDemoBuildButtonBounds(), screen_pos)) {
    ui_state_store_.SetMode(UiMode::kBuild);
    RecordAction("demo_build");
    RefreshUi();
    return true;
  }

  if (HitTest(GetDemoAttackButtonBounds(), screen_pos)) {
    ui_state_store_.SetMode(UiMode::kAttack);
    RecordAction("demo_attack");
    RefreshUi();
    return true;
  }

  if (HitTest(GetDemoResultsButtonBounds(), screen_pos)) {
    RecordAction("demo_results");
    TryShowResultsFromEvent();
    return true;
  }

  UiStateSnapshot snapshot = ui_state_store_.GetSnapshot();
  UiMode mode = snapshot.mode;
  if (snapshot.battle.in_battle) {
    mode = UiMode::kAttack;
  }

  if (mode == UiMode::kBuild && HitTest(GetSaveBaseButtonBounds(), screen_pos)) {
    RecordAction("save_base");
    return true;
  }

  if (mode == UiMode::kBuild && HitTest(GetLoadBaseButtonBounds(), screen_pos)) {
    RecordAction("load_base");
    return true;
  }

  if (mode == UiMode::kBuild && HitTest(GetBuildPanelBounds(), screen_pos)) {
    build_panel_open_ = !build_panel_open_;
    UpdatePanelsVisibility();
    return true;
  }

  if (mode == UiMode::kAttack && HitTest(GetAttackPanelBounds(), screen_pos)) {
    attack_panel_open_ = !attack_panel_open_;
    UpdatePanelsVisibility();
    return true;
  }

  return false;
}

void GameStageScene::RecordAction(const std::string& action) {
  last_action_ = action;
}

bool GameStageScene::HasProgressBarForEntityForTest(int entity_id) const {
  return progress_bars_.find(entity_id) != progress_bars_.end();
}

bool GameStageScene::HitTest(const cocos2d::Rect& bounds,
                             const cocos2d::Vec2& screen_pos) const {
  return screen_pos.x >= bounds.x && screen_pos.x <= bounds.x + bounds.width &&
         screen_pos.y >= bounds.y && screen_pos.y <= bounds.y + bounds.height;
}

cocos2d::Rect GameStageScene::GetBuildPanelBounds() const {
  return MakePanelRect(cocos2d::Vec2(120.0f, 320.0f), 220.0f, 80.0f);
}

cocos2d::Rect GameStageScene::GetAttackPanelBounds() const {
  return MakePanelRect(cocos2d::Vec2(680.0f, 320.0f), 220.0f, 80.0f);
}

cocos2d::Rect GameStageScene::GetSaveBaseButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(120.0f, 280.0f), 160.0f, 40.0f);
}

cocos2d::Rect GameStageScene::GetLoadBaseButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(120.0f, 250.0f), 160.0f, 40.0f);
}

cocos2d::Rect GameStageScene::GetDemoBuildButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(600.0f, 490.0f), 100.0f, 32.0f);
}

cocos2d::Rect GameStageScene::GetDemoAttackButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(680.0f, 490.0f), 100.0f, 32.0f);
}

cocos2d::Rect GameStageScene::GetDemoResultsButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(640.0f, 460.0f), 140.0f, 32.0f);
}

void GameStageScene::TryShowResultsFromEvent() {
  if (!scene_flow_) {
    return;
  }
  Gameplay::GameEventManager* manager = Gameplay::GameEventManager::GetInstance();
  if (!manager) {
    return;
  }
  Gameplay::BattleEndEvent cached = manager->GetLastBattleEnded();
  Integration::ResultsScreenData results;
  results.summary = cached;
  results.elapsed_seconds = cached.battle_duration_seconds;
  results.troops_deployed = cached.troops_deployed;
  results.troops_remaining = cached.troops_remaining;

  cocos2d::Scene* results_scene = scene_flow_->ShowResults(results);
  if (results_scene) {
    cocos2d::Director::getInstance()->replaceScene(results_scene);
  }
}
