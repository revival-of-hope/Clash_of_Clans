#ifndef TILE_PLACEMENT_CONTROLLER_H
#define TILE_PLACEMENT_CONTROLLER_H

#include "cocos2d.h"
#include "GameTileMap.h"

USING_NS_CC;

class TilePlacementController
{
public:
    TilePlacementController(Node* owner);
    ~TilePlacementController();
    // ⭐ 核心接口：给“外部菜单图标”绑定放置功能
    void bindMenuIcon(
        Sprite* menuIcon,
        GameTileMap* map,
        const std::string& unitSpriteFile
    );

    void startPlacement(GameTileMap* map,
        const std::string& unitSpriteFile);

    void cancelPlacement();
    bool isPlacing() const;

private:
    void initMouseListener();
    void placeToTile(const Vec2& worldPos);
    bool worldPosToTileCenter(const Vec2& worldPos, Vec2& outTileCenter);

private:
    Node* _owner = nullptr;          // Scene / UI Root
    GameTileMap* _gameMap = nullptr;
    TMXTiledMap* _map = nullptr;

    Sprite* _dragSprite = nullptr;   
    bool _placing = false;

    EventListenerMouse* _mouseListener = nullptr;
};

#endif
