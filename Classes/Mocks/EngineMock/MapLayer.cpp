#include "Classes/Contract/Engine/MapLayer.h"

#include <cmath>

namespace {
int g_world_to_tile_call_count_for_testing = 0;
int g_tile_to_world_center_call_count_for_testing = 0;
}

MapLayer::MapLayer(const std::string& tmx_file) : tmx_file_(tmx_file) {}

MapLayer* MapLayer::create(const std::string& tmx_file) {
    return new MapLayer(tmx_file);
}

cocos2d::TMXTiledMap* MapLayer::getMap() const {
    return map_;
}

cocos2d::Vec2 MapLayer::WorldToTile(const cocos2d::Vec2& world_pos) const {
    ++g_world_to_tile_call_count_for_testing;
    if (tile_size_ <= 0.0f) {
        return cocos2d::Vec2(0.0f, 0.0f);
    }
    float tile_x = std::floor(world_pos.x / tile_size_);
    float tile_y = std::floor(world_pos.y / tile_size_);
    return cocos2d::Vec2(tile_x, tile_y);
}

cocos2d::Vec2 MapLayer::TileToWorldCenter(const cocos2d::Vec2& tile_coord) const {
    ++g_tile_to_world_center_call_count_for_testing;
    float center_x = tile_coord.x * tile_size_ + tile_size_ / 2.0f;
    float center_y = tile_coord.y * tile_size_ + tile_size_ / 2.0f;
    return cocos2d::Vec2(center_x, center_y);
}

bool MapLayer::IsTileBlocked(const cocos2d::Vec2& tile_coord) const {
    for (const auto& blocked : blocked_tiles_) {
        if (blocked.x == tile_coord.x && blocked.y == tile_coord.y) {
            return true;
        }
    }
    return false;
}

void MapLayer::SetBlockedTilesForTesting(const std::vector<cocos2d::Vec2>& blocked_tiles) {
    blocked_tiles_ = blocked_tiles;
}

void MapLayer::SetTileSizeForTesting(float tile_size) {
    tile_size_ = tile_size;
}

int GetWorldToTileCallCountForTesting() {
    return g_world_to_tile_call_count_for_testing;
}

int GetTileToWorldCenterCallCountForTesting() {
    return g_tile_to_world_center_call_count_for_testing;
}

void ResetMapLayerCallCountsForTesting() {
    g_world_to_tile_call_count_for_testing = 0;
    g_tile_to_world_center_call_count_for_testing = 0;
}
