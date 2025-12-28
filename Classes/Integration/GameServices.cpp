#include "Classes/Integration/GameServices.h"

#include "Classes/Contract/Gameplay/GameEvents.h"
#include "Classes/Integration/SceneFlowServiceImpl.h"
#include "Classes/Mocks/IntegrationMock/SceneFlowService.h"

namespace {
constexpr bool kUseMocks = true;
}  // namespace

namespace Integration {

SceneFlowService* ResolveSceneFlowService() {
    if (kUseMocks) {
        return CreateMockSceneFlowService(CreateSceneFlowService());
    }
    return CreateSceneFlowService();
}

InputRouter* ResolveInputRouter() {
    if (kUseMocks) {
        return new InputRouter();
    }
    return nullptr;
}

Gameplay::GameEventManager* ResolveGameEventManager() {
    if (kUseMocks) {
        return Gameplay::GameEventManager::GetInstance();
    }
    return nullptr;
}

}  // namespace Integration
