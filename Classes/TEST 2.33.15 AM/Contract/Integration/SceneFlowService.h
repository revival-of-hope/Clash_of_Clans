// Source: Classes/Integration/Docs/Scene Flow and Entrypoint.md
#ifndef Contract_INTEGRATION_SCENEFLOWSERVICE_H_
#define Contract_INTEGRATION_SCENEFLOWSERVICE_H_

#include <string>

#include "Classes/Contract/Gameplay/GameEvents.h"
#include "cocos2d.h"

namespace Integration {

enum class SceneStage {
    kBoot,
    kMenu,
    kGame,
    kResults,
};

struct BattleLaunchParams {
    std::string map_path;
    int seed = 0;
};

struct ResultsScreenData {
    Gameplay::BattleEndEvent summary;
    int elapsed_seconds = 0;
    int troops_deployed = 0;
    int troops_remaining = 0;
};

class SceneFlowService {
public:
    virtual ~SceneFlowService() = default;

    virtual cocos2d::Scene* CreateBootScene() = 0;
    virtual cocos2d::Scene* ShowMenuScene() = 0;
    virtual cocos2d::Scene* StartGame(const BattleLaunchParams& params) = 0;
    virtual cocos2d::Scene* ShowResults(const ResultsScreenData& results) = 0;
    virtual SceneStage GetCurrentStage() const = 0;
    virtual BattleLaunchParams GetLastLaunchParams() const = 0;
    virtual ResultsScreenData GetLastResults() const = 0;
};

SceneFlowService* CreateSceneFlowService();

}  // namespace Integration

#endif  // Contract_INTEGRATION_SCENEFLOWSERVICE_H_
