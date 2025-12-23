#include "Engine/MapSystem/TileHighlighter.h"
#include "Core/GameConstants.h"
USING_NS_CC;

TileHighlighter::TileHighlighter(TMXTiledMap* map)
{
    _map = map;

    // 创建高亮用的 DrawNode（纯色矩形）
    _highlightNode = DrawNode::create();
    _highlightNode->setVisible(false);  // 还没鼠标时隐藏
    _map->addChild(_highlightNode, static_cast<int>(Core::ZOrder::kShadows)); 
}

void TileHighlighter::enable()
{
    if (_listener) return;  // 防止重复添加

    _listener = EventListenerMouse::create();
    _listener->onMouseMove = [this](Event* event) {
        auto* e = dynamic_cast<EventMouse*>(event);
        if (!e) return;

        Vec2 mousePos(e->getCursorX(), e->getCursorY());
        updateHighlight(mousePos);
        };

    Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(_listener, _map);
}

void TileHighlighter::disable()
{
    if (_listener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_listener);
        _listener = nullptr;
    }
    if (_highlightNode) {
        _highlightNode->setVisible(false);
    }
}

void TileHighlighter::updateHighlight(const Vec2& mousePos)
{
    Vec2 tile = screenToTilePos(mousePos);

    if (tile.x < 0) {
        _highlightNode->setVisible(false);
        return;
    }

    //log("当前瓦片坐标: (%d, %d)", (int)tile.x, (int)tile.y);

    Size tileSize = _map->getTileSize();
    Size mapSize  = _map->getMapSize();

    float x = tile.x * tileSize.width;
    float y = (mapSize.height - 1 - tile.y) * tileSize.height;

    _highlightNode->clear();

    _highlightNode->drawSolidRect(
        Vec2(x, y),
        Vec2(x + tileSize.width, y + tileSize.height),
        Color4F(1.0f, 1.0f, 0.0f, 0.3f)
    );

    _highlightNode->drawRect(
        Vec2(x, y),
        Vec2(x + tileSize.width, y + tileSize.height),
        Color4F::WHITE
    );

    _highlightNode->setVisible(true);
}

Vec2 TileHighlighter::screenToTilePos(const Vec2& screenPos)
{
    if (!_map) return Vec2(-1, -1);
    Vec2 local = _map->convertToNodeSpace(screenPos);

    Size tileSize = _map->getTileSize();
    Size mapSize  = _map->getMapSize();

    int tileX = static_cast<int>(local.x / tileSize.width);
    int tileY = static_cast<int>(local.y / tileSize.height);

    // 转成 Tiled 左上角坐标系
    tileY = static_cast<int>(mapSize.height) - 1 - tileY;

    if (tileX < 0 || tileX >= mapSize.width ||
        tileY < 0 || tileY >= mapSize.height) {
        return Vec2(-1, -1);
    }

    return Vec2(tileX, tileY);
}
