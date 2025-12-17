#include "MapLayer.h"
#include "TileHighlighter.h"


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
    auto highlighter = new TileHighlighter(_map);
    highlighter->enable();

    return true;
}
