#ifndef TILE_PLACEMENT_CONTROLLER_H
#define TILE_PLACEMENT_CONTROLLER_H

#include "cocos2d.h"
#include "Engine/Public Function/MapLayer.h"

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

    void startPlacement(MapLayer* map,
        const std::string& unitSpriteFile,
        const Vec2& worldPos);

    void cancelPlacement();
    bool isPlacing() const;

private:
    void initMouseListener();
    void placeToTile(const Vec2& worldPos);
    bool worldPosToTileCenter(const Vec2& worldPos, Vec2& outTileCenter);

private:
    Node* _owner = nullptr;          // Scene / UI Root
    MapLayer* _gameMap = nullptr;
    TMXTiledMap* _map = nullptr;

    Sprite* _dragSprite = nullptr;   
    bool _placing = false;

    EventListenerMouse* _mouseListener = nullptr;
};

#endif
