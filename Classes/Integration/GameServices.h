#ifndef INTEGRATION_GAMESERVICES_H_
#define INTEGRATION_GAMESERVICES_H_

#include "Classes/Contract/Engine/InputRouter.h"
#include "Classes/Contract/Integration/PlayerIdentityService.h"
#include "Classes/Contract/Integration/SceneFlowService.h"

#include "Classes/Contract/Gameplay/GameEvents.h"
#include "Classes/Contract/Gameplay/TroopCommandService.h"
#include "Classes/Managers/AudioManager/AudioManager.h"

namespace Integration {

SceneFlowService* ResolveSceneFlowService();
InputRouter* ResolveInputRouter();
Gameplay::GameEventManager* ResolveGameEventManager();
PlayerIdentityService* ResolvePlayerIdentityService();
Gameplay::TroopCommandService* ResolveTroopCommandService();
AudioManager* ResolveAudioManager();
bool IsUsingMocksForTest();

}  // namespace Integration

#endif  // INTEGRATION_GAMESERVICES_H_
