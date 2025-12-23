#ifndef __TILE_HIGHLIGHTER_H__
#define __TILE_HIGHLIGHTER_H__

#include "cocos2d.h"

class TileHighlighter
{
public:
    TileHighlighter(cocos2d::TMXTiledMap* map);
    ~TileHighlighter() = default;

    void enable();    
    void disable();  
    void updateHighlight(const cocos2d::Vec2& mousePos);
    cocos2d::Vec2 screenToTilePos(const cocos2d::Vec2& screenPos);

private:
    cocos2d::TMXTiledMap* _map = nullptr;
    cocos2d::EventListenerMouse* _listener = nullptr;
    cocos2d::DrawNode* _highlightNode = nullptr;

    
};

#endif // __TILE_HIGHLIGHTER_H__