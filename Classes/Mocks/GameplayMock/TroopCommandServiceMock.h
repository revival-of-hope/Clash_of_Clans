// Mock helpers for TroopCommandService.
#ifndef MOCKS_GAMEPLAYMOCK_TROOP_COMMAND_SERVICE_MOCK_H_
#define MOCKS_GAMEPLAYMOCK_TROOP_COMMAND_SERVICE_MOCK_H_

#include "Classes/Contract/Gameplay/TroopCommandService.h"

namespace Gameplay {

class TroopCommandServiceMock {
public:
    static void Reset();
    static int GetTrainRequestCount();
    static int GetDeployRequestCount();
    static TroopTrainRequest GetLastTrainRequest();
    static TroopDeployRequest GetLastDeployRequest();
};

}  // namespace Gameplay

#endif  // MOCKS_GAMEPLAYMOCK_TROOP_COMMAND_SERVICE_MOCK_H_
