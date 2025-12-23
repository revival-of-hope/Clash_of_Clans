#ifndef GAME_TILE_MAP_H
#define GAME_TILE_MAP_H

#include "cocos2d.h"
#include "Engine/Public/TileHighlighter.h"
USING_NS_CC;

class MapLayer : public Node
{
public:
    static MapLayer* create(const std::string& tmxFile);
    ~MapLayer();
    TMXTiledMap* getMap() const { return _map; }
    void setHighlightEnabled(bool enabled);
protected:
    bool initWithTMX(const std::string& tmxFile);

private:
    TMXTiledMap* _map = nullptr;
    TileHighlighter* _highlighter = nullptr;
    void fitPixelPerfect(float bottomMenuHeight = 0.0f,
        float rightMenuWidth = 0.0f);
};

#endif // GAME_TILE_MAP_H
