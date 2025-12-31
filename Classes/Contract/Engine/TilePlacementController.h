// Source: Classes/Integration/Docs/Engine Module API Reference.md (TilePlacementController)
//         Classes/Integration/Docs/Placement Validation and TMX Conventions.md
#ifndef Contract_ENGINE_TILEPLACEMENTCONTROLLER_H_
#define Contract_ENGINE_TILEPLACEMENTCONTROLLER_H_

#include <string>

namespace cocos2d {
class Scene;
class Sprite;
struct Vec2;
}

class MapLayer;

class TilePlacementController {
public:
    explicit TilePlacementController(cocos2d::Scene* scene);

    void startPlacement(MapLayer* map, const std::string& unit_sprite_file, const cocos2d::Vec2& world_pos);

    void cancelPlacement();

    bool isPlacing() const;

    cocos2d::Vec2 SnapToValidTile(const cocos2d::Vec2& world_pos) const;
    bool CanPlaceAt(const cocos2d::Vec2& world_pos) const;

    void bindMenuIcon(cocos2d::Sprite* menu_icon, MapLayer* map, const std::string& unit_sprite_file);

private:
    cocos2d::Scene* scene_ = nullptr;
    cocos2d::Sprite* menu_icon_ = nullptr;
    MapLayer* map_ = nullptr;
    std::string unit_sprite_file_;
    bool is_placement_active_ = false;
};

#endif  // Contract_ENGINE_TILEPLACEMENTCONTROLLER_H_
