// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of HealthComp.
//
// Path: Classes/Gameplay/Components/HealthComp.cpp

#include "Contract/GamePlay/HealthComp.h"
#include "Contract/GamePlay/BaseEntity.h"
#include "Contract/GamePlay/Unit.h"
#include "Contract/GamePlay/Building.h"

bool HealthComp::init() {
    if (!cocos2d::Node::init()) return false;

    current_hp_ = 100.0f;
    max_hp_ = 100.0f;
    is_dead_ = false;
    bar_offset_ = cocos2d::Vec2(0, 40);

    health_bar_node_ = cocos2d::DrawNode::create();
    this->addChild(health_bar_node_);
    health_bar_node_->setVisible(false);

    return true;
}

void HealthComp::InitStats(int max_hp) {
    max_hp_ = static_cast<float>(max_hp);
    current_hp_ = max_hp_;
    is_dead_ = false;
    health_bar_node_->setVisible(false);
}

bool HealthComp::TakeDamage(int amount) {
    if (is_dead_) return true;

    current_hp_ -= amount;

    if (!health_bar_node_->isVisible()) {
        health_bar_node_->setVisible(true);
    }

    auto parent = this->getParent();

    if (auto unit = dynamic_cast<Unit*>(parent)) {
        if (!is_dead_) unit->PlayHurtEffect();
    }
    else if (auto building = dynamic_cast<Building*>(parent)) {
        if (!is_dead_) building->PlayHurtEffect();
    }

    if (current_hp_ <= 0) {
        current_hp_ = 0;
        is_dead_ = true;

        if (auto unit = dynamic_cast<Unit*>(parent)) {
            unit->PlayDeathAnimation();
        }
        else if (auto building = dynamic_cast<Building*>(parent)) {
            building->PlayDestroyedAnimation();
        }

        UpdateHealthBar();
        return true;
    }

    UpdateHealthBar();
    return false;
}

void HealthComp::Heal(int amount) {
    if (is_dead_) return;

    current_hp_ += amount;
    if (current_hp_ > max_hp_) {
        current_hp_ = max_hp_;
        health_bar_node_->setVisible(false);
    }
    else {
        UpdateHealthBar();
    }
}

float HealthComp::GetHealthPercentage() const {
    if (max_hp_ <= 0) return 0.0f;
    return current_hp_ / max_hp_;
}

void HealthComp::SetHealthBarOffset(cocos2d::Vec2 offset) {
    bar_offset_ = offset;
    if (health_bar_node_->isVisible()) {
        UpdateHealthBar();
    }
}

void HealthComp::UpdateHealthBar() {
    if (!health_bar_node_) return;

    health_bar_node_->clear();

    float width = 40.0f;
    float height = 6.0f;
    float y_pos = bar_offset_.y;
    float x_start = bar_offset_.x - width / 2;

    health_bar_node_->drawSolidRect(
        cocos2d::Vec2(x_start, y_pos),
        cocos2d::Vec2(x_start + width, y_pos + height),
        cocos2d::Color4F(1.0f, 0.0f, 0.0f, 1.0f)
    );

    float pct = GetHealthPercentage();
    if (pct > 0) {
        health_bar_node_->drawSolidRect(
            cocos2d::Vec2(x_start, y_pos),
            cocos2d::Vec2(x_start + (width * pct), y_pos + height),
            cocos2d::Color4F(0.0f, 1.0f, 0.0f, 1.0f)
        );
    }
}