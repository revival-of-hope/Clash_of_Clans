#include "Classes/Scenes/GameStageScene.h"

#include <string>

#include "Classes/Contract/Gameplay/CostQuery.h"
#include "Classes/Contract/Gameplay/EconomySystem.h"
#include "Classes/Contract/Integration/PlayerIdentityService.h"
#include "Classes/Integration/GameServices.h"
#include "Core/GameConstants.h"

namespace {

cocos2d::Rect MakePanelRect(const cocos2d::Vec2& center, float width, float height) {
  return cocos2d::Rect(center.x - width * 0.5f, center.y - height * 0.5f, width,
                       height);
}

int ResolveLocalOwnerId() {
  auto* service = Integration::PlayerIdentityService::GetInstance();
  if (!service) {
    return 0;
  }
  Integration::PlayerIdentity identity = service->GetIdentity();
  return identity.player_id;
}

}  // namespace

GameStageScene::GameStageScene(Integration::SceneFlowService* scene_flow,
                               const Integration::BattleLaunchParams& launch_params)
    : scene_flow_(scene_flow),
      launch_params_(launch_params),
      local_owner_id_(ResolveLocalOwnerId()),
      ui_state_store_(local_owner_id_) {}

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

  if (!launch_params_.map_path.empty()) {
    map_layer_ = MapLayer::create(launch_params_.map_path);
  }
  if (map_layer_) {
    addChild(map_layer_, static_cast<int>(Core::ZOrder::kGround));
    tile_highlighter_ = new TileHighlighter(map_layer_->getMap());
  }

  input_router_ = Integration::ResolveInputRouter();
  if (input_router_) {
    input_router_->AttachMap(map_layer_, tile_highlighter_);
  }
  placement_controller_ = new TilePlacementController(this);
  troop_commands_ = Integration::ResolveTroopCommandService();

  AttachEvents();
  ui_state_store_.SetMode(UiMode::kBuild);
  Integration::PlayerIdentityService* identity_service =
      Integration::PlayerIdentityService::GetInstance();
  if (identity_service) {
    ui_state_store_.SetPlayerIdentity(identity_service->GetIdentity());
  }
  BuildStaticUi();

  RefreshUi();
  return true;
}

GameStageScene::~GameStageScene() {
  DetachEvents();
  delete placement_controller_;
  placement_controller_ = nullptr;
  delete input_router_;
  input_router_ = nullptr;
  delete tile_highlighter_;
  tile_highlighter_ = nullptr;
}

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

  gem_label_ = cocos2d::Label::createWithSystemFont("0/0", "Arial", 18);
  if (gem_label_) {
    gem_label_->setPosition(cocos2d::Vec2(264.0f, 0.0f));
    hud_layer->addChild(gem_label_);
  }

  player_label_ = cocos2d::Label::createWithSystemFont("", "Arial", 16);
  if (player_label_) {
    player_label_->setPosition(cocos2d::Vec2(24.0f, -24.0f));
    hud_layer->addChild(player_label_);
  }

  loot_label_ = cocos2d::Label::createWithSystemFont("", "Arial", 14);
  if (loot_label_) {
    loot_label_->setPosition(cocos2d::Vec2(24.0f, -46.0f));
    hud_layer->addChild(loot_label_);
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

  selected_building_label_ =
      cocos2d::Label::createWithSystemFont("", "Arial", 14);
  if (selected_building_label_) {
    selected_building_label_->setPosition(cocos2d::Vec2(300.0f, 320.0f));
    ui_root_->addChild(selected_building_label_);
  }

  troop_inspect_label_ =
      cocos2d::Label::createWithSystemFont("", "Arial", 14);
  if (troop_inspect_label_) {
    troop_inspect_label_->setPosition(cocos2d::Vec2(520.0f, 220.0f));
    ui_root_->addChild(troop_inspect_label_);
  }

  matchmaking_label_ =
      cocos2d::Label::createWithSystemFont("Find Match", "Arial", 16);
  if (matchmaking_label_) {
    matchmaking_label_->setPosition(cocos2d::Vec2(680.0f, 280.0f));
    ui_root_->addChild(matchmaking_label_);
  }

  train_troop_label_ =
      cocos2d::Label::createWithSystemFont("Train Troop", "Arial", 16);
  if (train_troop_label_) {
    train_troop_label_->setPosition(cocos2d::Vec2(520.0f, 180.0f));
    ui_root_->addChild(train_troop_label_);
  }
}

