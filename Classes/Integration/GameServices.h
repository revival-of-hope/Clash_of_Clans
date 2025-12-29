#ifndef INTEGRATION_GAMESERVICES_H_
#define INTEGRATION_GAMESERVICES_H_

#include "Classes/Contract/Engine/InputRouter.h"
#include "Classes/Contract/Integration/SceneFlowService.h"

#include "Classes/Contract/Gameplay/GameEvents.h"

namespace Integration {

SceneFlowService* ResolveSceneFlowService();
InputRouter* ResolveInputRouter();
Gameplay::GameEventManager* ResolveGameEventManager();
bool IsUsingMocksForTest();

}  // namespace Integration

#endif  // INTEGRATION_GAMESERVICES_H_
