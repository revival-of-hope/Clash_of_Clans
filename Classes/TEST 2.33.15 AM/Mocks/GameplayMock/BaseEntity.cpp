#include "Contracts/Gameplay/BaseEntity.h"

namespace {

bool ContainsEntity(const cocos2d::Vector<BaseEntity*>& entities,
                    const BaseEntity* target) {
    for (const auto* entity : entities) {
        if (entity == target) {
            return true;
        }
    }
    return false;
}

void RemoveEntity(cocos2d::Vector<BaseEntity*>& entities,
                  const BaseEntity* target) {
    cocos2d::Vector<BaseEntity*> remaining;
    for (auto* entity : entities) {
        if (entity != target) {
            remaining.pushBack(entity);
        }
    }
    entities = remaining;
}

}  // namespace

cocos2d::Vector<BaseEntity*> BaseEntity::global_entities_;

BaseEntity::BaseEntity() : instance_id_(0), is_marked_for_destruction_(false), owner_id_(0) {}

BaseEntity::~BaseEntity() = default;

bool BaseEntity::init() {
    return Node::init();
}

void BaseEntity::onEnter() {
    Node::onEnter();
    if (!ContainsEntity(global_entities_, this)) {
        global_entities_.pushBack(this);
    }
}

void BaseEntity::onExit() {
    RemoveEntity(global_entities_, this);
    Node::onExit();
}

void BaseEntity::update(float dt) {
    Node::update(dt);
    if (is_marked_for_destruction_) {
        RemoveEntity(global_entities_, this);
    }
}

cocos2d::Vec2 BaseEntity::GetCenterPosition() const {
    return getPosition();
}
