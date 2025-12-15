// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of Unit logic.

#include "Unit.h"
#include "Gameplay/Components/HealthComp.h"
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
    //设置阵营
    this->set_owner_id(owner_id);
    if (owner_id == 0) {
        this->set_camp(Core::CampType::kPlayer);
    }
    else {
        this->set_camp(Core::CampType::kEnemy);
    }
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
    // 组件挂载 (Component Composition)
    // -------------------------------------------------------------------------

    // (A) HealthComp: 负责血条和死亡
    auto health_comp = HealthComp::create();
    health_comp->InitStats(stats_.max_hp_);
    health_comp->setName("HealthComp"); // 设置名字，方便 getChildByName 查找

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
    // 传入: 移动速度, 攻击射程, 偏好目标(如巨人打防御塔)
    path_agent->InitStats(stats_.move_speed_, this->GetRangeInPixels(), stats_.favorite_target_);
    path_agent->setName("PathAgent");
    this->addChild(path_agent);

    //设为默认状态
    current_state_ = State::kIdle;
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
void Unit::SetState(State new_state) {
    // 如果状态没变，或者已经死了，就不要再切状态了
    if (current_state_ == new_state || current_state_ == State::kDead) return;

    current_state_ = new_state;

    // [关键修改] 如果进入死亡状态，立刻让 PathAgent 停止工作并释放目标
    // 这打破了 "A引用B，B引用A" 的死锁循环
    if (current_state_ == State::kDead) {
        auto agent = dynamic_cast<PathAgent*>(this->getChildByName("PathAgent"));
        if (agent) {
            agent->Stop();
        }
    }
    // 简单的视觉反馈 (Visual Feedback)
    // 实际应该替换为 AnimationController->Play("Attack")（暂时简化）
    if (visual_sprite_) {
        switch (current_state_) {
        case State::kIdle:
            // 恢复原色
            visual_sprite_->setColor(cocos2d::Color3B::WHITE);
            break;
        case State::kMove:
            // 移动时也可以保持原色，或者播放脚步粒子
            visual_sprite_->setColor(cocos2d::Color3B::WHITE);
            break;
        case State::kAttack:
            // 攻击时闪烁一下红色 (模拟发力)
            // 注意：这里最好只设一次，Update里不要重复设
            //visual_sprite_->setColor(cocos2d::Color3B::RED); 
            break;
        case State::kDead:
            // update中已经处理了淡出，这里主要做逻辑标记
            break;
        }
    }
}
//帧逻辑
void Unit::update(float dt) {
    // 先检查是否应该死亡
    auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));
    if (hp_comp && hp_comp->IsDead() && current_state_ != State::kDead) {

        // 切换状态 (这里面会调用 PathAgent->Stop() 释放引用)
        SetState(State::kDead);

        // 确保不会被立即清理
        this->is_marked_for_destruction_ = false;
    }

    BaseEntity::update(dt);
    if (IsMarkedForDestruction()) return;

    switch (current_state_) {
    case State::kIdle:
        // 待机：正常颜色
        if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);
        break;

    case State::kMove:
        // 移动：正常颜色 (未来这里调用奔跑动画)
        if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);
        break;

    case State::kAttack:
        // 攻击时会变得微红
        if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B(255, 220, 220));
        break;

    case State::kDead:
        // 死亡：变灰 -> 慢慢透明 -> 销毁
        if (visual_sprite_) {
            // 使用 Tag (例如 999) 检查死亡动画是否已经在播放中
            // 防止 update 每一帧都重复创建动画
            if (visual_sprite_->getActionByTag(999) == nullptr) {

                visual_sprite_->setColor(cocos2d::Color3B::GRAY);

                // 淡出动画 (1.0秒内透明度变为0)
                auto fade_out = cocos2d::FadeOut::create(1.0f);

                // 回调函数：动画播放完毕后，才真正标记销毁
                auto remove_callback = cocos2d::CallFunc::create([this]() {
                    this->MarkForDestruction();
                    });

                // 创建序列：先淡出，再销毁
                auto sequence = cocos2d::Sequence::create(fade_out, remove_callback, nullptr);
                sequence->setTag(999); // 标记此动作

                //  执行
                visual_sprite_->runAction(sequence);
            }
        }
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
        // 默认兜底：防止传入了未知的枚举值导致程序崩溃
    default:                            return "Barbarian.png";
    }
}
