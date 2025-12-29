#include "Classes/Contract/Gameplay/HealthComp.h"

#include <algorithm>

bool HealthComp::init() {
    return cocos2d::Node::init();
}

float HealthComp::GetHealthPercentage() const {
    if (max_hp_ <= 0.0f) {
        return 0.0f;
    }
    return std::min(1.0f, std::max(0.0f, current_hp_ / max_hp_));
}

bool HealthComp::TakeDamage(int amount) {
    if (amount <= 0) {
        return is_dead_;
    }
    current_hp_ -= static_cast<float>(amount);
    if (current_hp_ <= 0.0f) {
        current_hp_ = 0.0f;
        is_dead_ = true;
    }
    UpdateHealthBar();
    return is_dead_;
}

void HealthComp::Heal(int amount) {
    if (amount <= 0 || is_dead_) {
        return;
    }
    current_hp_ = std::min(max_hp_, current_hp_ + static_cast<float>(amount));
    UpdateHealthBar();
}

void HealthComp::InitStats(int max_hp) {
    max_hp_ = static_cast<float>(std::max(max_hp, 0));
    current_hp_ = max_hp_;
    is_dead_ = max_hp_ <= 0;
    UpdateHealthBar();
}

void HealthComp::SetHealthBarOffset(cocos2d::Vec2 offset) {
    bar_offset_ = offset;
}

void HealthComp::UpdateHealthBar() {
    // Mock does not draw, but keeps health values consistent.
}

