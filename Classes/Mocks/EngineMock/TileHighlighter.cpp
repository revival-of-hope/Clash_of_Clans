#include "Classes/Contract/Engine/TileHighlighter.h"

namespace {
int g_tile_highlighter_enable_call_count_for_testing = 0;
}

TileHighlighter::TileHighlighter(cocos2d::TMXTiledMap* map) : map_(map) {}

void TileHighlighter::enable() {
    ++g_tile_highlighter_enable_call_count_for_testing;
    is_enabled_ = true;
}

void TileHighlighter::disable() {
    is_enabled_ = false;
}

int GetTileHighlighterEnableCallCountForTesting() {
    return g_tile_highlighter_enable_call_count_for_testing;
}

void ResetTileHighlighterCallCountsForTesting() {
    g_tile_highlighter_enable_call_count_for_testing = 0;
}
