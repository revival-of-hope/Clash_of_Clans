// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of EntityAnimationController.
//
// Path: Classes/Gameplay/Components/EntityAnimationController.cpp

#include "Gameplay/Components/EntityAnimationController.h"

bool EntityAnimationController::init() {
    if (!cocos2d::Node::init()) return false;
    this->scheduleUpdate();
    return true;
}

void EntityAnimationController::InitWithSprite(cocos2d::Sprite* sprite,
    const std::string& sprite_sheet_path,
    LayoutType layout,
    int frame_width,
    int frame_height) {
    target_sprite_ = sprite;
    sprite_sheet_path_ = sprite_sheet_path;
    layout_type_ = layout;
    frame_width_ = frame_width;
    frame_height_ = frame_height;

    cols_ = (layout == LayoutType::kUnit) ? 4 : 2;

    sprite_sheet_ = cocos2d::Director::getInstance()
        ->getTextureCache()
        ->addImage(sprite_sheet_path);

    if (!sprite_sheet_) {
        cocos2d::log("EntityAnimationController: Failed to load: %s", sprite_sheet_path.c_str());
        return;
    }

    if (layout == LayoutType::kUnit) {
        std::vector<int> frames = { 0, 1, 4, 5, 8, 9, 12, 13, 14 };
        for (int idx : frames) GetFrameAt(idx);
    }
    else {
        for (int i = 0; i < 4; ++i) GetFrameAt(i);
    }

    UpdateCurrentFrame();
}

cocos2d::SpriteFrame* EntityAnimationController::GetFrameAt(int index) {
    auto it = frame_cache_.find(index);
    if (it != frame_cache_.end()) return it->second;

    if (!sprite_sheet_) return nullptr;

    int col = index % cols_;
    int row = index / cols_;

    cocos2d::Rect rect(
        col * frame_width_,
        row * frame_height_,
        frame_width_,
        frame_height_
    );

    auto frame = cocos2d::SpriteFrame::createWithTexture(sprite_sheet_, rect);
    if (frame) {
        frame->retain();
        frame_cache_[index] = frame;
    }
    return frame;
}

void EntityAnimationController::update(float dt) {
    if (!target_sprite_ || is_dead_) return;

    if (is_hurt_flashing_) {
        hurt_flash_timer_ -= dt;
        if (hurt_flash_timer_ <= 0.0f) {
            is_hurt_flashing_ = false;
            target_sprite_->setColor(cocos2d::Color3B::WHITE);
            UpdateCurrentFrame();
        }
        return;
    }

    if (is_playing_attack_) {
        attack_timer_ -= dt;

        if (layout_type_ == LayoutType::kUnit) {
            float half = attack_duration_ / 2.0f;
            int new_phase = (attack_timer_ > half) ? 1 : 0;

            if (new_phase != attack_frame_phase_) {
                attack_frame_phase_ = new_phase;
                if (attack_frame_phase_ == 1) {
                    auto f = GetFrameAt(static_cast<int>(UnitFrameIndex::kAttack));
                    if (f) target_sprite_->setSpriteFrame(f);
                }
                else {
                    auto f = GetFrameAt(GetUnitDirectionalFrameIndex(false));
                    if (f) target_sprite_->setSpriteFrame(f);
                }
            }
        }

        if (attack_timer_ <= 0.0f) {
            is_playing_attack_ = false;
            attack_frame_phase_ = 0;
            target_sprite_->setColor(cocos2d::Color3B::WHITE);
            UpdateCurrentFrame();
        }
        return;
    }

    if (layout_type_ == LayoutType::kUnit) {
        animation_timer_ += dt;
        if (animation_timer_ >= animation_interval_) {
            animation_timer_ = 0.0f;
            is_frame_b_ = !is_frame_b_;
            UpdateCurrentFrame();
        }
    }
}

void EntityAnimationController::PlayAttackAnimation(float duration) {
    if (is_dead_ || !target_sprite_) return;

    is_playing_attack_ = true;
    attack_duration_ = duration;
    attack_timer_ = duration;
    attack_frame_phase_ = 1;

    int attack_idx = (layout_type_ == LayoutType::kUnit)
        ? static_cast<int>(UnitFrameIndex::kAttack)
        : static_cast<int>(BuildingFrameIndex::kAttack);

    auto frame = GetFrameAt(attack_idx);
    if (frame) {
        target_sprite_->setSpriteFrame(frame);
    }

    target_sprite_->setColor(cocos2d::Color3B(255, 230, 230));
}

