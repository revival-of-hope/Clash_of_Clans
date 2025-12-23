// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of Unit logic.

#include "Contract/Gameplay/Unit.h"
#include "Contract/Gameplay/HealthComp.h"
#include "Gameplay/Components/AttackComp.h"
#include "Gameplay/Components/PathAgent.h"

//用Factory Method创造
Unit* Unit::create(Core::TroopType type, int level, int owner_id) {
    // std::nothrow 的作用：如果内存耗尽，不抛出崩溃异常，而是返回 nullptr
    Unit* pRet = new(std::nothrow) Unit();

    if (pRet && pRet->init(type, level, owner_id)) {
        // 如果我不立刻addChild ，下一帧它就会自动从内存消失。
        pRet->autorelease();
        return pRet;
    }
    else {
        //初始化失败(如图片没找到)，手动清理内存，防止泄漏
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

//初始化
bool Unit::init(Core::TroopType type, int level, int owner_id) {
    //初始化父类
    if (!BaseEntity::init()) {
        return false;
    }

    this->setLocalZOrder(static_cast<int>(Core::ZOrder::kUnits));
    //保存类型信息
    this->type_ = type;
    //去GameConfig中调用
    this->stats_ = Core::GameConfig::GetInstance()->GetTroopStats(type, level);

    // [修复] 不再依赖 Core::CampType，直接存储 owner_id
    this->set_owner_id(owner_id);

    //根据兵种获取对应的图片
    std::string filename = GetSpriteFilename(type);
    //创建精灵
    visual_sprite_ = cocos2d::Sprite::create(filename);
    //检查图片是否成功加载
    if (visual_sprite_) {
        // 图片的中心点与 Unit 逻辑节点的中心点重合
        visual_sprite_->setPosition(cocos2d::Vec2::ZERO);

        // Unit 移动时，图片会跟着一起移动；Unit 销毁时，图片也会一起销毁。
        this->addChild(visual_sprite_);
    }
    else {
        //如果没找到图片则报错
        cocos2d::log("Error: Failed to load unit sprite: %s", filename.c_str());
        // 屏幕上会出现一个蓝点（可视化debug）
        auto debugDraw = cocos2d::DrawNode::create();
        debugDraw->drawSolidCircle(cocos2d::Vec2::ZERO, 10, 0, 10, cocos2d::Color4F::BLUE);
        this->addChild(debugDraw);
    }

    // -------------------------------------------------------------------------
    // [DEBUG] 可视化碰撞体积 (Collision Box Visualization)
    // -------------------------------------------------------------------------
    auto debug_collider = cocos2d::DrawNode::create();
    debug_collider->drawCircle(cocos2d::Vec2::ZERO, 20.0f, 0, 30, false, cocos2d::Color4F::GREEN);
    debug_collider->drawDot(cocos2d::Vec2::ZERO, 2.0f, cocos2d::Color4F::GREEN);
    this->addChild(debug_collider, 100);


    // -------------------------------------------------------------------------
    // 组件挂载 (Component Composition)
    // -------------------------------------------------------------------------

    // (A) HealthComp: 负责血条和死亡
    auto health_comp = HealthComp::create();
    health_comp->InitStats(stats_.max_hp_);
    health_comp->setName("HealthComp");

    // 根据体型调整血条高度
    float height_offset = 40.0f;
    if (type == Core::TroopType::kGiant) height_offset = 60.0f;
    if (type == Core::TroopType::kBabyDragon) height_offset = 70.0f;
    health_comp->SetHealthBarOffset(cocos2d::Vec2(0, height_offset));
    this->addChild(health_comp);

    // (B) AttackComp: 负责攻击CD和造成伤害
    auto attack_comp = AttackComp::create();
    attack_comp->InitStats(stats_.damage_per_shot_, this->GetRangeInPixels(), stats_.attack_speed_, stats_.projectile_);
    attack_comp->setName("AttackComp");
    this->addChild(attack_comp);

    // (C) PathAgent: 负责寻路 AI
    auto path_agent = PathAgent::create();
    path_agent->InitStats(stats_.move_speed_, this->GetRangeInPixels(), stats_.favorite_target_);
    path_agent->setName("PathAgent");
    this->addChild(path_agent);

    //设为默认状态
    current_state_ = Core::UnitAnimationState::kIdle;
    return true;
}

// 攻击能力判定
bool Unit::CanAttack(Core::GeneralType target_type) const {
    Core::GeneralType my_capability = Core::GeneralType::kGround;
    switch (type_) {
    case Core::TroopType::kArcher:
    case Core::TroopType::kBabyDragon:
        my_capability = Core::GeneralType::kGround | Core::GeneralType::kAir;
        break;
    case Core::TroopType::kBarbarian:
    case Core::TroopType::kGiant:
    case Core::TroopType::kWallBreaker:
        my_capability = Core::GeneralType::kGround;
        break;
    }
    // 使用位运算检查是否有交集
    return (static_cast<unsigned int>(my_capability) &
        static_cast<unsigned int>(target_type)) != 0;
}

// SetState 实现
void Unit::SetState(Core::UnitAnimationState new_state) {
    // 如果状态没变，或者已经死了，就不要再切状态了
    if (current_state_ == new_state || current_state_ == Core::UnitAnimationState::kDead) return;

    current_state_ = new_state;

    // [关键修改] 如果进入死亡状态，立刻让 PathAgent 停止工作并释放目标
    if (current_state_ == Core::UnitAnimationState::kDead) {
        auto agent = dynamic_cast<PathAgent*>(this->getChildByName("PathAgent"));
        if (agent) {
            agent->Stop();
        }
    }
    // 简单的视觉反馈 (Visual Feedback)
    if (visual_sprite_) {
        switch (current_state_) {
        case Core::UnitAnimationState::kIdle:
            visual_sprite_->setColor(cocos2d::Color3B::WHITE);
            break;
        case Core::UnitAnimationState::kMove:
            visual_sprite_->setColor(cocos2d::Color3B::WHITE);
            break;
        case Core::UnitAnimationState::kAttack:
            // 攻击时闪烁一下红色 (模拟发力) - 逻辑交由 AttackComp 触发反馈，这里保持状态一致性
            break;
        case Core::UnitAnimationState::kDead:
            // update中已经处理了淡出
            break;
        default:
            break;
        }
    }
}

void Unit::SetFacing(Core::Facing facing) {
    if (current_facing_ == facing) return;
    current_facing_ = facing;

    // 简单的 2D 翻转实现
    if (facing == Core::Facing::kLeft) {
        this->setScaleX(-1.0f);
    }
    else if (facing == Core::Facing::kRight) {
        this->setScaleX(1.0f);
    }
    // Up/Down 暂不处理，或者留给 AnimationController
}

//帧逻辑
void Unit::update(float dt) {
    // 先检查是否应该死亡
    auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));
    if (hp_comp && hp_comp->IsDead() && current_state_ != Core::UnitAnimationState::kDead) {

        // 切换状态 (这里面会调用 PathAgent->Stop() 释放引用)
        SetState(Core::UnitAnimationState::kDead);

        // 确保不会被立即清理
        this->is_marked_for_destruction_ = false;
    }

    BaseEntity::update(dt);
    if (IsMarkedForDestruction()) return;

    switch (current_state_) {
    case Core::UnitAnimationState::kIdle:
        if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);
        break;

    case Core::UnitAnimationState::kMove:
        if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);
        break;

    case Core::UnitAnimationState::kAttack:
        if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B(255, 220, 220));
        break;

    case Core::UnitAnimationState::kDead:
        // 死亡：变灰 -> 慢慢透明 -> 销毁
        if (visual_sprite_) {
            if (visual_sprite_->getActionByTag(999) == nullptr) {

                visual_sprite_->setColor(cocos2d::Color3B::GRAY);
                auto fade_out = cocos2d::FadeOut::create(1.0f);

                auto remove_callback = cocos2d::CallFunc::create([this]() {
                    this->MarkForDestruction();
                    });

                auto sequence = cocos2d::Sequence::create(fade_out, remove_callback, nullptr);
                sequence->setTag(999);
                visual_sprite_->runAction(sequence);
            }
        }
        break;
    default:
        break;
    }
}
//辅助函数
std::string Unit::GetSpriteFilename(Core::TroopType type) {
    switch (type) {
    case Core::TroopType::kBarbarian:   return "Troops_Icon/Barbarian.png";
    case Core::TroopType::kArcher:      return "Troops_Icon/Archer.png";
    case Core::TroopType::kGiant:       return "Troops_Icon/Giant.png";
    case Core::TroopType::kWallBreaker: return "Troops_Icon/WallBreaker.png";
    case Core::TroopType::kBabyDragon:  return "Troops_Icon/BabyDragon.png";
    default:                            return "Barbarian.png";
    }
}