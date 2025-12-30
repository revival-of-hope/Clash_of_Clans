#include "Contract/Engine/MapLayer.h"
#include <cmath>

MapLayer::~MapLayer()
{
    CC_SAFE_DELETE(_highlighter);
}

MapLayer* MapLayer::create(const std::string& tmxFile)
{
    auto ret = new (std::nothrow) MapLayer();
    if (ret && ret->initWithTMX(tmxFile))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
void MapLayer::fitPixelPerfect(float bottomMenuHeight, float rightMenuWidth)
{
    if (!_map) return;

    // 可视区域
    const Size visibleSize = Director::getInstance()->getVisibleSize();

    // 单个瓦片像素尺寸（逻辑尺寸）
    const Size tileSize = _map->getTileSize(); // e.g. 64 x 64

    /* --------------------------------------------------
     * Step 0: 计算可用于地图显示的区域（扣掉 UI）
     * -------------------------------------------------- */
    float usableWidth  = visibleSize.width  - rightMenuWidth;
    float usableHeight = visibleSize.height - bottomMenuHeight;

    // 防止 UI 把空间扣没（至少能显示 1 个瓦片）
    usableWidth  = std::max(usableWidth,  tileSize.width);
    usableHeight = std::max(usableHeight, tileSize.height);

    /* --------------------------------------------------
     * Step 1: 可完整显示的瓦片数量（向下取整）
     * -------------------------------------------------- */
    int tilesX = static_cast<int>(usableWidth  / tileSize.width);
    int tilesY = static_cast<int>(usableHeight / tileSize.height);

    tilesX = std::max(tilesX, 1);
    tilesY = std::max(tilesY, 1);

    /* --------------------------------------------------
     * Step 2: 基于瓦片数量反推缩放比例
     * -------------------------------------------------- */
    float scaleByWidth  = usableWidth  / (tilesX * tileSize.width);
    float scaleByHeight = usableHeight / (tilesY * tileSize.height);

    /* --------------------------------------------------
     * Step 3: 宁可裁剪，也不留黑边
     * -------------------------------------------------- */
    float finalScale = std::max(scaleByWidth, scaleByHeight);

    /* --------------------------------------------------
     * Step 4: 应用缩放
     * -------------------------------------------------- */
    _map->setScale(finalScale);

    /* --------------------------------------------------
     * Step 5: 左上角对齐
     * -------------------------------------------------- */
    _map->setAnchorPoint(Vec2(0.0f, 1.0f));

    // 像素对齐（避免亚像素抖动）
    _map->setPosition(Vec2(
        -0.5f,
        usableHeight + bottomMenuHeight + 0.5f
    ));
}

bool MapLayer::initWithTMX(const std::string& tmxFile)
{
    if (!Node::init()) return false;

    _map = TMXTiledMap::create(tmxFile);
    CCASSERT(_map, "Failed to load tmx file");

    this->addChild(_map);
    fitPixelPerfect();
    
    _highlighter = new TileHighlighter(_map);
    _highlighter->enable();
    CCASSERT(_highlighter != nullptr, "TileHighlighter creation failed");

    return true;
}
void MapLayer::setHighlightEnabled(bool enabled)
{
    if (!_highlighter) return;

    if (enabled)
        _highlighter->enable();
    else
        _highlighter->disable();
}
Vec2 MapLayer::WorldToTile(const Vec2& worldPos) const
{
    CCASSERT(_map || _testTileSize > 0.0f,
             "MapLayer requires TMX map or test tile size");

    // 1. 世界坐标 → map 本地坐标
    Vec2 localPos = _map
        ? _map->convertToNodeSpace(worldPos)
        : worldPos; // 纯测试模式（无 TMX）

    // 2. tile 尺寸（测试优先）
    const float tileSize = (_testTileSize > 0.0f)
        ? _testTileSize
        : _map->getTileSize().width;

    const float scale = _map ? _map->getScale() : 1.0f;

    // 3. anchor = (0,1)，Y 轴反向
    float x = localPos.x / (tileSize * scale);
    float y = -localPos.y / (tileSize * scale);

    // 4. 显式 floor，避免负值截断问题
    return Vec2(
        static_cast<int>(std::floor(x)),
        static_cast<int>(std::floor(y))
    );
}

Vec2 MapLayer::TileToWorldCenter(const Vec2& tilePos) const
{
    CCASSERT(_map, "TMXTiledMap is null");

    const Size tileSize = _map->getTileSize();
    const float scale = _map->getScale();

    // 1. tile → map 本地坐标（tile 左上角）
    Vec2 localPos(
        tilePos.x * tileSize.width * scale,
        -tilePos.y * tileSize.height * scale
    );

    // 2. 移动到 tile 中心
    localPos.x += tileSize.width * scale * 0.5f;
    localPos.y -= tileSize.height * scale * 0.5f;

    // 3. map 本地坐标 → 世界坐标
    return _map->convertToWorldSpace(localPos);
}
bool MapLayer::CanPlaceBuildingAt(const Vec2& tileCenterWorldPos) const
{
    if (!_map) return false;

    // 世界坐标 → tile 坐标
    Vec2 tile = WorldToTile(tileCenterWorldPos);

    Size mapSize = _map->getMapSize();

    // ① 边界检查
    if (tile.x < 0 || tile.y < 0 ||
        tile.x >= mapSize.width ||
        tile.y >= mapSize.height)
    {
        return false;
    }

    // ② 地形规则（预留：以后可接 TMX property）
    // 目前：地图范围内一律允许
    return true;
}

bool MapLayer::IsTileOccupied(const Vec2& tileCenterWorldPos) const
{
    if (!_map) return true;

    // 世界坐标 → tile 坐标
    Vec2 tile = WorldToTile(tileCenterWorldPos);
    if (!_blockedTiles.empty()){
        if (_blockedTiles.count(HashTileCoord(tile)) > 0)
            return true;
    }
    //  检查对象层（ObjectGroup）是否已有建筑
    const auto& groups = _map->getObjectGroups();
    for (const auto& group : groups)
    {
        const auto& objects = group->getObjects();
        for (const auto& obj : objects)
        {
            ValueMap dict = obj.asValueMap();

            float x = dict["x"].asFloat();
            float y = dict["y"].asFloat();
            float w = dict["width"].asFloat();
            float h = dict["height"].asFloat();

            Rect objRect(x, y, w, h);

            if (objRect.containsPoint(tileCenterWorldPos))
            {
                return true;
            }
        }
    }

    // ② （预留）以后可加：已放置建筑表 / 单位表
    return false;
}
long long MapLayer::HashTileCoord(const Vec2& tilePos)
{
    int x = static_cast<int>(tilePos.x);
    int y = static_cast<int>(tilePos.y);

    return (static_cast<long long>(x) << 32) |
           (static_cast<unsigned int>(y));
}
void MapLayer::SetTileSizeForTesting(float tileSize)
{
    CCASSERT(tileSize > 0.0f, "tileSize must be positive");
    _testTileSize = tileSize;
}
void MapLayer::SetBlockedTilesForTesting(const std::vector<Vec2>& blockedTiles)
{
    _blockedTiles.clear();

    for (const auto& tile : blockedTiles)
    {
        _blockedTiles.insert(HashTileCoord(tile));
    }
}
