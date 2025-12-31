#ifndef Contract_ENGINE_MOUSECONTROLLER_H_
#define Contract_ENGINE_MOUSECONTROLLER_H_

class MapLayer;

class MouseController {
public:
    explicit MouseController(MapLayer* map_layer);

    void enable();
    void disable();

private:
    MapLayer* map_layer_ = nullptr;
    bool is_enabled_ = false;
};

#endif  // Contract_ENGINE_MOUSECONTROLLER_H_
