#include "Classes/Integration/GameServices.h"

#include "Classes/Contract/Gameplay/GameEvents.h"
#include "Classes/Integration/SceneFlowServiceImpl.h"
#include "Classes/Mocks/IntegrationMock/SceneFlowService.h"

namespace {
#if defined(USE_COCOS_ENGINE) && USE_COCOS_ENGINE
constexpr bool kUseMocks = false;
#else
constexpr bool kUseMocks = true;
#endif
}  // namespace

namespace Integration {

SceneFlowService* ResolveSceneFlowService() {
    if (kUseMocks) {
        return CreateMockSceneFlowService(CreateSceneFlowService());
    }
    return CreateSceneFlowService();
}

InputRouter* ResolveInputRouter() {
    return new InputRouter();
}

Gameplay::GameEventManager* ResolveGameEventManager() {
    return Gameplay::GameEventManager::GetInstance();
}

bool IsUsingMocksForTest() { return kUseMocks; }

}  // namespace Integration
