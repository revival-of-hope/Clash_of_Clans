#include "Classes/Scenes/MenuScene.h"

namespace {

cocos2d::Rect MakeButtonRect(const cocos2d::Vec2& center, float width, float height) {
  return cocos2d::Rect(center.x - width * 0.5f, center.y - height * 0.5f, width,
                       height);
}

}  // namespace

MenuScene::MenuScene(Integration::SceneFlowService* scene_flow)
    : scene_flow_(scene_flow),
      level_manager_(LevelManager::GetInstance()),
      audio_manager_(Integration::ResolveAudioManager()) {}

MenuScene* MenuScene::Create(Integration::SceneFlowService* scene_flow) {
  auto* scene = new MenuScene(scene_flow);
  if (scene && scene->init()) {
    scene->VerifyStage();
    return scene;
  }
  delete scene;
  return nullptr;
}

bool MenuScene::init() {
  if (level_manager_) {
    level_manager_->SelectMapA();
  }
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

  auto* background = cocos2d::Node::create();
  if (background) {
    background->setPosition(cocos2d::Vec2(400.0f, 320.0f));
    addChild(background);
  }

  auto* start_button = cocos2d::Node::create();
  if (start_button) {
    start_button->setPosition(cocos2d::Vec2(400.0f, 140.0f));
    addChild(start_button);
  }

  auto* start_label =
      cocos2d::Label::createWithSystemFont("Start", "Arial", 20);
  if (start_label) {
    start_label->setPosition(cocos2d::Vec2(400.0f, 140.0f));
    addChild(start_label);
  }

  auto* map_a_label = cocos2d::Label::createWithSystemFont("Map A", "Arial", 18);
  if (map_a_label) {
    map_a_label->setPosition(cocos2d::Vec2(260.0f, 200.0f));
    addChild(map_a_label);
  }

  auto* map_b_label = cocos2d::Label::createWithSystemFont("Map B", "Arial", 18);
  if (map_b_label) {
    map_b_label->setPosition(cocos2d::Vec2(540.0f, 200.0f));
    addChild(map_b_label);
  }

  auto* leagues_label =
      cocos2d::Label::createWithSystemFont("Leagues", "Arial", 18);
  if (leagues_label) {
    leagues_label->setPosition(cocos2d::Vec2(260.0f, 100.0f));
    addChild(leagues_label);
  }

  auto* replays_label =
      cocos2d::Label::createWithSystemFont("Replays", "Arial", 18);
  if (replays_label) {
    replays_label->setPosition(cocos2d::Vec2(540.0f, 100.0f));
    addChild(replays_label);
  }

  if (audio_manager_) {
    audio_manager_->PlayMenuMusic();
  }

  return true;
}

void MenuScene::SelectMapA() {
  if (!level_manager_) {
    return;
  }
  level_manager_->SelectMapA();
}

void MenuScene::SelectMapB() {
  if (!level_manager_) {
    return;
  }
  level_manager_->SelectMapB();
}

cocos2d::Scene* MenuScene::StartSelectedMap() {
  if (!scene_flow_ || !level_manager_) {
    return nullptr;
  }
  Integration::BattleLaunchParams params;
  params.map_path = level_manager_->GetSelectedMapPath();
  params.seed = level_manager_->GetSeed();
  return scene_flow_->StartGame(params);
}

bool MenuScene::HandleTap(const cocos2d::Vec2& screen_pos) {
  if (HitTest(GetStartButtonBounds(), screen_pos)) {
    RecordAction("start");
    if (audio_manager_) {
      audio_manager_->PlayUiClick();
    }
    return StartSelectedMap() != nullptr;
  }
  if (HitTest(GetMapAButtonBounds(), screen_pos)) {
    SelectMapA();
    RecordAction("map_a");
    if (audio_manager_) {
      audio_manager_->PlayUiClick();
    }
    return true;
  }
  if (HitTest(GetMapBButtonBounds(), screen_pos)) {
    SelectMapB();
    RecordAction("map_b");
    if (audio_manager_) {
      audio_manager_->PlayUiClick();
    }
    return true;
  }
  if (HitTest(GetLeaguesButtonBounds(), screen_pos)) {
    RecordAction("leagues");
    if (audio_manager_) {
      audio_manager_->PlayUiClick();
    }
    return true;
  }
  if (HitTest(GetReplaysButtonBounds(), screen_pos)) {
    RecordAction("replays");
    if (audio_manager_) {
      audio_manager_->PlayUiClick();
    }
    return true;
  }
  return false;
}

void MenuScene::VerifyStage() const {
  if (!scene_flow_) {
    return;
  }
  assert(scene_flow_->GetCurrentStage() == Integration::SceneStage::kMenu);
}

bool MenuScene::HitTest(const cocos2d::Rect& bounds,
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

void MenuScene::RecordAction(const std::string& action) {
  last_action_ = action;
}

cocos2d::Rect MenuScene::GetStartButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(400.0f, 140.0f), 200.0f, 60.0f);
}

cocos2d::Rect MenuScene::GetMapAButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(260.0f, 200.0f), 140.0f, 50.0f);
}

cocos2d::Rect MenuScene::GetMapBButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(540.0f, 200.0f), 140.0f, 50.0f);
}

cocos2d::Rect MenuScene::GetLeaguesButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(260.0f, 100.0f), 160.0f, 50.0f);
}

cocos2d::Rect MenuScene::GetReplaysButtonBounds() const {
  return MakeButtonRect(cocos2d::Vec2(540.0f, 100.0f), 160.0f, 50.0f);
}
