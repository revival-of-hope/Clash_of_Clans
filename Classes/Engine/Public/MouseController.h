#ifndef __MOUSE_CONTROLLER_H__
#define __MOUSE_CONTROLLER_H__

#include "cocos2d.h"
#include <functional>

class MouseController
{
public:
    MouseController(cocos2d::Node* target);
    ~MouseController() = default;

    void enable();
    void disable();

    // 获取鼠标对应的瓦片坐标（仅当 target 是 TMXTiledMap 时有效）
    cocos2d::Vec2 getTilePosAtScreenPos(const cocos2d::Vec2& screenPos);

    // 鼠标移动回调
    std::function<void(const cocos2d::Vec2&)> onMouseMoveCallback;


private:
    cocos2d::Node* target;
    cocos2d::EventListenerMouse* listener = nullptr;

    bool isDragging = false;
    cocos2d::Vec2 lastMousePos;

    void limitMapInsideScreen();
};

#endif
