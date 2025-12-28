#ifndef CLASSES_MOCKS_INTEGRATIONMOCK_SCENEFLOWSERVICE_H_
#define CLASSES_MOCKS_INTEGRATIONMOCK_SCENEFLOWSERVICE_H_

#include "Classes/Contract/Integration/SceneFlowService.h"

namespace Integration {

// Creates a mock wrapper that delegates to the provided implementation and
// records via the delegate's state.
SceneFlowService* CreateMockSceneFlowService(SceneFlowService* delegate);

}  // namespace Integration

#endif  // CLASSES_MOCKS_INTEGRATIONMOCK_SCENEFLOWSERVICE_H_
