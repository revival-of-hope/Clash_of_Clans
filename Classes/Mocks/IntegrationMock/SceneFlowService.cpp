#include "Classes/Mocks/IntegrationMock/SceneFlowService.h"

#include <memory>

#include "Classes/Integration/SceneFlowServiceImpl.h"

#include "Classes/Scenes/BootScene.h"
#include "Classes/Scenes/GameStageScene.h"
#include "Classes/Scenes/MenuScene.h"
#include "Classes/Scenes/ResultsScene.h"

namespace Integration {
namespace {
class MockSceneFlowService : public SceneFlowService {
 public:
  explicit MockSceneFlowService(SceneFlowService* delegate)
      : delegate_(delegate) {}

  cocos2d::Scene* CreateBootScene() override {
    return delegate_->CreateBootScene();
  }

  cocos2d::Scene* ShowMenuScene() override {
    return delegate_->ShowMenuScene();
  }

  cocos2d::Scene* StartGame(const BattleLaunchParams& params) override {
    return delegate_->StartGame(params);
  }

  cocos2d::Scene* ShowResults(const ResultsScreenData& results) override {
    return delegate_->ShowResults(results);
  }

  SceneStage GetCurrentStage() const override {
    return delegate_->GetCurrentStage();
  }

  BattleLaunchParams GetLastLaunchParams() const override {
    return delegate_->GetLastLaunchParams();
  }

  ResultsScreenData GetLastResults() const override {
    return delegate_->GetLastResults();
  }

 private:
  std::unique_ptr<SceneFlowService> delegate_;
};
}  // namespace

SceneFlowService* CreateMockSceneFlowService(SceneFlowService* delegate) {
  return new MockSceneFlowService(delegate);
}

}  // namespace Integration
