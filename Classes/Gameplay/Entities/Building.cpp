// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of Building.
// [UPDATE] Added GameEvents broadcasting for spawn/destroy/state changes
//
// Path: Classes/Gameplay/Entities/Building.cpp

#include "Contract/GamePlay/Building.h"
#include "Contract/GamePlay/Unit.h" 
#include "Contract/GamePlay/HealthComp.h" 
#include "Contract/GamePlay/EconomySystem.h"
#include "Contract/GamePlay/CombatResolver.h"
#include "Contract/GamePlay/GameEvents.h"
#include "Gameplay/Components/AttackComp.h"
#include "Gameplay/Components/PathAgent.h"
#include "Gameplay/Components/EntityAnimationController.h"

// 静态 ID 生成器
static int s_next_building_id = 1000;

Building* Building::create(Core::BuildingType type, int level, int owner_id) {
    Building* pRet = new(std::nothrow) Building();
    if (pRet && pRet->init(type, level, owner_id)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Building::init(Core::BuildingType type, int level, int owner_id) {
    if (!BaseEntity::init()) return false;

    // 分配唯一 ID
    this->set_instance_id(s_next_building_id++);

    this->setLocalZOrder(static_cast<int>(Core::ZOrder::kBuildingBase));
    this->type_ = type;
    this->level_ = level;
    this->stats_ = Core::GameConfig::GetInstance()->GetBuildingStats(type, level);
    this->set_owner_id(owner_id);

    this->current_state_ = Core::BuildingAnimationState::kIdle;
    this->construction_timer_ = 0.0f;
    this->construction_duration_ = 0.0f;
    this->obstacle_registered_ = false;

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
                EntityAnimationController::LayoutType::kBuilding,
                frame_w,
                frame_h
            );
            animation_controller_->setName("AnimationController");
            this->addChild(animation_controller_);
        }
    }
    else {
        cocos2d::log("Error: Failed to create building sprite for type: %d", static_cast<int>(type));
        auto debugRect = cocos2d::DrawNode::create();
        debugRect->drawSolidRect(cocos2d::Vec2(-30, -30), cocos2d::Vec2(30, 30), cocos2d::Color4F::RED);
        this->addChild(debugRect);
    }

    // Debug 占地范围
    auto debug_grid = cocos2d::DrawNode::create();
    float total_w = stats_.width_ * Core::kTileWidth;
    float total_h = stats_.height_ * Core::kTileHeight;
    cocos2d::Vec2 min_pos(-total_w / 2.0f, -total_h / 2.0f);
    cocos2d::Vec2 max_pos(total_w / 2.0f, total_h / 2.0f);
    debug_grid->drawRect(min_pos, max_pos, cocos2d::Color4F::RED);
    debug_grid->drawSolidRect(min_pos, max_pos, cocos2d::Color4F(1.0f, 0.0f, 0.0f, 0.2f));
    this->addChild(debug_grid, 100);

    // HealthComp
    auto health_comp = HealthComp::create();
    health_comp->InitStats(stats_.max_hp_);
    health_comp->setName("HealthComp");
    health_comp->SetHealthBarOffset(cocos2d::Vec2(0, 50));
    this->addChild(health_comp);

    // AttackComp (仅攻击建筑)
    if (stats_.damage_ > 0) {
        auto attack_comp = AttackComp::create();
        Core::ProjectileType projType = GetProjectileTypeFromBuilding(type);
        attack_comp->InitStats(stats_.damage_, GetRangeInPixels(), stats_.attack_speed_, projType);
        attack_comp->setName("AttackComp");
        this->addChild(attack_comp);
    }

    stored_resource_ = 0.0f;
    return true;
}

void Building::onEnter() {
    BaseEntity::onEnter();

    // 注册障碍物
    if (type_ != Core::BuildingType::kNone && !obstacle_registered_) {
        cocos2d::Rect rect = this->GetOccupiedRect();
        PathAgent::UpdateObstacle(rect, true);
        obstacle_registered_ = true;
    }

    // [EVENT] 广播实体生成事件
    cocos2d::Vec2 pos = this->getPosition();

    Gameplay::EntitySpawnEvent evt;
    evt.instance_id = this->get_instance_id();
    evt.owner_id = this->get_owner_id();
    evt.x = pos.x;
    evt.y = pos.y;
    evt.level = level_;
    evt.current_hp = stats_.max_hp_;
    evt.max_hp = stats_.max_hp_;
    evt.is_building = true;
    evt.troop_type = Core::TroopType::kBarbarian;  // 无效值
    evt.building_type = type_;

    Gameplay::GameEventManager::GetInstance()->BroadcastEntitySpawned(evt);

    cocos2d::log("Building Spawned: ID=%d, Type=%d, Owner=%d, Pos=(%.0f, %.0f)",
        evt.instance_id, static_cast<int>(type_), evt.owner_id, pos.x, pos.y);
}

