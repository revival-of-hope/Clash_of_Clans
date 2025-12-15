#pragma once
#include "cocos2d.h"

USING_NS_CC;

class GameTileMap : public Node
{
public:
    static GameTileMap* create(const std::string& tmxFile);

    TMXTiledMap* getMap() const { return _map; }


protected:
    bool initWithTMX(const std::string& tmxFile);

private:
    TMXTiledMap* _map = nullptr;

    void fitPixelPerfect(float bottomMenuHeight = 0.0f,float rightMenuWidth = 0.0f);
};
