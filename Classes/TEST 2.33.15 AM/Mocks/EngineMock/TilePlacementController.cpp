#include "Contracts/Engine/TilePlacementController.h"
#include "Contracts/Engine/MapLayer.h"

TilePlacementController::TilePlacementController(cocos2d::Scene* scene) : scene_(scene) {}

void TilePlacementController::startPlacement(MapLayer* map,
                                             const std::string& unit_sprite_file,
                                             const cocos2d::Vec2& /*world_pos*/) {
    map_ = map;
    unit_sprite_file_ = unit_sprite_file;
    is_placement_active_ = true;
}

void TilePlacementController::cancelPlacement() {
    is_placement_active_ = false;
}

bool TilePlacementController::isPlacing() const {
    return is_placement_active_;
}

cocos2d::Vec2 TilePlacementController::SnapToValidTile(const cocos2d::Vec2& world_pos) const {
    if (map_ == nullptr) {
        return world_pos;
    }
    cocos2d::Vec2 tile = map_->WorldToTile(world_pos);
    return map_->TileToWorldCenter(tile);
}

bool TilePlacementController::CanPlaceAt(const cocos2d::Vec2& world_pos) const {
    if (map_ == nullptr) {
        return false;
    }
    cocos2d::Vec2 tile = map_->WorldToTile(world_pos);
    return !map_->IsTileBlocked(tile);
}

void TilePlacementController::bindMenuIcon(cocos2d::Sprite* menu_icon,
                                           MapLayer* map,
                                           const std::string& unit_sprite_file) {
    menu_icon_ = menu_icon;
    map_ = map;
    unit_sprite_file_ = unit_sprite_file;
}
