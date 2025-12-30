#ifndef TILE_PLACEMENT_CONTROLLER_H
#define TILE_PLACEMENT_CONTROLLER_H

#include "cocos2d.h"
#include "Contract/Engine/MapLayer.h"

USING_NS_CC;

class TilePlacementController
{
public:
    TilePlacementController(Node* owner);
    ~TilePlacementController();

    void bindMenuIcon(
        Sprite* menuIcon,
        MapLayer* map,
        const std::string& unitSpriteFile
    );

    void startPlacement(
        MapLayer* map,
        const std::string& unitSpriteFile,
        const Vec2& worldPos
    );

    void cancelPlacement();
    bool isPlacing() const;
    bool CanPlaceAt(const Vec2& worldPos) const;

    // ================= 吸附 / 对齐接口 =================

    // 将任意世界坐标吸附到最近的「可放置 tile 中心」
    // 若无合法 tile，返回原 worldPos
    Vec2 SnapToValidTile(const Vec2& worldPos) const;

private:
    void initMouseListener();
    void placeToTile(const Vec2& worldPos);

    // worldPos → tile 中心（不做合法性判断）
    bool worldPosToTileCenter(
        const Vec2& worldPos,
        Vec2& outTileCenter
    ) const;

private:
    Node* _owner = nullptr;          // Scene / UI Root
    MapLayer* _gameMap = nullptr;
    TMXTiledMap* _map = nullptr;

    Sprite* _dragSprite = nullptr;
    bool _placing = false;

    EventListenerMouse* _mouseListener = nullptr;
};

#endif // TILE_PLACEMENT_CONTROLLER_H
 