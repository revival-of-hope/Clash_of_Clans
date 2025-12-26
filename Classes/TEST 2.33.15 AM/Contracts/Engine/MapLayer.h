#ifndef CONTRACTS_ENGINE_MAPLAYER_H_
#define CONTRACTS_ENGINE_MAPLAYER_H_

#include <string>
#include <vector>

#include "Core/GameConstants.h"
#include "cocos2d.h"

// Source: Classes/Integration/Docs/Engine Module API Reference.md (MapLayer)
//         Classes/Integration/Docs/Placement Validation and TMX Conventions.md
class MapLayer : public cocos2d::Node {
public:
    static MapLayer* create(const std::string& tmx_file);

    cocos2d::TMXTiledMap* getMap() const;

    cocos2d::Vec2 WorldToTile(const cocos2d::Vec2& world_pos) const;
    cocos2d::Vec2 TileToWorldCenter(const cocos2d::Vec2& tile_coord) const;
    bool IsTileBlocked(const cocos2d::Vec2& tile_coord) const;
    void SetBlockedTilesForTesting(const std::vector<cocos2d::Vec2>& blocked_tiles);
    void SetTileSizeForTesting(float tile_size);

private:
    explicit MapLayer(const std::string& tmx_file);

    std::string tmx_file_;
    cocos2d::TMXTiledMap* map_ = nullptr;
    float tile_size_ = static_cast<float>(Core::kTileWidth);
    std::vector<cocos2d::Vec2> blocked_tiles_;
};

#endif  // CONTRACTS_ENGINE_MAPLAYER_H_