void Building::onExit() {
    if (type_ != Core::BuildingType::kNone && obstacle_registered_) {
        cocos2d::Rect rect = this->GetOccupiedRect();
        PathAgent::UpdateObstacle(rect, false);
        obstacle_registered_ = false;
    }
    BaseEntity::onExit();
}

void Building::SetState(Core::BuildingAnimationState new_state) {
    if (current_state_ == new_state) return;

    Core::BuildingAnimationState old_state = current_state_;
    current_state_ = new_state;

    if (animation_controller_) {
        animation_controller_->SetBuildingAnimationState(new_state);
    }

    // [EVENT] 广播建筑状态变化
    Gameplay::BuildingStateEvent state_evt;
    state_evt.instance_id = this->get_instance_id();
    state_evt.type = type_;
    state_evt.time_remaining = construction_timer_;
    state_evt.total_build_time = construction_duration_;

    switch (new_state) {
    case Core::BuildingAnimationState::kConstructing:
        state_evt.new_state = Gameplay::BuildingState::kConstructing;
        break;
    case Core::BuildingAnimationState::kIdle:
        state_evt.new_state = Gameplay::BuildingState::kIdle;
        break;
    case Core::BuildingAnimationState::kDestroyed:
        state_evt.new_state = Gameplay::BuildingState::kDestroyed;
        break;
    default:
        state_evt.new_state = Gameplay::BuildingState::kIdle;
        break;
    }

    Gameplay::GameEventManager::GetInstance()->BroadcastBuildingStateChanged(state_evt);

    // 销毁处理
    if (current_state_ == Core::BuildingAnimationState::kDestroyed) {
        // 移除障碍物
        if (obstacle_registered_) {
            cocos2d::Rect rect = this->GetOccupiedRect();
            PathAgent::UpdateObstacle(rect, false);
            obstacle_registered_ = false;
        }

        // [EVENT] 广播实体销毁事件
        Gameplay::EntityDestroyEvent destroy_evt;
        destroy_evt.instance_id = this->get_instance_id();
        destroy_evt.is_building = true;
        Gameplay::GameEventManager::GetInstance()->BroadcastEntityDestroyed(destroy_evt);

        cocos2d::log("Building Destroyed: ID=%d, Type=%d",
            this->get_instance_id(), static_cast<int>(type_));

        this->is_marked_for_destruction_ = false;

        if (visual_sprite_) {
            auto collapse = cocos2d::Spawn::create(
                cocos2d::ScaleTo::create(0.5f, 1.2f, 0.1f),
                cocos2d::FadeOut::create(0.5f),
                nullptr
            );
            auto seq = cocos2d::Sequence::create(
                collapse,
                cocos2d::CallFunc::create([this]() {
                    this->MarkForDestruction();
                    }),
                nullptr
            );
            visual_sprite_->runAction(seq);

            auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));
            if (hp_comp) hp_comp->setVisible(false);
        }
        else {
            this->MarkForDestruction();
        }
    }
}

void Building::StartConstruction(float duration) {
    if (duration <= 0) return;
    construction_timer_ = duration;
    construction_duration_ = duration;
    SetState(Core::BuildingAnimationState::kConstructing);
}

float Building::GetConstructionProgress() const {
    if (current_state_ != Core::BuildingAnimationState::kConstructing) {
        return 1.0f;
    }

    if (construction_duration_ <= 0.0f) {
        return 1.0f;
    }

    float elapsed = construction_duration_ - construction_timer_;
    return std::min(1.0f, std::max(0.0f, elapsed / construction_duration_));
}

float Building::GetStoragePercentage() const {
    if (stats_.resource_capacity_ <= 0) {
        return 0.0f;
    }
    return std::min(1.0f, stored_resource_ / static_cast<float>(stats_.resource_capacity_));
}

void Building::PlayAttackAnimation() {
    if (animation_controller_) {
        float dur = stats_.attack_speed_ * 0.3f;
        dur = std::max(0.1f, std::min(dur, 0.5f));
        animation_controller_->PlayAttackAnimation(dur);
    }
}

void Building::PlayHurtEffect() {
    if (animation_controller_) {
        animation_controller_->PlayHurtFlash();
    }
}

void Building::PlayDestroyedAnimation() {
    if (animation_controller_) {
        animation_controller_->PlayDeathAnimation();
    }
}

void Building::update(float dt) {
    auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));

    if (hp_comp && hp_comp->IsDead() && current_state_ != Core::BuildingAnimationState::kDestroyed) {
        SetState(Core::BuildingAnimationState::kDestroyed);
        return;
    }

    if (current_state_ == Core::BuildingAnimationState::kDestroyed) {
        BaseEntity::update(dt);
        return;
    }

    BaseEntity::update(dt);
    if (IsMarkedForDestruction()) return;

    // 建造逻辑
    if (current_state_ == Core::BuildingAnimationState::kConstructing) {
        construction_timer_ -= dt;
        if (construction_timer_ <= 0.0f) {
            construction_timer_ = 0.0f;
            SetState(Core::BuildingAnimationState::kIdle);
            if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);
            cocos2d::Vector<Building*> dummy;
            EconomySystem::GetInstance()->RecalculateLimits(dummy);
        }
        else {
            if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::GRAY);
            return;
        }
    }

    // 资源生产
    if (stats_.production_rate_ > 0) {
        ProduceResource(dt);
    }

    // 防御索敌
    if (stats_.damage_ > 0) {
        UpdateCombatLogic(dt);
    }
}

