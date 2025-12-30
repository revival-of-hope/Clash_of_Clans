#include "Classes/Contract/Gameplay/TroopCommandService.h"

#include "Classes/Mocks/GameplayMock/TroopCommandServiceMock.h"

namespace Gameplay {

namespace {

TroopTrainRequest last_train_request{};
TroopDeployRequest last_deploy_request{};
int train_request_count = 0;
int deploy_request_count = 0;

}  // namespace

TroopCommandService* TroopCommandService::GetInstance() {
    static TroopCommandService instance;
    return &instance;
}

bool TroopCommandService::RequestTrainTroop(const TroopTrainRequest& request) {
    last_train_request = request;
    ++train_request_count;
    return true;
}

bool TroopCommandService::RequestDeployTroop(const TroopDeployRequest& request) {
    last_deploy_request = request;
    ++deploy_request_count;
    return true;
}

void TroopCommandServiceMock::Reset() {
    last_train_request = TroopTrainRequest{};
    last_deploy_request = TroopDeployRequest{};
    train_request_count = 0;
    deploy_request_count = 0;
}

int TroopCommandServiceMock::GetTrainRequestCount() {
    return train_request_count;
}

int TroopCommandServiceMock::GetDeployRequestCount() {
    return deploy_request_count;
}

TroopTrainRequest TroopCommandServiceMock::GetLastTrainRequest() {
    return last_train_request;
}

TroopDeployRequest TroopCommandServiceMock::GetLastDeployRequest() {
    return last_deploy_request;
}

}  // namespace Gameplay
