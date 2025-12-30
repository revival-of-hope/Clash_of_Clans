// Source: Classes/Integration/Docs/DevC_Spec.md (Player Identity Service)
#ifndef CONTRACT_INTEGRATION_PLAYERIDENTITYSERVICE_H_
#define CONTRACT_INTEGRATION_PLAYERIDENTITYSERVICE_H_

#include <string>

#include "Classes/Contract/Integration/PlayerIdentity.h"

namespace Integration {

class PlayerIdentityService {
public:
    static PlayerIdentityService* GetInstance();

    PlayerIdentity CreateIdentity(const std::string& name, const std::string& icon_id);
    PlayerIdentity GetIdentity() const;
    void SetIdentity(const PlayerIdentity& identity);
};

}  // namespace Integration

#endif  // CONTRACT_INTEGRATION_PLAYERIDENTITYSERVICE_H_
