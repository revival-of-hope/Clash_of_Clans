# Input Routing Policy (Dev C SOT)

Dev C scenes route pointer input through a UI-first router to avoid engine/gameplay coupling.

## Contract
* `InputRouter` (Contract path: `Classes/Contract/Engine/InputRouter.h`).
```
void AttachMap(MapLayer* map_layer, TileHighlighter* highlighter);
void SetUiConsumesInput(bool consume);
bool RoutePointerDown(const cocos2d::Vec2& screen_pos);
bool RoutePointerMove(const cocos2d::Vec2& screen_pos);
bool RoutePointerUp(const cocos2d::Vec2& screen_pos);
InputRouteState GetLastRouteState() const;
```
* Return value: `true` if the map received the event; `false` if UI consumed it first or no map was attached.

## Semantics
* **UI-first:** If `consume` is true, all routes return `false` and no map/highlighter updates occur (tests record zero MapLayer/TileHighlighter calls on this path).
* **Map routing:** When not consumed, events compute the snapped tile center via `MapLayer` (Core 64-unit tile grid by default) and enable the attached `TileHighlighter` (position remains UI-controlled).
* **State tracking:** `GetLastRouteState()` exposes the last routed world position and whether UI consumed the event for determinism in tests.
* **Null safety:** If no map is attached, routes are ignored and return `false`.
* **Implementation location:** Routing policy is implemented in durable Integration code; mocks must not own or redefine it.

## Usage example
```
InputRouter router;
router.AttachMap(map_layer, tile_highlighter);
router.SetUiConsumesInput(false);
router.RoutePointerMove(cocos2d::Vec2(12.f, 9.f));
InputRouteState last = router.GetLastRouteState();
// last.world_pos == snapped tile center, last.ui_consumed == false
```

## Error handling
* Negative coordinates are allowed and snapped via `MapLayer`.
* Router calls are no-ops when `map_layer` is null; the state still records `ui_consumed=true` for clarity.
