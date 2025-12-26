// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of Unit.
//
// Path: Classes/Gameplay/Entities/Unit.cpp

#include "Contract/GamePlay/Unit.h"
#include "Contract/GamePlay/HealthComp.h"
#include "Gameplay/Components/AttackComp.h"
#include "Gameplay/Components/PathAgent.h"
#include "Gameplay/Components/EntityAnimationController.h"

Unit* Unit::create(Core::TroopType type, int level, int owner_id) {
    Unit* pRet = new(std::nothrow) Unit();
    if (pRet && pRet->init(type, level, owner_id)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Unit::init(Core::TroopType type, int level, int owner_id) {
    if (!BaseEntity::init()) return false;

    this->setLocalZOrder(static_cast<int>(Core::ZOrder::kUnits));
    this->type_ = type;
    this->level_ = level;
    this->stats_ = Core::GameConfig::GetInstance()->GetTroopStats(type, level);
    this->set_owner_id(owner_id);

    // 根据兵种类型设置碰撞半径
    switch (type) {
    case Core::TroopType::kGiant:
        collision_radius_ = 30.0f;
        break;
    case Core::TroopType::kBabyDragon:
        collision_radius_ = 25.0f;
        break;
    case Core::TroopType::kWallBreaker:
        collision_radius_ = 15.0f;
        break;
    default:
        collision_radius_ = 20.0f;
        break;
    }

    // 精灵和动画初始化
    std::string sprite_sheet = GetSpriteSheetFilename(type);
    int frame_w, frame_h;
    GetFrameSize(type, frame_w, frame_h);

    visual_sprite_ = cocos2d::Sprite::create();
    if (visual_sprite_) {
        visual_sprite_->setPosition(cocos2d::Vec2::ZERO);
        this->addChild(visual_sprite_);

        animation_controller_ = EntityAnimationController::create();
        if (animation_controller_) {
            animation_controller_->InitWithSprite(
                visual_sprite_,
                sprite_sheet,
                EntityAnimationController::LayoutType::kUnit,
                frame_w,
                frame_h
            );
            animation_controller_->setName("AnimationController");
            this->addChild(animation_controller_);
        }
    }
    else {
        auto debugDraw = cocos2d::DrawNode::create();
        debugDraw->drawSolidCircle(cocos2d::Vec2::ZERO, 10, 0, 10, cocos2d::Color4F::BLUE);
        this->addChild(debugDraw);
    }

    // Debug 碰撞体积
    auto debug_collider = cocos2d::DrawNode::create();
    debug_collider->drawCircle(cocos2d::Vec2::ZERO, collision_radius_, 0, 30, false, cocos2d::Color4F::GREEN);
    this->addChild(debug_collider, 100);

    // HealthComp
    auto health_comp = HealthComp::create();
    health_comp->InitStats(stats_.max_hp_);
    health_comp->setName("HealthComp");
    float height_offset = 40.0f;
    if (type == Core::TroopType::kGiant) height_offset = 60.0f;
    if (type == Core::TroopType::kBabyDragon) height_offset = 70.0f;
    health_comp->SetHealthBarOffset(cocos2d::Vec2(0, height_offset));
    this->addChild(health_comp);

    // AttackComp
    auto attack_comp = AttackComp::create();
    attack_comp->InitStats(stats_.damage_per_shot_, this->GetRangeInPixels(),
        stats_.attack_speed_, stats_.projectile_);
    attack_comp->setName("AttackComp");
    this->addChild(attack_comp);

    // PathAgent
    auto path_agent = PathAgent::create();
    path_agent->InitStats(stats_.move_speed_, this->GetRangeInPixels(), stats_.favorite_target_);
    path_agent->setName("PathAgent");
    this->addChild(path_agent);

    current_state_ = Core::UnitAnimationState::kIdle;
    current_facing_ = Core::Facing::kRight;

    return true;
}

bool Unit::CanAttack(Core::GeneralType target_type) const {
    Core::GeneralType cap = Core::GeneralType::kGround;
    switch (type_) {
    case Core::TroopType::kArcher:
    case Core::TroopType::kBabyDragon:
        cap = Core::GeneralType::kGround | Core::GeneralType::kAir;
        break;
    default:
        cap = Core::GeneralType::kGround;
        break;
    }
    return (static_cast<unsigned int>(cap) & static_cast<unsigned int>(target_type)) != 0;
}

cocos2d::Rect Unit::GetOccupiedRect() const {
    cocos2d::Vec2 pos = this->getPosition();
    float size = collision_radius_ * 2.0f;

    return cocos2d::Rect(
        pos.x - collision_radius_,
        pos.y - collision_radius_,
        size,
        size
    );
}

void Unit::SetState(Core::UnitAnimationState new_state) {
    if (current_state_ == new_state || current_state_ == Core::UnitAnimationState::kDead) return;
    current_state_ = new_state;

    if (current_state_ == Core::UnitAnimationState::kDead) {
        auto agent = dynamic_cast<PathAgent*>(this->getChildByName("PathAgent"));
        if (agent) agent->Stop();
    }

    if (animation_controller_) {
        animation_controller_->SetUnitAnimationState(new_state);
    }
}

void Unit::SetFacing(Core::Facing facing) {
    if (current_facing_ == facing) return;
    current_facing_ = facing;

    if (animation_controller_) {
        animation_controller_->SetFacing(facing);
    }
}

void Unit::PlayAttackAnimation() {
    if (animation_controller_) {
        float dur = stats_.attack_speed_ * 0.3f;
        dur = std::max(0.1f, std::min(dur, 0.5f));
        animation_controller_->PlayAttackAnimation(dur);
    }
}

void Unit::PlayHurtEffect() {
    if (animation_controller_) {
        animation_controller_->PlayHurtFlash();
    }
}

void Unit::PlayDeathAnimation() {
    if (animation_controller_) {
        animation_controller_->PlayDeathAnimation();
    }
}

void Unit::update(float dt) {
    auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));
    if (hp_comp && hp_comp->IsDead() && current_state_ != Core::UnitAnimationState::kDead) {
        SetState(Core::UnitAnimationState::kDead);
        this->is_marked_for_destruction_ = false;
    }

    BaseEntity::update(dt);
    if (IsMarkedForDestruction()) return;

    if (current_state_ == Core::UnitAnimationState::kDead) {
        if (visual_sprite_ && visual_sprite_->getActionByTag(999) == nullptr) {
            auto fade = cocos2d::FadeOut::create(1.0f);
            auto remove = cocos2d::CallFunc::create([this]() {
                this->MarkForDestruction();
                });
            auto seq = cocos2d::Sequence::create(fade, remove, nullptr);
            seq->setTag(999);
            visual_sprite_->runAction(seq);
        }
    }
}

std::string Unit::GetSpriteSheetFilename(Core::TroopType type) {
    switch (type) {
    case Core::TroopType::kBarbarian:   return "Troops_Icon/Barbarian-status.png";
    case Core::TroopType::kArcher:      return "Troops_Icon/Archer-status.png";
    case Core::TroopType::kGiant:       return "Troops_Icon/Giant-status.png";
    case Core::TroopType::kWallBreaker: return "Troops_Icon/WallBreaker-status.png";
    case Core::TroopType::kBabyDragon:  return "Troops_Icon/BabyDragon-status.png";
    default:                            return "Troops_Icon/Default-status.png";
    }
}

void Unit::GetFrameSize(Core::TroopType type, int& out_width, int& out_height) {
    // 500x500 精灵图，4x4 布局，每帧 125x125
    out_width = 125;
    out_height = 125;
}