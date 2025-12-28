#include "Classes/Contract/Gameplay/HealthComp.h"

#include <unordered_map>

namespace {

struct HealthState {
    int current_hp = 0;
    int max_hp = 0;
    cocos2d::Vec2 bar_offset{0.0f, 0.0f};
};

std::unordered_map<const HealthComp*, HealthState>& HealthStates() {
    static std::unordered_map<const HealthComp*, HealthState> states;
    return states;
}

HealthState& GetState(const HealthComp* comp) {
    return HealthStates()[comp];
}

}  // namespace

float HealthComp::GetHealthPercentage() const {
    const auto& state = GetState(this);
    if (state.max_hp <= 0) {
        return 0.0f;
    }
    float ratio = static_cast<float>(state.current_hp) / static_cast<float>(state.max_hp);
    if (ratio < 0.0f) {
        return 0.0f;
    }
    if (ratio > 1.0f) {
        return 1.0f;
    }
    return ratio;
}

int HealthComp::GetCurrentHP() const {
    return GetState(this).current_hp;
}

int HealthComp::GetMaxHP() const {
    return GetState(this).max_hp;
}

bool HealthComp::IsDead() const {
    return GetState(this).current_hp <= 0;
}

void HealthComp::SetHealthBarOffset(cocos2d::Vec2 offset) {
    GetState(this).bar_offset = offset;
}

bool HealthComp::TakeDamage(int amount) {
    if (amount <= 0) {
        return IsDead();
    }
    auto& state = GetState(this);
    state.current_hp -= amount;
    if (state.current_hp < 0) {
        state.current_hp = 0;
    }
    return state.current_hp <= 0;
}

void HealthComp::InitStats(int max_hp) {
    auto& state = GetState(this);
    state.max_hp = max_hp > 0 ? max_hp : 0;
    state.current_hp = state.max_hp;
}
