#include "Classes/Contract/Gameplay/Unit.h"

#include <unordered_map>

namespace {

std::unordered_map<const Unit*, Core::UnitAnimationState>& UnitStates() {
    static std::unordered_map<const Unit*, Core::UnitAnimationState> states;
    return states;
}

std::unordered_map<const Unit*, int>& UnitDamages() {
    static std::unordered_map<const Unit*, int> damages;
    return damages;
}

}  // namespace

Unit::Unit(Core::TroopType type, int level, int owner_id)
    : type_(type), level_(level), owner_id_(owner_id) {
    UnitStates()[this] = Core::UnitAnimationState::kIdle;
    int base_damage = level_ > 0 ? level_ * 10 : 1;
    UnitDamages()[this] = base_damage;
}

Unit* Unit::create(Core::TroopType type, int level, int owner_id) {
    return new Unit(type, level, owner_id);
}

Core::TroopType Unit::GetTroopType() const {
    return type_;
}

Core::GeneralType Unit::GetGeneralType() const {
    if (type_ == Core::TroopType::kBabyDragon) {
        return Core::GeneralType::kAir;
    }
    return Core::GeneralType::kGround;
}

Core::UnitAnimationState Unit::GetState() const {
    auto& states = UnitStates();
    auto it = states.find(this);
    if (it == states.end()) {
        return Core::UnitAnimationState::kIdle;
    }
    return it->second;
}

int Unit::GetDamage() const {
    auto& damages = UnitDamages();
    auto it = damages.find(this);
    if (it == damages.end()) {
        return 0;
    }
    return it->second;
}

int Unit::GetLevel() const {
    return level_;
}

bool Unit::CanAttack(Core::GeneralType /*target_type*/) const {
    return true;
}

cocos2d::Rect Unit::GetOccupiedRect() const {
    auto position = getPosition();
    float width = static_cast<float>(Core::kTileWidth);
    float height = static_cast<float>(Core::kTileHeight);
    return cocos2d::Rect(position.x - (width / 2.0f),
                         position.y - (height / 2.0f),
                         width,
                         height);
}

void Unit::SetFacing(Core::Facing facing) {
    facing_ = facing;
}
