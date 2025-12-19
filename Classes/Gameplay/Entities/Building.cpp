// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of Building logic.
// [FIXED] Moved UpdateObstacle to onEnter/onExit to fix the (0,0) coordinate bug.
// [REFACTOR] Adopted Core::BuildingAnimationState for state management.

#include "GamePlay/Public/Building.h"
#include "GamePlay/Public/Unit.h" 
#include "GamePlay/Public/HealthComp.h" 
#include "GamePlay/Public/EconomySystem.h" // 引入经济系统
#include "GamePlay/Public/CombatResolver.h"
#include "Gameplay/Components/AttackComp.h"
#include "Gameplay/Components/PathAgent.h"

//静态创建函数
Building* Building::create(Core::BuildingType type, int level, int owner_id) {
    Building* pRet = new(std::nothrow) Building();
    // 调用 init 初始化
    if (pRet && pRet->init(type, level, owner_id)) {
        pRet->autorelease();
        return pRet;
    }
    else {
        // 初始化失败，手动删除防止内存泄漏
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}
//初始化函数
bool Building::init(Core::BuildingType type, int level, int owner_id) {
    //初始化父类 BaseEntity
    if (!BaseEntity::init()) {
        return false;
    }

    this->setLocalZOrder(static_cast<int>(Core::ZOrder::kBuildingBase));
    //保存类型和等级
    this->type_ = type;
    this->level_ = level;
    //从GameConfig读取数值
    // 血量、攻击力、攻击范围、产出效率
    this->stats_ = Core::GameConfig::GetInstance()->GetBuildingStats(type, level);

    //设置阵营
    this->set_owner_id(owner_id);
    if (owner_id == 0) {
        this->set_camp(Core::CampType::kPlayer);
    }
    else {
        this->set_camp(Core::CampType::kEnemy);
    }

    // 默认状态
    this->current_state_ = Core::BuildingAnimationState::kIdle;
    this->construction_timer_ = 0.0f;
    this->obstacle_registered_ = false; // 初始化标记

    //加载外观
    std::string filename = GetSpriteFilename(type);
    visual_sprite_ = cocos2d::Sprite::create(filename);

    if (visual_sprite_) {
        // 放在节点中心
        visual_sprite_->setPosition(cocos2d::Vec2::ZERO);
        this->addChild(visual_sprite_);
    }
    else {
        cocos2d::log("Error: Failed to load building sprite: %s", filename.c_str());
        auto debugRect = cocos2d::DrawNode::create();
        debugRect->drawSolidRect(cocos2d::Vec2(-30, -30), cocos2d::Vec2(30, 30), cocos2d::Color4F::RED);
        this->addChild(debugRect);
    }

    // -------------------------------------------------------------------------
    // [DEBUG] 可视化占地范围 (Obstacle Box Visualization)
    // -------------------------------------------------------------------------
    auto debug_grid = cocos2d::DrawNode::create();
    float total_w = stats_.width_ * Core::kTileWidth;
    float total_h = stats_.height_ * Core::kTileHeight;
    cocos2d::Vec2 min_pos(-total_w / 2.0f, -total_h / 2.0f);
    cocos2d::Vec2 max_pos(total_w / 2.0f, total_h / 2.0f);

    debug_grid->drawRect(min_pos, max_pos, cocos2d::Color4F::RED);
    debug_grid->drawSolidRect(min_pos, max_pos, cocos2d::Color4F(1.0f, 0.0f, 0.0f, 0.2f));
    this->addChild(debug_grid, 100);


    //挂载HealthComp
    auto health_comp = HealthComp::create();
    health_comp->InitStats(stats_.max_hp_);
    health_comp->setName("HealthComp");
    health_comp->SetHealthBarOffset(cocos2d::Vec2(0, 50));
    this->addChild(health_comp);

    // 只有具有攻击力的建筑才挂载攻击组件
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

    if (type_ != Core::BuildingType::kNone && !obstacle_registered_) {
        cocos2d::Rect rect = this->GetOccupiedRect();
        PathAgent::UpdateObstacle(rect, true); // 注册：设为阻挡
        obstacle_registered_ = true;

        cocos2d::log("Building[%d] registered obstacle at GridPos: (%.1f, %.1f)",
            get_instance_id(), rect.getMidX(), rect.getMidY());
    }
}

void Building::onExit() {
    if (type_ != Core::BuildingType::kNone && obstacle_registered_) {
        cocos2d::Rect rect = this->GetOccupiedRect();
        PathAgent::UpdateObstacle(rect, false); // 注销：设为通行
        obstacle_registered_ = false;
    }
    BaseEntity::onExit();
}

void Building::SetState(Core::BuildingAnimationState new_state) {
    if (current_state_ == new_state) return;
    current_state_ = new_state;

    // 状态切换逻辑
    if (current_state_ == Core::BuildingAnimationState::kDestroyed) {
        // [新增] 建筑倒塌瞬间，立即移除碰撞体积
        if (obstacle_registered_) {
            cocos2d::Rect rect = this->GetOccupiedRect();
            PathAgent::UpdateObstacle(rect, false);
            obstacle_registered_ = false;
        }

        // 撤销 BaseEntity 的销毁标记 (等待动画播放完毕)
        this->is_marked_for_destruction_ = false;

        // 播放倒塌动画
        if (visual_sprite_) {
            visual_sprite_->setColor(cocos2d::Color3B::GRAY);
            auto collapse = cocos2d::Spawn::create(
                cocos2d::ScaleTo::create(0.5f, 1.2f, 0.1f),
                cocos2d::FadeOut::create(0.5f),
                nullptr
            );

            auto seq = cocos2d::Sequence::create(
                collapse,
                cocos2d::CallFunc::create([this]() {
                    this->MarkForDestruction(); // 动画结束，真正销毁
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

// 每一帧的逻辑 (Update)
void Building::update(float dt) {
    auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));

    // 检查死亡转换
    if (hp_comp && hp_comp->IsDead() && current_state_ != Core::BuildingAnimationState::kDestroyed) {
        SetState(Core::BuildingAnimationState::kDestroyed);
        return; // 倒塌中，跳过后续逻辑
    }

    // 倒塌中状态，不执行其他逻辑
    if (current_state_ == Core::BuildingAnimationState::kDestroyed) {
        BaseEntity::update(dt);
        return;
    }

    // 正常销毁检查
    BaseEntity::update(dt);
    if (IsMarkedForDestruction()) return;

    // 建造逻辑
    if (current_state_ == Core::BuildingAnimationState::kConstructing) {
        construction_timer_ -= dt;

        if (construction_timer_ <= 0.0f) {
            // 建造完成!
            SetState(Core::BuildingAnimationState::kIdle);
            construction_timer_ = 0.0f;

            cocos2d::log("Building[%d] construction finished!", this->get_instance_id());

            if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);

            // 通知 EconomySystem 重新计算上限 (Capacity/Population)
            auto parent_node = this->getParent();
            if (parent_node) {
                cocos2d::Vector<Building*> all_buildings;
                auto children = parent_node->getChildren();

                for (auto node : children) {
                    auto b = dynamic_cast<Building*>(node);
                    if (b) {
                        all_buildings.pushBack(b);
                    }
                }
                EconomySystem::GetInstance()->RecalculateLimits(all_buildings);
            }
        }
        else {
            // 还在建造中
            if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::GRAY);
            return;
        }
    }

    // 资源生产
    if (stats_.production_rate_ > 0) {
        ProduceResource(dt);
    }

    // 防御塔索敌
    if (stats_.damage_ > 0) {
        UpdateCombatLogic(dt);
    }
}


void Building::StartConstruction(float duration) {
    if (duration <= 0) return;
    SetState(Core::BuildingAnimationState::kConstructing);
    construction_timer_ = duration;
    cocos2d::log("Building[%d] started construction. Time: %.1f", this->get_instance_id(), duration);
}

// 经济系统实现 (Economy Logic)
// [内部] 每帧累加资源
void Building::ProduceResource(float dt) {
    // 如果满了，停止生产
    if (stored_resource_ >= stats_.resource_capacity_) {
        return;
    }

    // 计算公式：时产 / 3600 = 秒产
    // dt 是这一帧经过的秒数
    float amount_per_sec = stats_.production_rate_ / 3600.0f;
    stored_resource_ += amount_per_sec * dt;

    // 封顶检查
    if (stored_resource_ > stats_.resource_capacity_) {
        stored_resource_ = static_cast<float>(stats_.resource_capacity_);
    }
}

// [外部接口] 玩家点击收集时调用
int Building::CollectResource(int max_amount) {
    int available = static_cast<int>(stored_resource_);
    int to_collect = available;

    // 如果指定了上限，且库存充足，则只取上限值
    if (max_amount >= 0 && available > max_amount) {
        to_collect = max_amount;
    }

    // 扣除实际取走的量
    stored_resource_ -= static_cast<float>(to_collect);

    // 浮点数误差修正（防止剩余 0.0001 这种）
    if (stored_resource_ < 0.0f) stored_resource_ = 0.0f;

    return to_collect;
}

// [核心重构] 建筑充当"大脑"(索敌)，AttackComp充当"手臂"(执行)
void Building::UpdateCombatLogic(float dt) {
    // 1. 获取攻击组件
    auto attack_comp = dynamic_cast<AttackComp*>(this->getChildByName("AttackComp"));
    if (!attack_comp) return;

    // 2. 性能优化：如果组件还在冷却中，不要费劲去遍历全图索敌
    if (attack_comp->GetCooldownTimer() > 0.0f) {
        return;
    }

    auto parent_node = this->getParent();
    if (!parent_node) return;

    // 3. 索敌 (这部分逻辑依然需要 Building 自己做，因为它没有 PathAgent)
    const auto& all_nodes = parent_node->getChildren();
    BaseEntity* best_target = nullptr;
    float min_dist_sq = FLT_MAX;

    // 使用 AttackComp 计算好的距离平方 (它包含了 射程^2)
    // 但这里我们需要获取射程的数值来做初步筛选
    float range_pixels = GetRangeInPixels();
    float range_sq = range_pixels * range_pixels;

    for (auto node : all_nodes) {
        if (node == this) continue;

        auto target_entity = dynamic_cast<BaseEntity*>(node);
        if (!target_entity) continue;
        if (target_entity->get_camp() == this->get_camp()) continue; // 排除友军
        if (target_entity->IsMarkedForDestruction()) continue; // 排除将死之人

        auto hp_comp = dynamic_cast<HealthComp*>(target_entity->getChildByName("HealthComp"));
        if (hp_comp && hp_comp->IsDead()) continue; // 排除死人

        // 排除打不到的目标 (陆/空 筛选)
        auto target_unit = dynamic_cast<Unit*>(target_entity);
        if (target_unit) {
            if (!this->CanAttack(target_unit->GetGeneralType())) continue;
        }

        // 距离筛选 (简单的中心点距离，精细判断交给 AttackComp)
        float dist_sq = this->getPosition().getDistanceSq(target_entity->getPosition());
        if (dist_sq <= range_sq) {
            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                best_target = target_entity;
            }
        }
    }

    // 4. 执行攻击
    if (best_target) {
        // [委托] 直接告诉组件去打
        // 组件内部会处理：冷却重置、边缘距离精细判定、调用 CombatResolver 发射投射物
        if (attack_comp->TryAttack(best_target)) {
            // 如果攻击成功触发 (返回 true)，播放一个闪烁特效作为开火反馈
            if (visual_sprite_) {
                auto tint = cocos2d::Sequence::create(
                    cocos2d::TintTo::create(0.1f, 255, 100, 100),
                    cocos2d::TintTo::create(0.1f, 255, 255, 255),
                    nullptr
                );
                visual_sprite_->runAction(tint);
            }
        }
    }
}

// 辅助函数
std::string Building::GetSpriteFilename(Core::BuildingType type) {
    switch (type) {
    case Core::BuildingType::kTownHall:       return "Buildings_Icon/TownHall.png";
    case Core::BuildingType::kGoldMine:       return "Buildings_Icon/GoldMine.png";
    case Core::BuildingType::kElixirCollector:return "Buildings_Icon/ElixirCollector.png";
    case Core::BuildingType::kCannon:         return "Buildings_Icon/Cannon.png";
    case Core::BuildingType::kArcherTower:    return "Buildings_Icon/ArcherTower.png";
    case Core::BuildingType::kWall:           return "Buildings_Icon/Wall.png";
    case Core::BuildingType::kArmyCamp:       return "Buildings_Icon/ArmyCamp.png";
    default:                                  return "Buildings_Icon/Building_Default.png";
    }
}

// 辅助：映射建筑类型到投射物类型
Core::ProjectileType Building::GetProjectileTypeFromBuilding(Core::BuildingType type) {
    switch (type) {
    case Core::BuildingType::kCannon:
        return Core::ProjectileType::kCannonBall;
    case Core::BuildingType::kArcherTower:
        return Core::ProjectileType::kArrow;
    case Core::BuildingType::kAirDefense:
        return Core::ProjectileType::kRocket;
    default:
        return Core::ProjectileType::kNone;
    }
}

// 占地矩形 (显式使用 cocos2d::Rect)
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
