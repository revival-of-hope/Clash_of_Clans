#include "Contracts/Engine/TileHighlighter.h"

TileHighlighter::TileHighlighter(cocos2d::TMXTiledMap* map) : map_(map) {}

void TileHighlighter::enable() {
    is_enabled_ = true;
}

void TileHighlighter::disable() {
    is_enabled_ = false;
}
