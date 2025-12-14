// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of BaseEntity.

#include "BaseEntity.h"

// 构造函数
BaseEntity::BaseEntity() : instance_id_(-1)
, owner_id_(-1)//初始化无效ID
, is_marked_for_destruction_(false) {
}

// 析构函数
BaseEntity::~BaseEntity() {
}

// 初始化函数
bool BaseEntity::init() {
    //调用父类初始化
    if (!cocos2d::Node::init()) {
        return false;
    }
    //开启update,确保每一帧都能运行 update()
    this->scheduleUpdate();

    return true;
}

void BaseEntity::update(float dt) {
    // 这行代码确保了所有死亡的实体最终都会消失，不会变成僵尸
    // 如果被标记为销毁，则从父节点移除，Cocos 引用计数会自动处理内存释放
    if (is_marked_for_destruction_) {
        this->removeFromParent();
        return;
    }
}

// 获取中心点坐标
cocos2d::Vec2 BaseEntity::GetCenterPosition() const {
    return this->getPosition();
}