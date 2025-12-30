#include "Classes/Contract/Integration/PlayerIdentityService.h"

namespace Integration {
    namespace {

        PlayerIdentity& IdentityStorage() {
            static PlayerIdentity identity{};
            return identity;
        }

        int& NextPlayerId() {
            static int next_id = 1;
            return next_id;
        }

    }  // namespace

    PlayerIdentityService* PlayerIdentityService::GetInstance() {
        static PlayerIdentityService instance;
        return &instance;
    }

    PlayerIdentity PlayerIdentityService::CreateIdentity(const std::string& name,
        const std::string& icon_id) {
        PlayerIdentity identity;
        identity.player_id = NextPlayerId();
        identity.name = name;
        identity.icon_id = icon_id;
        NextPlayerId() += 1;
        IdentityStorage() = identity;
        return identity;
    }

    PlayerIdentity PlayerIdentityService::GetIdentity() const {
        return IdentityStorage();
    }

    void PlayerIdentityService::SetIdentity(const PlayerIdentity& identity) {
        IdentityStorage() = identity;
    }

}  // namespace Integration
