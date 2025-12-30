#include "Classes/Integration/GameServices.h"

#include "Classes/Contract/Gameplay/GameEvents.h"
#include "Classes/Contract/Gameplay/TroopCommandService.h"
#include "Classes/Contract/Integration/PlayerIdentityService.h"
#include "Classes/Integration/SceneFlowServiceImpl.h"
#include "Classes/Managers/AudioManager/AudioManager.h"
#include "Classes/Mocks/IntegrationMock/SceneFlowService.h"

#if defined(USE_COCOS_ENGINE) && USE_COCOS_ENGINE
#include "Classes/Integration/Engine/CocosAudioSink.h"
#else
#include "Classes/Mocks/Audio/AudioSinkMock.h"
#endif

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

PlayerIdentityService* ResolvePlayerIdentityService() {
    return PlayerIdentityService::GetInstance();
}

Gameplay::TroopCommandService* ResolveTroopCommandService() {
    return Gameplay::TroopCommandService::GetInstance();
}

AudioManager* ResolveAudioManager() {
    static Gameplay::GameEventManager* event_manager = ResolveGameEventManager();
#if defined(USE_COCOS_ENGINE) && USE_COCOS_ENGINE
    static CocosAudioSink sink;
#else
    static AudioSinkMock sink;
#endif
    static AudioManager manager(event_manager, &sink);
    return &manager;
}

bool IsUsingMocksForTest() { return kUseMocks; }

}  // namespace Integration