void GameStageScene::RefreshUi() {
  UiStateSnapshot snapshot = ui_state_store_.GetSnapshot();
  if (snapshot.battle.in_battle) {
    snapshot.mode = UiMode::kAttack;
  }
  last_render_plan_ = ui_binding_.BuildRenderPlan(snapshot);
  UpdateHud(last_render_plan_);
  UpdateHealthBars(last_render_plan_);
  UpdateBuildingOverlays(last_render_plan_);
  UpdateBuildPalette(last_render_plan_);
  UpdateSelectedBuilding(last_render_plan_);
  UpdateDeploymentBar(last_render_plan_);
  UpdateTroopInspect(last_render_plan_);
  UpdateMatchmakingPanel(last_render_plan_);
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
  if (plan.hud_items.size() >= 3 && gem_label_) {
    gem_label_->setString(plan.hud_items[2].label_text);
  }
  if (player_label_) {
    if (plan.trophies > 0) {
      player_label_->setString(plan.player_identity.name + " (" +
                               std::to_string(plan.trophies) + ")");
    } else {
      player_label_->setString(plan.player_identity.name);
    }
  }
  if (loot_label_) {
    if (!plan.loot_items.empty()) {
      loot_label_->setString(plan.loot_items[0].label_text);
    } else {
      loot_label_->setString("");
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

void GameStageScene::UpdateBuildPalette(const UiRenderPlan& plan) {
  float y_offset = 220.0f;
  const float x_pos = 60.0f;
  if (build_palette_labels_.size() < plan.build_palette_items.size()) {
    size_t current = build_palette_labels_.size();
    for (size_t i = current; i < plan.build_palette_items.size(); ++i) {
      auto* label = cocos2d::Label::createWithSystemFont("", "Arial", 12);
      if (label) {
        ui_root_->addChild(label);
        build_palette_labels_.push_back(label);
      }
    }
  }

  for (size_t i = 0; i < plan.build_palette_items.size(); ++i) {
    const BuildPaletteRenderItem& item = plan.build_palette_items[i];
    cocos2d::Label* label = build_palette_labels_[i];
    if (!label) {
      continue;
    }
    std::string text = item.label_text + " " + item.cost_text + " " + item.time_text;
    if (item.is_selected) {
      text = "* " + text;
    }
    label->setString(text);
    label->setPosition(cocos2d::Vec2(x_pos, y_offset));
    y_offset -= 18.0f;
  }
}

void GameStageScene::UpdateSelectedBuilding(const UiRenderPlan& plan) {
  if (!selected_building_label_) {
    return;
  }
  if (!plan.selected_building.show) {
    selected_building_label_->setString("");
    return;
  }
  std::string text = plan.selected_building.title;
  for (const std::string& line : plan.selected_building.stat_lines) {
    text += "\n" + line;
  }
  selected_building_label_->setString(text);
}

void GameStageScene::UpdateDeploymentBar(const UiRenderPlan& plan) {
  float x_offset = 480.0f;
  const float y_pos = 120.0f;
  if (deployment_labels_.size() < plan.deployment_items.size()) {
    size_t current = deployment_labels_.size();
    for (size_t i = current; i < plan.deployment_items.size(); ++i) {
      auto* label = cocos2d::Label::createWithSystemFont("", "Arial", 12);
      if (label) {
        ui_root_->addChild(label);
        deployment_labels_.push_back(label);
      }
    }
  }

  for (size_t i = 0; i < plan.deployment_items.size(); ++i) {
    const DeploymentRenderItem& item = plan.deployment_items[i];
    cocos2d::Label* label = deployment_labels_[i];
    if (!label) {
      continue;
    }
    if (item.is_selected) {
      selected_troop_type_ = item.troop_type;
    }
    std::string text = std::to_string(static_cast<int>(item.troop_type)) + ":" +
                       std::to_string(item.remaining_count);
    if (item.is_selected) {
      text = "[" + text + "]";
    }
    label->setString(text);
    label->setPosition(cocos2d::Vec2(x_offset, y_pos));
    x_offset += 80.0f;
  }
}

void GameStageScene::UpdateTroopInspect(const UiRenderPlan& plan) {
  if (!troop_inspect_label_) {
    return;
  }
  if (!plan.troop_inspect.show) {
    troop_inspect_label_->setString("");
  } else {
    std::string text = plan.troop_inspect.title;
    for (const std::string& line : plan.troop_inspect.stat_lines) {
      text += "\n" + line;
    }
    troop_inspect_label_->setString(text);
  }

  if (train_troop_label_) {
    CostQuery* cost_query = CostQuery::GetInstance();
    if (cost_query) {
      ResourceCost cost = cost_query->GetTroopTrainingCost(selected_troop_type_, 1);
      std::string label = "Train " +
                          std::to_string(static_cast<int>(selected_troop_type_)) +
                          " " + std::to_string(cost.gold) + "g " +
                          std::to_string(cost.elixir) + "e";
      train_troop_label_->setString(label);
    } else {
      train_troop_label_->setString("Train Troop");
    }
  }
}

void GameStageScene::UpdateMatchmakingPanel(const UiRenderPlan& plan) {
  if (!matchmaking_label_) {
    return;
  }
  if (plan.mode != UiMode::kAttack) {
    matchmaking_label_->setString("");
    return;
  }
  matchmaking_label_->setString("Find Match " + plan.matchmaking.cost_text);
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

void GameStageScene::UpdateHealthBars(const UiRenderPlan& plan) {
  std::unordered_map<int, cocos2d::Label*> next_labels;
  float y_offset = 520.0f;
  for (const auto& bar : plan.health_bars) {
    cocos2d::Label* label = nullptr;
    auto existing = health_bar_labels_.find(bar.entity_id);
    if (existing != health_bar_labels_.end()) {
      label = existing->second;
    } else {
      label = cocos2d::Label::createWithSystemFont("", "Arial", 12);
      if (label) {
        ui_root_->addChild(label);
      }
    }
    if (!label) {
      continue;
    }
    int percent = static_cast<int>(bar.health_ratio * 100.0f);
    std::string prefix = bar.is_ally ? "Ally" : "Enemy";
    label->setString(prefix + " " + std::to_string(bar.entity_id) + " " +
                     std::to_string(percent) + "%");
    label->setPosition(cocos2d::Vec2(360.0f, y_offset));
    y_offset -= 16.0f;
    next_labels[bar.entity_id] = label;
  }

  for (const auto& pair : health_bar_labels_) {
    if (next_labels.find(pair.first) == next_labels.end()) {
      if (pair.second) {
        pair.second->setString("");
      }
    }
  }
  health_bar_labels_.swap(next_labels);
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

  if (mode == UiMode::kBuild) {
    for (size_t i = 0; i < last_render_plan_.build_palette_items.size(); ++i) {
      cocos2d::Rect bounds = MakePanelRect(
          cocos2d::Vec2(60.0f, 220.0f - static_cast<float>(i) * 18.0f), 140.0f, 16.0f);
      if (HitTest(bounds, screen_pos)) {
        ui_state_store_.SetBuildSelection(last_render_plan_.build_palette_items[i].type, 1);
        RecordAction("build_select");
        RefreshUi();
        return true;
      }
    }
  }

  if (mode == UiMode::kBuild && HitTest(GetTrainTroopButtonBounds(), screen_pos)) {
    if (troop_commands_) {
      Gameplay::TroopTrainRequest request{};
      request.troop_type = selected_troop_type_;
      request.level = 1;
      request.count = 1;
      request.owner_id = local_owner_id_;
      bool accepted = troop_commands_->RequestTrainTroop(request);
      RecordAction(accepted ? "train_troop" : "train_troop_rejected");
      return true;
    }
    RecordAction("train_troop_unavailable");
    return true;
  }

  if (mode == UiMode::kAttack && HitTest(GetMatchmakingButtonBounds(), screen_pos)) {
    CostQuery* cost_query = CostQuery::GetInstance();
    EconomySystem* economy = EconomySystem::GetInstance();
    if (cost_query && economy) {
      ResourceCost cost = cost_query->GetMatchmakingCost();
      if (economy->CanAffordCost(cost)) {
        economy->SpendGold(cost.gold);
        RecordAction("find_match");
      } else {
        RecordAction("find_match_blocked");
      }
    }
    return true;
  }

  if (mode == UiMode::kAttack && HitTest(GetAttackPanelBounds(), screen_pos)) {
    attack_panel_open_ = !attack_panel_open_;
    UpdatePanelsVisibility();
    return true;
  }

  if (input_router_) {
    input_router_->SetUiConsumesInput(false);
    bool routed = input_router_->RoutePointerDown(screen_pos);
    InputRouteState route_state = input_router_->GetLastRouteState();
    if (routed && mode == UiMode::kBuild && placement_controller_ && map_layer_) {
      if (!placement_controller_->CanPlaceAt(route_state.world_pos)) {
        RecordAction("place_blocked");
        return true;
      }
      placement_controller_->startPlacement(
          map_layer_, last_render_plan_.assets.deployment_column, route_state.world_pos);
      RecordAction("place_building");
      return true;
    }
    if (routed && mode == UiMode::kAttack && troop_commands_) {
      Gameplay::TroopDeployRequest request{};
      request.troop_type = selected_troop_type_;
      request.level = 1;
      request.owner_id = local_owner_id_;
      request.world_x = route_state.world_pos.x;
      request.world_y = route_state.world_pos.y;
      bool accepted = troop_commands_->RequestDeployTroop(request);
      RecordAction(accepted ? "deploy_troop" : "deploy_troop_rejected");
      return true;
    }
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
#if USE_COCOS_ENGINE
  return screen_pos.x >= bounds.origin.x &&
         screen_pos.x <= bounds.origin.x + bounds.size.width &&
         screen_pos.y >= bounds.origin.y &&
         screen_pos.y <= bounds.origin.y + bounds.size.height;
#else
  return screen_pos.x >= bounds.x && screen_pos.x <= bounds.x + bounds.width &&
         screen_pos.y >= bounds.y && screen_pos.y <= bounds.y + bounds.height;
#endif
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

cocos2d::Rect GameStageScene::GetMatchmakingButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(680.0f, 280.0f), 200.0f, 40.0f);
}

cocos2d::Rect GameStageScene::GetTrainTroopButtonBounds() const {
  return MakePanelRect(cocos2d::Vec2(520.0f, 180.0f), 200.0f, 40.0f);
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
