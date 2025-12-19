#include "Engine/Public/MouseController.h"

USING_NS_CC;

MouseController::MouseController(Node* target)
{
    this->target = target;
}

void MouseController::enable()
{
    listener = EventListenerMouse::create();

    listener->onMouseMove = [this](Event* event){
        EventMouse* e = (EventMouse*)event;
        Vec2 cursor(e->getCursorX(), e->getCursorY());

        // 回调
        if (this->onMouseMoveCallback)
            this->onMouseMoveCallback(cursor);

        // 拖动
        if(isDragging && target)
        {
            Vec2 delta = cursor - lastMousePos;
            target->setPosition(target->getPosition() + delta);
            lastMousePos = cursor;
            limitMapInsideScreen();
        }
    };

    listener->onMouseDown = [this](Event* event){
        EventMouse* e = (EventMouse*)event;
        if(e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
        {
            isDragging = true;
            lastMousePos = Vec2(e->getCursorX(), e->getCursorY());
        }
    };

    listener->onMouseUp = [this](Event* event){
        EventMouse* e = (EventMouse*)event;
        if(e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
            isDragging = false;
    };

    listener->onMouseScroll = [this](Event* event){
        EventMouse* e = (EventMouse*)event;
        if(!target) return;

        float scaleStep = 0.15f;
        float oldScale = target->getScale();
        float newScale = oldScale + (e->getScrollY() < 0 ? scaleStep : -scaleStep);
        newScale = clampf(newScale, 0.3f, 3.0f);

        if(fabs(newScale - oldScale) < 0.001f) return;

        Vec2 cursor(e->getCursorX(), e->getCursorY());
        Vec2 localBefore = target->convertToNodeSpace(cursor);
        target->setScale(newScale);
        Vec2 localAfter = target->convertToNodeSpace(cursor);

        target->setPosition(target->getPosition() - (localAfter - localBefore) * newScale);
        limitMapInsideScreen();
    };

    Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(listener, target);
}

void MouseController::disable()
{
    if(listener)
        Director::getInstance()->getEventDispatcher()->removeEventListener(listener);
}

Vec2 MouseController::getTilePosAtScreenPos(const Vec2& screenPos)
{
    TMXTiledMap* map = dynamic_cast<TMXTiledMap*>(target);
    if (!map) return Vec2(-1, -1);

    // 鼠标 → 地图本地坐标（自动支持缩放/拖拽）
    Vec2 local = map->convertToNodeSpace(screenPos);

    Size tileSize = map->getTileSize();
    Size mapSize = map->getMapSize();

    int tileX = local.x / tileSize.width;
    int tileY = (mapSize.height - 1) - (local.y / tileSize.height);

    if (tileX < 0 || tileX >= mapSize.width ||
        tileY < 0 || tileY >= mapSize.height)
        return Vec2(-1, -1);

    return Vec2(tileX, tileY);
}

void MouseController::limitMapInsideScreen()
{
    if(!target) return;

    TMXTiledMap* map = dynamic_cast<TMXTiledMap*>(target);
    if(!map) return;

    Size win = Director::getInstance()->getWinSize();
    Size mapSize = map->getMapSize();
    Size tileSize = map->getTileSize();
    float scale = map->getScale();

    float mapW = mapSize.width * tileSize.width * scale;
    float mapH = mapSize.height * tileSize.height * scale;

    Vec2 pos = map->getPosition();

    float minX = win.width - mapW;
    float maxX = 0;
    if(mapW <= win.width) pos.x = (win.width - mapW) * 0.5f;
    else pos.x = clampf(pos.x, minX, maxX);

    float minY = win.height - mapH;
    float maxY = 0;
    if(mapH <= win.height) pos.y = (win.height - mapH) * 0.5f;
    else pos.y = clampf(pos.y, minY, maxY);

    map->setPosition(pos);
}
