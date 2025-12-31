// Source: Classes/Integration/Docs/Gameplay Troop Command API.md
#ifndef CONTRACT_GAMEPLAY_TROOP_COMMAND_SERVICE_H_
#define CONTRACT_GAMEPLAY_TROOP_COMMAND_SERVICE_H_

#include "Core/GameConstants.h"

namespace Gameplay {

struct TroopTrainRequest {
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    int level = 1;
    int count = 1;
    int owner_id = 0;
};

struct TroopDeployRequest {
    Core::TroopType troop_type = Core::TroopType::kBarbarian;
    int level = 1;
    int owner_id = 0;
    float world_x = 0.0f;
    float world_y = 0.0f;
};

class TroopCommandService {
public:
    static TroopCommandService* GetInstance();

    bool RequestTrainTroop(const TroopTrainRequest& request);
    bool RequestDeployTroop(const TroopDeployRequest& request);
};

}  // namespace Gameplay

#endif  // CONTRACT_GAMEPLAY_TROOP_COMMAND_SERVICE_H_
