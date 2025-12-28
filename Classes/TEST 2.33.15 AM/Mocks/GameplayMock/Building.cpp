#include "Classes/Contract/Gameplay/Building.h"

#include <unordered_map>

namespace {

std::unordered_map<const Building*, int>& StoredResources() {
    static std::unordered_map<const Building*, int> stored_resources;
    return stored_resources;
}

int GetStoredResourceFor(const Building* building) {
    auto& stored_resources = StoredResources();
    auto it = stored_resources.find(building);
    if (it == stored_resources.end()) {
        return 0;
    }
    return it->second;
}

void SetStoredResourceFor(const Building* building, int amount) {
    StoredResources()[building] = amount;
}

}  // namespace

Building::Building(Core::BuildingType type, int level, int owner_id)
    : type_(type), level_(level), owner_id_(owner_id) {
    if (type_ == Core::BuildingType::kGoldMine ||
        type_ == Core::BuildingType::kElixirCollector) {
        SetStoredResourceFor(this, level_ * 100);
    }
}

Building* Building::create(Core::BuildingType type, int level, int owner_id) {
    return new Building(type, level, owner_id);
}

void Building::StartConstruction(float duration) {
    is_constructing_ = true;
    construction_time_remaining_ = duration;
}

int Building::CollectResource(int max) {
    int available = GetStoredResourceFor(this);
    int collected = available;
    if (max >= 0 && collected > max) {
        collected = max;
    }
    SetStoredResourceFor(this, available - collected);
    return collected;
}

cocos2d::Rect Building::GetOccupiedRect() const {
    return cocos2d::Rect(0.0f, 0.0f, 0.0f, 0.0f);
}

int Building::GetLevel() const {
    return level_;
}

Core::BuildingType Building::GetBuildingType() const {
    return type_;
}

int Building::GetStoredResource() const {
    return GetStoredResourceFor(this);
}

bool Building::IsConstructing() const {
    return is_constructing_;
}
