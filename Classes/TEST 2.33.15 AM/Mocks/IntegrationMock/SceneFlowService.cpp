#include "Contracts/Integration/SceneFlowService.h"

namespace Integration {
namespace {
class MockSceneFlowService : public SceneFlowService {
public:
    cocos2d::Scene* CreateBootScene() override {
        stage_ = SceneStage::kBoot;
        return new cocos2d::Scene();
    }

    cocos2d::Scene* ShowMenuScene() override {
        stage_ = SceneStage::kMenu;
        return new cocos2d::Scene();
    }

    cocos2d::Scene* StartGame(const BattleLaunchParams& params) override {
        last_launch_params_ = params;
        stage_ = SceneStage::kGame;
        return new cocos2d::Scene();
    }

    cocos2d::Scene* ShowResults(const ResultsScreenData& results) override {
        last_results_ = results;
        stage_ = SceneStage::kResults;
        return new cocos2d::Scene();
    }

    SceneStage GetCurrentStage() const override { return stage_; }

    BattleLaunchParams GetLastLaunchParams() const override { return last_launch_params_; }

    ResultsScreenData GetLastResults() const override { return last_results_; }

private:
    SceneStage stage_ = SceneStage::kBoot;
    BattleLaunchParams last_launch_params_{};
    ResultsScreenData last_results_{};
};
}  // namespace

SceneFlowService* CreateSceneFlowService() { return new MockSceneFlowService(); }

}  // namespace Integration
