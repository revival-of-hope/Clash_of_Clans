// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of BaseEntity.
//
// Path: Classes/Gameplay/Entities/BaseEntities.cpp

#include "Contract/GamePlay/BaseEntity.h"

cocos2d::Vector<BaseEntity*> BaseEntity::global_entities_;

BaseEntity::BaseEntity()
    : instance_id_(-1)
    , owner_id_(-1)
    , is_marked_for_destruction_(false) {
}

BaseEntity::~BaseEntity() {
}

bool BaseEntity::init() {
    if (!cocos2d::Node::init()) {
        return false;
    }

    this->scheduleUpdateWithPriority(0);

    return true;
}

void BaseEntity::onEnter() {
    cocos2d::Node::onEnter();
    global_entities_.pushBack(this);
}

void BaseEntity::onExit() {
    global_entities_.eraseObject(this);
    cocos2d::Node::onExit();
}

void BaseEntity::update(float dt) {
    if (is_marked_for_destruction_) {
        this->removeFromParent();
        return;
    }
}

cocos2d::Vec2 BaseEntity::GetCenterPosition() const {
    return this->getPosition();
}