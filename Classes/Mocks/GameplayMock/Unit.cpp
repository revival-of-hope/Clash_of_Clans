#include "Classes/Contract/Gameplay/Unit.h"

#include <algorithm>
#include <string>

namespace {

Core::UnitStats MakeStats(Core::TroopType type, int level) {
    Core::UnitStats stats{};
    stats.max_hp_ = std::max(level, 1) * 80;
    stats.damage_per_shot_ = std::max(level, 1) * 8;
    stats.attack_speed_ = 1.0f;
    stats.move_speed_ = 1.5f;
    stats.range_ = 1.0f;
    stats.housing_space_ = 1;
    stats.favorite_target_ = Core::BuildingType::kTownHall;
    stats.projectile_ = Core::ProjectileType::kNone;
    stats.unit_type_ = Core::GeneralType::kGround;
    if (type == Core::TroopType::kBabyDragon) {
        stats.unit_type_ = Core::GeneralType::kAir;
        stats.projectile_ = Core::ProjectileType::kArrow;
    }
    stats.hitbox_def_.width_ = static_cast<float>(Core::kTileWidth);
    stats.hitbox_def_.height_ = static_cast<float>(Core::kTileHeight);
    return stats;
}

float Clamp01(float value) {
    return std::min(1.0f, std::max(0.0f, value));
}

}  // namespace

Unit* Unit::create(Core::TroopType type, int level, int owner_id) {
    Unit* unit = new Unit();
    if (unit && unit->init(type, level, owner_id)) {
        return unit;
    }
    delete unit;
    return nullptr;
}

bool Unit::init(Core::TroopType type, int level, int owner_id) {
    if (!BaseEntity::init()) {
        return false;
    }
    set_owner_id(owner_id);
    type_ = type;
    level_ = level;
    stats_ = MakeStats(type_, level_);
    current_state_ = Core::UnitAnimationState::kIdle;
    current_facing_ = Core::Facing::kRight;
    collision_radius_ = 20.0f;
    return true;
}

void Unit::onEnter() {
    BaseEntity::onEnter();
}

void Unit::update(float dt) {
    BaseEntity::update(dt);
    (void)dt;
}

cocos2d::Rect Unit::GetOccupiedRect() const {
    float diameter = collision_radius_ * 2.0f;
    cocos2d::Vec2 center = getPosition();
    return cocos2d::Rect(center.x - collision_radius_, center.y - collision_radius_, diameter, diameter);
}

bool Unit::CanAttack(Core::GeneralType target_type) const {
    return static_cast<unsigned int>(stats_.unit_type_) & static_cast<unsigned int>(target_type);
}

void Unit::SetState(Core::UnitAnimationState new_state) {
    current_state_ = new_state;
}

void Unit::SetFacing(Core::Facing facing) {
    current_facing_ = facing;
}

void Unit::PlayAttackAnimation() {
    SetState(Core::UnitAnimationState::kAttack);
}

void Unit::PlayHurtEffect() {
    SetState(Core::UnitAnimationState::kHit);
}

void Unit::PlayDeathAnimation() {
    SetState(Core::UnitAnimationState::kDead);
}

std::string Unit::GetSpriteSheetFilename(Core::TroopType type) {
    switch (type) {
        case Core::TroopType::kBabyDragon:
            return "baby_dragon.png";
        default:
            return "unit.png";
    }
}

void Unit::GetFrameSize(Core::TroopType /*type*/, int& out_width, int& out_height) {
    out_width = Core::kTileWidth;
    out_height = Core::kTileHeight;
}

