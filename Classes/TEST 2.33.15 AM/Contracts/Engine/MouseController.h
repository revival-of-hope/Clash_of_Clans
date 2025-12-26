#ifndef CONTRACTS_ENGINE_MOUSECONTROLLER_H_
#define CONTRACTS_ENGINE_MOUSECONTROLLER_H_

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

#endif  // CONTRACTS_ENGINE_MOUSECONTROLLER_H_
