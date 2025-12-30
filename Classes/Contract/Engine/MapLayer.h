#ifndef GAME_TILE_MAP_H
#define GAME_TILE_MAP_H

#include "cocos2d.h"
#include "Classes/Contract/Engine/TileHighlighter.h"
#include <unordered_set>
USING_NS_CC;

class MapLayer : public Node
{
public:
    static MapLayer* create(const std::string& tmxFile);
    ~MapLayer();

    TMXTiledMap* getMap() const { return _map; }

    void setHighlightEnabled(bool enabled);

    // ================= 坐标转换接口 =================

    // 世界坐标（屏幕 / Node 空间） → 瓦片坐标 (tile x, y)
    Vec2 WorldToTile(const Vec2& worldPos) const;

    // 瓦片坐标 → 世界坐标（该 tile 的中心点）
    Vec2 TileToWorldCenter(const Vec2& tilePos) const;

    // ================= 放置判定接口 =================

    // 该 tile 是否允许放置建筑（地形 / 属性 / 图层规则）
    bool CanPlaceBuildingAt(const Vec2& tileCenterWorldPos) const;

    // 该 tile 是否已被建筑 / 单位占用
    bool IsTileOccupied(const Vec2& tileCenterWorldPos) const;
     // 强制覆盖 tile 尺寸（不依赖 TMX）
    void SetTileSizeForTesting(float tileSize);

    // 直接注入被阻塞的 tile 坐标集合
    void SetBlockedTilesForTesting(const std::vector<Vec2>& blockedTiles);

protected:
    bool initWithTMX(const std::string& tmxFile);

private:
    TMXTiledMap* _map = nullptr;
    TileHighlighter* _highlighter = nullptr;
    // 测试用 tile size（<= 0 表示使用 TMX 原始值）
    float _testTileSize = -1.0f;

    // 被占用 / 阻塞的 tile（tile 坐标）
    std::unordered_set<long long> _blockedTiles;
    static long long HashTileCoord(const Vec2& tilePos);
    void fitPixelPerfect(float bottomMenuHeight = 0.0f,
                         float rightMenuWidth = 0.0f);
};

#endif // GAME_TILE_MAP_H
