#include "Classes/Contract/Gameplay/Building.h"

#include <algorithm>
#include <string>

namespace {

Core::BuildingStats MakeStats(Core::BuildingType type, int level) {
    Core::BuildingStats stats{};
    stats.width_ = 1;
    stats.height_ = 1;
    stats.range_ = 1.0f;
    stats.attack_speed_ = 1.0f;
    stats.target_type_ = Core::GeneralType::kGround;
    stats.damage_ = std::max(level, 1) * 5;
    stats.max_hp_ = std::max(level, 1) * 100;
    stats.resource_capacity_ = 200 * std::max(level, 1);
    stats.production_rate_ = (type == Core::BuildingType::kGoldMine ||
                              type == Core::BuildingType::kElixirCollector)
                                 ? 20 * std::max(level, 1)
                                 : 0;

    if (type == Core::BuildingType::kAirDefense) {
        stats.target_type_ = Core::GeneralType::kAir;
    }
    return stats;
}

float Clamp01(float value) {
    return std::min(1.0f, std::max(0.0f, value));
}

}  // namespace

Building* Building::create(Core::BuildingType type, int level, int owner_id) {
    Building* building = new Building();
    if (building && building->init(type, level, owner_id)) {
        return building;
    }
    delete building;
    return nullptr;
}

bool Building::init(Core::BuildingType type, int level, int owner_id) {
    if (!BaseEntity::init()) {
        return false;
    }
    set_owner_id(owner_id);
    type_ = type;
    level_ = level;
    stats_ = MakeStats(type_, level_);
    current_state_ = Core::BuildingAnimationState::kIdle;
    stored_resource_ = 0.0f;
    if (type_ == Core::BuildingType::kGoldMine || type_ == Core::BuildingType::kElixirCollector) {
        stored_resource_ = static_cast<float>(stats_.resource_capacity_) / 2.0f;
    }
    construction_timer_ = 0.0f;
    construction_duration_ = 0.0f;
    obstacle_registered_ = false;
    return true;
}

void Building::onEnter() {
    BaseEntity::onEnter();
    obstacle_registered_ = true;
}

void Building::onExit() {
    obstacle_registered_ = false;
    BaseEntity::onExit();
}

void Building::update(float dt) {
    BaseEntity::update(dt);
    if (IsConstructing()) {
        construction_timer_ += dt;
        if (construction_duration_ > 0.0f && construction_timer_ >= construction_duration_) {
            construction_timer_ = construction_duration_;
            SetState(Core::BuildingAnimationState::kIdle);
        }
    } else {
        ProduceResource(dt);
    }
    UpdateCombatLogic(dt);
}

cocos2d::Rect Building::GetOccupiedRect() const {
    float width = static_cast<float>(stats_.width_) * static_cast<float>(Core::kTileWidth);
    float height = static_cast<float>(stats_.height_) * static_cast<float>(Core::kTileHeight);
    cocos2d::Vec2 center = getPosition();
    return cocos2d::Rect(center.x - (width / 2.0f), center.y - (height / 2.0f), width, height);
}

void Building::StartConstruction(float duration) {
    construction_duration_ = std::max(duration, 0.0f);
    construction_timer_ = 0.0f;
    SetState(Core::BuildingAnimationState::kConstructing);
}

float Building::GetConstructionProgress() const {
    if (!IsConstructing()) {
        return 1.0f;
    }
    if (construction_duration_ <= 0.0f) {
        return 1.0f;
    }
    return Clamp01(construction_timer_ / construction_duration_);
}

float Building::GetStoragePercentage() const {
    if (stats_.resource_capacity_ <= 0) {
        return 0.0f;
    }
    return Clamp01(stored_resource_ / static_cast<float>(stats_.resource_capacity_));
}

int Building::CollectResource(int max_amount) {
    int available = static_cast<int>(stored_resource_);
    int to_collect = available;
    if (max_amount >= 0) {
        to_collect = std::min(to_collect, max_amount);
    }
    stored_resource_ -= static_cast<float>(to_collect);
    if (stored_resource_ < 0.0f) {
        stored_resource_ = 0.0f;
    }
    return to_collect;
}

void Building::PlayAttackAnimation() {
    SetState(Core::BuildingAnimationState::kDamaged);
}

void Building::PlayHurtEffect() {
    SetState(Core::BuildingAnimationState::kDamaged);
}

void Building::PlayDestroyedAnimation() {
    SetState(Core::BuildingAnimationState::kDestroyed);
}

void Building::SetState(Core::BuildingAnimationState new_state) {
    current_state_ = new_state;
}

void Building::UpdateCombatLogic(float /*dt*/) {
    // Mock does not simulate combat rules.
}

void Building::ProduceResource(float dt) {
    if (stats_.production_rate_ <= 0 || stats_.resource_capacity_ <= 0) {
        return;
    }
    stored_resource_ += static_cast<float>(stats_.production_rate_) * (dt / 60.0f);
    float max_capacity = static_cast<float>(stats_.resource_capacity_);
    if (stored_resource_ > max_capacity) {
        stored_resource_ = max_capacity;
    }
}

std::string Building::GetSpriteSheetFilename(Core::BuildingType type) {
    switch (type) {
        case Core::BuildingType::kGoldMine:
            return "gold_mine.png";
        case Core::BuildingType::kElixirCollector:
            return "elixir_collector.png";
        default:
            return "building.png";
    }
}

void Building::GetFrameSize(Core::BuildingType /*type*/, int& out_width, int& out_height) {
    out_width = 1;
    out_height = 1;
}

Core::ProjectileType Building::GetProjectileTypeFromBuilding(Core::BuildingType type) {
    if (type == Core::BuildingType::kCannon) {
        return Core::ProjectileType::kCannonBall;
    }
    return Core::ProjectileType::kNone;
}

