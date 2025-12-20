// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of BaseEntity.

#include "Gameplay/Public/BaseEntity.h"

// 静态成员初始化
cocos2d::Vector<BaseEntity*> BaseEntity::global_entities_;

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

    // [修复] 时序模型集成 (Timing Model Integration)
    // 使用 Priority 0 确保实体逻辑（移动、状态）在 战斗仲裁（Priority 10）之前执行。
    // 避免 "Double-Step" 或 逻辑判定使用了旧坐标的问题。
    this->scheduleUpdateWithPriority(0);

    return true;
}

// [新增] 注册到全局列表
void BaseEntity::onEnter() {
    cocos2d::Node::onEnter(); // 必须调用父类
    global_entities_.pushBack(this);
}

// [新增] 从全局列表注销
void BaseEntity::onExit() {
    global_entities_.eraseObject(this);
    cocos2d::Node::onExit(); // 必须调用父类
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