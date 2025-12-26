#ifndef CONTRACTS_ENGINE_TILEHIGHLIGHTER_H_
#define CONTRACTS_ENGINE_TILEHIGHLIGHTER_H_

namespace cocos2d {
class TMXTiledMap;
}

class TileHighlighter {
public:
    explicit TileHighlighter(cocos2d::TMXTiledMap* map);

    void enable();
    void disable();

private:
    cocos2d::TMXTiledMap* map_ = nullptr;
    bool is_enabled_ = false;
};

#endif  // CONTRACTS_ENGINE_TILEHIGHLIGHTER_H_
