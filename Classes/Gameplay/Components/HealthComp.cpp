// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of HealthComp.

#include "GamePlay/Public/HealthComp.h"
#include "GamePlay/Public/BaseEntity.h" // 引用 BaseEntity 以调用 MarkForDestruction

bool HealthComp::init() {
    if (!cocos2d::Node::init()) {
        return false;
    }

    current_hp_ = 100.0f;
    max_hp_ = 100.0f;
    is_dead_ = false;
    bar_offset_ = cocos2d::Vec2(0, 40); // 默认显示在中心点上方 40 像素

    // 创建绘制节点
    health_bar_node_ = cocos2d::DrawNode::create();
    this->addChild(health_bar_node_);

    // 初始状态：满血不显示血条
    health_bar_node_->setVisible(false);

    return true;
}

void HealthComp::InitStats(int max_hp) {
    max_hp_ = static_cast<float>(max_hp);
    current_hp_ = max_hp_;
    is_dead_ = false;

    // 确保重置时隐藏血条
    health_bar_node_->setVisible(false);
}
// 承受伤害
bool HealthComp::TakeDamage(int amount) {
    if (is_dead_) return true; // 已经死了，不要鞭尸

    current_hp_ -= amount;

    // 1. 只有受过伤才显示血条
    if (health_bar_node_->isVisible() == false) {
        health_bar_node_->setVisible(true);
    }

    // 2. 死亡判定
    if (current_hp_ <= 0) {
        current_hp_ = 0;
        is_dead_ = true;

        UpdateHealthBar(); // 绘制最后一次空血条
        return true; // 致命一击
    }

    // 3. 更新血条显示
    UpdateHealthBar();
    return false; // 还没死
}
//治疗扩展
void HealthComp::Heal(int amount) {
    if (is_dead_) return;

    current_hp_ += amount;
    if (current_hp_ > max_hp_) {
        current_hp_ = max_hp_;
        // 满血了，隐藏血条
        health_bar_node_->setVisible(false);
    }
    else {
        UpdateHealthBar();
    }
}
//计算血量百分比
float HealthComp::GetHealthPercentage() const {
    if (max_hp_ <= 0) return 0.0f;
    return current_hp_ / max_hp_;
}

void HealthComp::SetHealthBarOffset(cocos2d::Vec2 offset) {
    bar_offset_ = offset;
    // 强制刷新一次位置
    if (health_bar_node_->isVisible()) {
        UpdateHealthBar();
    }
}
//绘制血条
void HealthComp::UpdateHealthBar() {
    if (!health_bar_node_) return;

    health_bar_node_->clear();

    // 尺寸定义
    float width = 40.0f;  // 血条总宽
    float height = 6.0f;  // 血条高度
    float y_pos = bar_offset_.y;
    float x_start = bar_offset_.x - width / 2;

    // 绘制背景 (红色 - 代表扣掉的血)
    cocos2d::Vec2 bg_origin(x_start, y_pos);
    cocos2d::Vec2 bg_dest(x_start + width, y_pos + height);
    health_bar_node_->drawSolidRect(bg_origin, bg_dest, cocos2d::Color4F(1.0f, 0.0f, 0.0f, 1.0f)); // Red

    // 绘制前景 (绿色 - 代表当前血量)
    float pct = GetHealthPercentage();
    if (pct > 0) {
        cocos2d::Vec2 fg_origin(x_start, y_pos);
        cocos2d::Vec2 fg_dest(x_start + (width * pct), y_pos + height);
        health_bar_node_->drawSolidRect(fg_origin, fg_dest, cocos2d::Color4F(0.0f, 1.0f, 0.0f, 1.0f)); // Green
    }
}