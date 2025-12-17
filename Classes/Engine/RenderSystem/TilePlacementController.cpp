#include "TilePlacementController.h"

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
    const std::string& unitSpriteFile)
{
    CCASSERT(map, "MapLayer is null");

    cancelPlacement();

    _gameMap = map;
    _map = map->getMap();

    // 创建“新单位”的拖拽预览精灵
    _dragSprite = Sprite::create(unitSpriteFile);
    CCASSERT(_dragSprite, "Failed to create unit sprite");

    _dragSprite->setOpacity(180);
    _gameMap->addChild(_dragSprite, 10);

    _placing = true;
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
            Vec2 worldPos = Director::getInstance()
                ->convertToGL(e->getLocationInView());

            Vec2 tileCenter;
            if (worldPosToTileCenter(worldPos, tileCenter))
            {
                _dragSprite->setPosition(tileCenter);
            }
        };

    _mouseListener->onMouseUp = [this](Event* event)
        {
            if (!_placing || !_dragSprite || !_gameMap) return;

            auto e = static_cast<EventMouse*>(event);
            Vec2 worldPos = Director::getInstance()
                ->convertToGL(e->getLocationInView());

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
    const Vec2& worldPos, Vec2& outTileCenter)
{
    Vec2 mapPos = _map->convertToNodeSpace(worldPos);

    Size tileSize = _map->getTileSize();
    Size mapSize  = _map->getMapSize();

    float mapPixelHeight = mapSize.height * tileSize.height;

    int col = mapPos.x / tileSize.width;
    int row = (mapPixelHeight - mapPos.y) / tileSize.height;

    if (col < 0 || row < 0 ||
        col >= mapSize.width || row >= mapSize.height)
        return false;

    outTileCenter.x = col * tileSize.width + tileSize.width / 2;
    outTileCenter.y = mapPixelHeight
                    - row * tileSize.height
                    - tileSize.height / 2;

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
            startPlacement(map, unitSpriteFile);
            return true;
        }
        return false;
        };

    _owner->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(listener, menuIcon);
}
