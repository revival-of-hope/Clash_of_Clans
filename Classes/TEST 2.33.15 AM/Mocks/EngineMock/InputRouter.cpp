#include "Contracts/Engine/InputRouter.h"

#include "Contracts/Engine/MapLayer.h"
#include "Contracts/Engine/TileHighlighter.h"

void InputRouter::AttachMap(MapLayer* map_layer, TileHighlighter* highlighter) {
    map_layer_ = map_layer;
    highlighter_ = highlighter;
}

void InputRouter::SetUiConsumesInput(bool consume) {
    ui_consumes_input_ = consume;
}

bool InputRouter::RoutePointerDown(const cocos2d::Vec2& screen_pos) {
    return Route(screen_pos);
}

bool InputRouter::RoutePointerMove(const cocos2d::Vec2& screen_pos) {
    return Route(screen_pos);
}

bool InputRouter::RoutePointerUp(const cocos2d::Vec2& screen_pos) {
    return Route(screen_pos);
}

InputRouteState InputRouter::GetLastRouteState() const {
    return last_state_;
}

bool InputRouter::Route(const cocos2d::Vec2& screen_pos) {
    last_state_.world_pos = screen_pos;
    if (ui_consumes_input_) {
        last_state_.ui_consumed = true;
        return false;
    }
    if (map_layer_ == nullptr) {
        last_state_.ui_consumed = true;
        return false;
    }

    cocos2d::Vec2 tile = map_layer_->WorldToTile(screen_pos);
    last_state_.world_pos = map_layer_->TileToWorldCenter(tile);
    last_state_.ui_consumed = false;
    if (highlighter_ != nullptr) {
        highlighter_->enable();
    }
    return true;
}
