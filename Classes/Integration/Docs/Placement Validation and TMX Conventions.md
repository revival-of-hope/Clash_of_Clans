# Placement Validation and TMX Conventions (Dev C SOT)

This document defines Dev C-owned placement rules for map-driven building/troop placement.

## TMX map conventions
* **Blocked layer name:** `collision` (tile layer). Tiles with property `blocked=true` are not placeable.
* **Optional override:** A tile property `walkable=false` is treated as blocked.
* **Tile size:** 64 world units per tile (Core::kTileWidth/kTileHeight). Maps that override tile size must stay consistent with Core constants; mocks default to 64.

## Contract
* `MapLayer` (Contract path: `Classes/Contract/Engine/MapLayer.h`)
  * `cocos2d::Vec2 WorldToTile(const cocos2d::Vec2& world_pos) const;`
  * `cocos2d::Vec2 TileToWorldCenter(const cocos2d::Vec2& tile_coord) const;`
  * `bool IsTileBlocked(const cocos2d::Vec2& tile_coord) const;`
  * `void SetBlockedTilesForTesting(const std::vector<cocos2d::Vec2>& blocked_tiles);` (mock-only helper)
  * `void SetTileSizeForTesting(float tile_size);` (mock-only helper)
* `TilePlacementController` (Contract path: `Classes/Contract/Engine/TilePlacementController.h`)
  * `cocos2d::Vec2 SnapToValidTile(const cocos2d::Vec2& world_pos) const;`
  * `bool CanPlaceAt(const cocos2d::Vec2& world_pos) const;`
  * Existing `startPlacement`/`cancelPlacement`/`isPlacing` semantics are unchanged.

## Semantics
* `WorldToTile` uses floor division by tile size (x,y) using the Core 64-unit tile size unless overridden. Negative coordinates floor toward negative infinity.
* `TileToWorldCenter` returns the center of the tile in world units using the same tile size.
* `IsTileBlocked` checks the `collision` layer first; if missing, all tiles are treated as walkable.
* `SnapToValidTile` converts `world_pos` to the nearest tile center. If blocked, returns the same snapped position but `CanPlaceAt` returns `false`.
* Mock-only `SetBlockedTilesForTesting` replaces the blocked set deterministically for tests.

## Usage example
```
auto map = MapLayer::create("maps/mock.tmx");
auto placement = new TilePlacementController(scene);
cocos2d::Vec2 snapped = placement->SnapToValidTile(cocos2d::Vec2(15.f, 18.f));
bool allowed = placement->CanPlaceAt(snapped);
if (allowed) {
    placement->startPlacement(map, "Troops_Icon/Archer.png", snapped);
}
```

## Error handling
* If `MapLayer` has no map data (nullptr), `IsTileBlocked` always returns `false` and snapping still centers on the tile grid.
* Passing `nullptr` map pointers into `TilePlacementController` results in `CanPlaceAt` returning `false`.
