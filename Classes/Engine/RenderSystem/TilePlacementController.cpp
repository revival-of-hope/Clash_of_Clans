#include "Contract/Engine/TilePlacementController.h"
#include "Core/GameConstants.h"
TilePlacementController::TilePlacementController(Node* owner)
    : _owner(owner)
{
    CCASSERT(_owner, "Owner is null");
    initMouseListener();
}

TilePlacementController::~TilePlacementController()
{
    cancelPlacement();

    if (_mouseListener)
    {
        Director::getInstance()->getEventDispatcher()
            ->removeEventListener(_mouseListener);
        _mouseListener = nullptr;
    }
}

void TilePlacementController::startPlacement(
    MapLayer* map,
    const std::string& unitSpriteFile,
    const Vec2& worldPos)
{
    CCASSERT(map, "MapLayer is null");

    cancelPlacement();

    _gameMap = map;
    _map = map->getMap();

    // 创建“新单位”的拖拽预览精灵
    _dragSprite = Sprite::create(unitSpriteFile);
    CCASSERT(_dragSprite, "Failed to create unit sprite");
    _dragSprite->setOpacity(180);
    _gameMap->addChild(_dragSprite, static_cast<int>(Core::ZOrder::kUnits));
    _placing = true;

    // 精灵直接出现在鼠标位置
    Vec2 localPos = _gameMap->convertToNodeSpace(worldPos);
    _dragSprite->setPosition(localPos);
}

void TilePlacementController::cancelPlacement()
{
    _placing = false;

    if (_dragSprite)
    {
        _dragSprite->removeFromParent();
        _dragSprite = nullptr;
    }

    _gameMap = nullptr;
    _map = nullptr;
}

bool TilePlacementController::isPlacing() const
{
    return _placing;
}


void TilePlacementController::initMouseListener()
{
    _mouseListener = EventListenerMouse::create();

    _mouseListener->onMouseMove = [this](Event* event)
        {
            if (!_placing || !_dragSprite || !_map) return;

            auto e = static_cast<EventMouse*>(event);
            Vec2 worldPos(e->getCursorX(), e->getCursorY());

            // 转成地图节点坐标
            Vec2 localPos = _gameMap->convertToNodeSpace(worldPos);

            // 精灵永远用“父节点坐标”
            _dragSprite->setPosition(localPos);
        };

    _mouseListener->onMouseUp = [this](Event* event)
        {
            if (!_placing || !_dragSprite || !_gameMap) return;

            auto e = static_cast<EventMouse*>(event);
            Vec2 worldPos(e->getCursorX(), e->getCursorY());

            placeToTile(worldPos);
        };

    Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(_mouseListener, _owner);
}


void TilePlacementController::placeToTile(const Vec2& worldPos)
{
    Vec2 tileCenter;
    if (!worldPosToTileCenter(worldPos, tileCenter))
        return;
    // 1️⃣ 先保存指针
    auto placedSprite = _dragSprite;

    // 2️⃣ 立刻切换状态，阻止后续 mouseMove
    _dragSprite = nullptr;
    _placing = false;
    // 4️⃣ 恢复外观
    placedSprite->setOpacity(255);
    placedSprite->setPosition(tileCenter);
}


bool TilePlacementController::worldPosToTileCenter(
    const Vec2& worldPos,
    Vec2& outTileCenter)const
{
    // ① 世界坐标 → 地图本地坐标
    Vec2 local = _map->convertToNodeSpace(worldPos);

    Size tileSize = _map->getTileSize();
    Size mapSize = _map->getMapSize();

    int tileX = local.x / tileSize.width;
    int tileY = local.y / tileSize.height;

    // 转成 Tiled 左上角坐标
    tileY = mapSize.height - 1 - tileY;

    if (tileX < 0 || tileX >= mapSize.width ||
        tileY < 0 || tileY >= mapSize.height)
        return false;

    // ② 再从“瓦片坐标”算中心点
    float centerX = tileX * tileSize.width + tileSize.width * 0.5f;
    float centerY =
        (mapSize.height - 1 - tileY) * tileSize.height
        + tileSize.height * 0.5f;
    Vec2 mapLocalCenter(centerX, centerY);

    // 把 map 本地坐标 → world
    Vec2 worldCenter = _map->convertToWorldSpace(mapLocalCenter);

    // 再 world → gameMap 本地
    outTileCenter = _gameMap->convertToNodeSpace(worldCenter);
    
    return true;
}

void TilePlacementController::bindMenuIcon(
    Sprite* menuIcon,
    MapLayer* map,
    const std::string& unitSpriteFile)
{
    CCASSERT(menuIcon, "Menu icon is null");
    CCASSERT(map, "MapLayer is null");

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [=](Touch* t, Event*) {
        if (menuIcon->getBoundingBox().containsPoint(t->getLocation()))
        {
            startPlacement(map, unitSpriteFile, t->getLocation());
            return true;
        }
        return false;
        };

    _owner->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(listener, menuIcon);
}
bool TilePlacementController::CanPlaceAt(const Vec2& worldPos) const
{
    Vec2 tileCenter;
    if (!worldPosToTileCenter(worldPos, tileCenter))
        return false;

    // ① 地图逻辑判断
    if (!_gameMap->CanPlaceBuildingAt(tileCenter))
        return false;

    // ② 占用检测（建筑 / 单位）
    if (_gameMap->IsTileOccupied(tileCenter))
        return false;

    return true;
}
Vec2 TilePlacementController::SnapToValidTile(const Vec2& worldPos) const
{
    // 没在放置状态，或地图无效，直接返回原坐标
    if (!_placing || !_gameMap || !_map)
        return worldPos;

    Vec2 tileCenter;

    // ① 先尝试吸附到 tile 中心（几何层）
    if (!worldPosToTileCenter(worldPos, tileCenter))
        return worldPos;

    // ② 再判断是否允许放置（规则层）
    if (!CanPlaceAt(tileCenter))
        return worldPos;

    // ③ 合法，返回吸附后的中心点
    return tileCenter;
}
