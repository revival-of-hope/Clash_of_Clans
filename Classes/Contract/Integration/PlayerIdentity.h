// Source: Classes/Integration/Docs/DevC_Spec.md (Player Identity)
#ifndef CONTRACT_INTEGRATION_PLAYERIDENTITY_H_
#define CONTRACT_INTEGRATION_PLAYERIDENTITY_H_

#include <string>

namespace Integration {

struct PlayerIdentity {
    int player_id = 0;
    std::string name;
    std::string icon_id;
};

}  // namespace Integration

#endif  // CONTRACT_INTEGRATION_PLAYERIDENTITY_H_