void Building::ProduceResource(float dt) {
    if (stored_resource_ >= stats_.resource_capacity_) return;
    float amount_per_sec = stats_.production_rate_ / 3600.0f;
    stored_resource_ += amount_per_sec * dt;
    if (stored_resource_ > stats_.resource_capacity_) {
        stored_resource_ = static_cast<float>(stats_.resource_capacity_);
    }
}

int Building::CollectResource(int max_amount) {
    int available = static_cast<int>(stored_resource_);
    int to_collect = available;
    if (max_amount >= 0 && available > max_amount) {
        to_collect = max_amount;
    }
    stored_resource_ -= static_cast<float>(to_collect);
    if (stored_resource_ < 0.0f) stored_resource_ = 0.0f;
    return to_collect;
}

void Building::UpdateCombatLogic(float dt) {
    auto attack_comp = dynamic_cast<AttackComp*>(this->getChildByName("AttackComp"));
    if (!attack_comp) return;
    if (attack_comp->GetCooldownTimer() > 0.0f) return;

    auto& all_entities = BaseEntity::GetAllEntities();

    BaseEntity* best_target = nullptr;
    float min_dist_sq = FLT_MAX;
    float range_sq = GetRangeInPixels() * GetRangeInPixels();

    for (auto node : all_entities) {
        if (node == this) continue;

        auto target = dynamic_cast<BaseEntity*>(node);
        if (!target) continue;
        if (target->IsAlly(this)) continue;
        if (target->IsMarkedForDestruction()) continue;

        auto hp = dynamic_cast<HealthComp*>(target->getChildByName("HealthComp"));
        if (hp && hp->IsDead()) continue;

        auto target_unit = dynamic_cast<Unit*>(target);
        if (target_unit && !this->CanAttack(target_unit->GetGeneralType())) continue;

        float dist_sq = this->getPosition().getDistanceSq(target->getPosition());
        if (dist_sq <= range_sq && dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            best_target = target;
        }
    }

    if (best_target) {
        attack_comp->TryAttack(best_target);
    }
}

std::string Building::GetSpriteSheetFilename(Core::BuildingType type) {
    switch (type) {
    case Core::BuildingType::kTownHall:       return "Buildings_Icon/TownHall-status.png";
    case Core::BuildingType::kGoldMine:       return "Buildings_Icon/GoldMine-status.png";
    case Core::BuildingType::kElixirCollector:return "Buildings_Icon/ElixirCollector-status.png";
    case Core::BuildingType::kCannon:         return "Buildings_Icon/Cannon-status.png";
    case Core::BuildingType::kArcherTower:    return "Buildings_Icon/ArcherTower-status.png";
    case Core::BuildingType::kWall:           return "Buildings_Icon/Wall-status.png";
    case Core::BuildingType::kArmyCamp:       return "Buildings_Icon/ArmyCamp-status.png";
    case Core::BuildingType::kBarracks:       return "Buildings_Icon/Barracks-status.png";
    case Core::BuildingType::kGoldStorage:    return "Buildings_Icon/GoldStorage-status.png";
    case Core::BuildingType::kElixirStorage:  return "Buildings_Icon/ElixirStorage-status.png";
    case Core::BuildingType::kAirDefense:     return "Buildings_Icon/AirDefense-status.png";
    default:                                  return "Buildings_Icon/Default-status.png";
    }
}

void Building::GetFrameSize(Core::BuildingType type, int& out_width, int& out_height) {
    // 500x500 精灵图，2x2 布局，每帧 250x250
    out_width = 250;
    out_height = 250;
}

Core::ProjectileType Building::GetProjectileTypeFromBuilding(Core::BuildingType type) {
    switch (type) {
    case Core::BuildingType::kCannon:      return Core::ProjectileType::kCannonBall;
    case Core::BuildingType::kArcherTower: return Core::ProjectileType::kArrow;
    case Core::BuildingType::kAirDefense:  return Core::ProjectileType::kRocket;
    default:                               return Core::ProjectileType::kNone;
    }
}

cocos2d::Rect Building::GetOccupiedRect() const {
    cocos2d::Vec2 pos = this->getPosition();
    float total_width = stats_.width_ * Core::kTileWidth;
    float total_height = stats_.height_ * Core::kTileHeight;
    return cocos2d::Rect(
        pos.x - total_width / 2,
        pos.y - total_height / 2,
        total_width,
        total_height
    );
}