void EntityAnimationController::PlayHurtFlash() {
    if (is_dead_ || !target_sprite_) return;

    is_hurt_flashing_ = true;
    hurt_flash_timer_ = (layout_type_ == LayoutType::kUnit) ? 0.15f : 0.2f;

    int hurt_idx = (layout_type_ == LayoutType::kUnit)
        ? static_cast<int>(UnitFrameIndex::kHurt)
        : static_cast<int>(BuildingFrameIndex::kHurt);

    auto frame = GetFrameAt(hurt_idx);
    if (frame) {
        target_sprite_->setSpriteFrame(frame);
    }

    target_sprite_->setColor(cocos2d::Color3B(255, 80, 80));
}

void EntityAnimationController::PlayDeathAnimation() {
    if (is_dead_ || !target_sprite_) return;

    is_dead_ = true;
    is_playing_attack_ = false;
    is_hurt_flashing_ = false;

    int dead_idx = (layout_type_ == LayoutType::kUnit)
        ? static_cast<int>(UnitFrameIndex::kDead)
        : static_cast<int>(BuildingFrameIndex::kDead);

    auto frame = GetFrameAt(dead_idx);
    if (frame) {
        target_sprite_->setSpriteFrame(frame);
    }

    target_sprite_->setColor(cocos2d::Color3B::GRAY);
}

void EntityAnimationController::SetUnitAnimationState(Core::UnitAnimationState state) {
    if (layout_type_ != LayoutType::kUnit) return;
    if (unit_state_ == state || is_dead_) return;

    unit_state_ = state;
    animation_timer_ = 0.0f;
    is_frame_b_ = false;

    switch (state) {
    case Core::UnitAnimationState::kIdle:
        animation_interval_ = 0.4f;
        break;
    case Core::UnitAnimationState::kMove:
        animation_interval_ = 0.15f;
        break;
    case Core::UnitAnimationState::kAttack:
        animation_interval_ = 0.2f;
        break;
    case Core::UnitAnimationState::kDead:
        PlayDeathAnimation();
        return;
    default:
        animation_interval_ = 0.2f;
        break;
    }

    if (!is_playing_attack_ && !is_hurt_flashing_) {
        UpdateCurrentFrame();
    }
}

void EntityAnimationController::SetFacing(Core::Facing facing) {
    if (layout_type_ != LayoutType::kUnit) return;
    if (current_facing_ == facing || is_dead_) return;

    current_facing_ = facing;

    if (!target_sprite_) return;

    switch (facing) {
    case Core::Facing::kLeft:
        target_sprite_->setFlippedX(true);
        break;
    case Core::Facing::kRight:
    case Core::Facing::kUp:
    case Core::Facing::kDown:
        target_sprite_->setFlippedX(false);
        break;
    }

    if (!is_playing_attack_ && !is_hurt_flashing_) {
        UpdateCurrentFrame();
    }
}

int EntityAnimationController::GetUnitDirectionalFrameIndex(bool is_frame_b) {
    switch (current_facing_) {
    case Core::Facing::kUp:
        return is_frame_b ? static_cast<int>(UnitFrameIndex::kUpB)
            : static_cast<int>(UnitFrameIndex::kUpA);
    case Core::Facing::kDown:
        return is_frame_b ? static_cast<int>(UnitFrameIndex::kDownB)
            : static_cast<int>(UnitFrameIndex::kDownA);
    case Core::Facing::kLeft:
    case Core::Facing::kRight:
    default:
        return is_frame_b ? static_cast<int>(UnitFrameIndex::kRightB)
            : static_cast<int>(UnitFrameIndex::kRightA);
    }
}

void EntityAnimationController::SetBuildingAnimationState(Core::BuildingAnimationState state) {
    if (layout_type_ != LayoutType::kBuilding) return;
    if (building_state_ == state || is_dead_) return;

    building_state_ = state;

    if (state == Core::BuildingAnimationState::kDestroyed) {
        PlayDeathAnimation();
        return;
    }

    if (!is_playing_attack_ && !is_hurt_flashing_) {
        UpdateCurrentFrame();
    }
}

void EntityAnimationController::UpdateCurrentFrame() {
    if (!target_sprite_ || !sprite_sheet_ || is_dead_) return;

    int frame_index = 0;

    if (layout_type_ == LayoutType::kUnit) {
        frame_index = GetUnitDirectionalFrameIndex(is_frame_b_);
    }
    else {
        frame_index = static_cast<int>(BuildingFrameIndex::kIdle);
    }

    auto frame = GetFrameAt(frame_index);
    if (frame) {
        target_sprite_->setSpriteFrame(frame);
        target_sprite_->setColor(cocos2d::Color3B::WHITE);
    }
}