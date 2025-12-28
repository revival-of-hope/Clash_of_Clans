#include "Classes/Contract/Engine/MouseController.h"
#include "Classes/Contract/Engine/MapLayer.h"

MouseController::MouseController(MapLayer* map_layer) : map_layer_(map_layer) {}

void MouseController::enable() {
    is_enabled_ = true;
}

void MouseController::disable() {
    is_enabled_ = false;
}
