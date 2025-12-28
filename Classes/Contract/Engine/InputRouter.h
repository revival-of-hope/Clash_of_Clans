// Source: Classes/Integration/Docs/Input Routing Policy.md (InputRouter)
#ifndef Contract_ENGINE_INPUTROUTER_H_
#define Contract_ENGINE_INPUTROUTER_H_

#include "cocos2d.h"

class MapLayer;
class TileHighlighter;

struct InputRouteState {
    cocos2d::Vec2 world_pos;
    bool ui_consumed = true;
};

class InputRouter {
public:
    InputRouter() = default;

    void AttachMap(MapLayer* map_layer, TileHighlighter* highlighter);
    void SetUiConsumesInput(bool consume);

    bool RoutePointerDown(const cocos2d::Vec2& screen_pos);
    bool RoutePointerMove(const cocos2d::Vec2& screen_pos);
    bool RoutePointerUp(const cocos2d::Vec2& screen_pos);

    InputRouteState GetLastRouteState() const;

private:
    bool Route(const cocos2d::Vec2& screen_pos);

    MapLayer* map_layer_ = nullptr;
    TileHighlighter* highlighter_ = nullptr;
    bool ui_consumes_input_ = true;
    InputRouteState last_state_{};
};

#endif  // Contract_ENGINE_INPUTROUTER_H_
