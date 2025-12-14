// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of PathAgent.

#include "PathAgent.h"
#include "Gameplay/Entities/Unit.h"
#include "Gameplay/Entities/Building.h"
#include "Gameplay/Components/AttackComp.h"
#include "Gameplay/Components/HealthComp.h"//用于检查目标是否逻辑死亡
#include <cmath>

// 静态辅助函数: 检查实体是否"存活"且"有效"
static bool IsEntityAlive(BaseEntity* entity) {
    // 1. 空指针检查
    if (!entity) return false;

    // 2. 内存销毁检查 (Cocos 层面的销毁)
    if (entity->IsMarkedForDestruction()) return false;

    // 3. 逻辑死亡检查 (游戏层面的死亡)
    auto hp = dynamic_cast<HealthComp*>(entity->getChildByName("HealthComp"));
    if (hp && hp->IsDead()) {
        return false;
    }

    return true;
}

bool PathAgent::init() {
    if (!cocos2d::Node::init()) return false;

    this->scheduleUpdate();

    move_speed_ = 0.0f;
    attack_range_sq_ = 0.0f;
    favorite_target_type_ = Core::BuildingType::kNone;
    current_target_ = nullptr;
    owner_unit_ = nullptr;
    target_search_timer_ = 0.0f; // 确保第一帧就搜索

    return true;
}

// 析构时确保释放引用，防止内存泄漏
PathAgent::~PathAgent() {
    if (current_target_) {
        current_target_->release();
        current_target_ = nullptr;
    }
}

void PathAgent::InitStats(float move_speed, float attack_range, Core::BuildingType fav_target) {
    move_speed_ = move_speed * Core::kTileWidth; // 将 速度(格/秒) 转换为 (像素/秒)
    attack_range_sq_ = attack_range * attack_range; // 预计算平方
    favorite_target_type_ = fav_target;
}

void PathAgent::Stop() {
    if (current_target_) {
        current_target_->release();
        current_target_ = nullptr;
    }
}

void PathAgent::update(float dt) {
    // 1. 获取Owner
    if (!owner_unit_) {
        // 第一次运行时获取父节点
        owner_unit_ = dynamic_cast<Unit*>(this->getParent());
        if (!owner_unit_) return; // 还没挂载到 Unit 上
    }

    // 2. 检查当前目标状态
    if (!IsTargetValid()) {
        if (current_target_) {
            current_target_->release();
            current_target_ = nullptr;
        }
        // 只有当计时器归零时才进行全图搜索
        target_search_timer_ -= dt;
        if (target_search_timer_ <= 0.0f) {
            target_search_timer_ = kSearchInterval; // 重置计时器 (例如 0.5s)
            FindNewTarget();
        }

        // 如果还是找不到 (比如全场推平了)，就发呆
        if (!current_target_) {
            // 无目标则待机
            owner_unit_->SetState(Unit::State::kIdle);
            return;
        }
    }

    // 3. 执行移动或攻击决策
    UpdateMovement(dt);
}

void PathAgent::UpdateMovement(float dt) {
    if (!owner_unit_ || !current_target_) return;

    cocos2d::Vec2 my_pos = owner_unit_->getPosition();
    cocos2d::Vec2 target_pos = current_target_->getPosition();

    // [完善的射程判定] (Robust Range Check - Edge to Edge)
    // 解决 "大型单位穿模" 问题。

    bool is_in_range = false;
    auto target_building = dynamic_cast<Building*>(current_target_);

    if (target_building) {
        // CASE A: 目标是建筑

        cocos2d::Rect rect = target_building->GetOccupiedRect();
        float dx = 0.0f;
        float dy = 0.0f;

        // 计算 Unit 中心点距离矩形四边的距离
        if (my_pos.x < rect.getMinX()) dx = rect.getMinX() - my_pos.x;
        else if (my_pos.x > rect.getMaxX()) dx = my_pos.x - rect.getMaxX();

        if (my_pos.y < rect.getMinY()) dy = rect.getMinY() - my_pos.y;
        else if (my_pos.y > rect.getMaxY()) dy = my_pos.y - rect.getMaxY();

        float dist_to_edge_sq = dx * dx + dy * dy;

        // 判定: 边缘距离 <= 攻击射程
        if (dist_to_edge_sq <= attack_range_sq_) {
            is_in_range = true;
        }
    }
    else {
        // CASE B: 目标是单位 (圆形/点)

        float target_radius = 20.0f; // 假设单位半径
        float dist_sq = my_pos.getDistanceSq(target_pos);

        // 我们需要判定: dist <= range + radius
        // 即: dist^2 <= (range + radius)^2
        float range_val = std::sqrt(attack_range_sq_); // 开方还原
        float effective_range = range_val + target_radius;

        if (dist_sq <= effective_range * effective_range) {
            is_in_range = true;
        }
    }

    // 行为执行

    if (is_in_range) {
        // 已经在射程内 -> 停止移动，尝试攻击
        // 状态同步: 攻击
        owner_unit_->SetState(Unit::State::kAttack);
        auto attack_comp = dynamic_cast<AttackComp*>(owner_unit_->getChildByName("AttackComp"));
        if (attack_comp) {
            // TryAttack 内部会处理冷却时间
            attack_comp->TryAttack(current_target_);
        }
        return;
    }

    // 尚未到达 -> 移动逻辑
    // 状态同步: 移动
    owner_unit_->SetState(Unit::State::kMove);
    // 计算方向向量 (归一化)
    cocos2d::Vec2 direction = (target_pos - my_pos).getNormalized();
    // 简单的防抖动：如果距离非常近，不要乱动
    if (direction.length() > 0.0f) {
        cocos2d::Vec2 displacement = direction * move_speed_ * dt;
        owner_unit_->setPosition(my_pos + displacement);
        if (direction.x < 0) owner_unit_->setScaleX(-1);
        else if (direction.x > 0) owner_unit_->setScaleX(1);
    }
}

void PathAgent::FindNewTarget() {
    // 搜索前先清理旧目标（防御性）
    if (current_target_) {
        current_target_->release();
        current_target_ = nullptr;
    }
    // 获取场景根节点
    auto scene = owner_unit_->getParent();
    if (!scene) return;

    const auto& all_nodes = scene->getChildren();

    BaseEntity* best_target = nullptr;
    float min_dist_sq = FLT_MAX;

    // 优先寻找 "偏好目标" (例如巨人找防御塔)
    bool search_favorite_only = (favorite_target_type_ != Core::BuildingType::kNone);

    // 辅助 lambda: 检查是否能攻击
    auto checkCanAttack = [&](BaseEntity* entity) -> bool {
        Core::GeneralType target_type = Core::GeneralType::kGround; // 默认为地面 (如建筑)

        // 如果是 Unit，获取其真实类型 (陆/空)
        auto u = dynamic_cast<Unit*>(entity);
        if (u) {
            target_type = u->GetGeneralType();
        }

        // 调用 Unit::CanAttack 进行能力判断
        return owner_unit_->CanAttack(target_type);
        };

    // 阶段一：尝试寻找偏好目标
    if (search_favorite_only) {
        for (auto node : all_nodes) {
            auto building = dynamic_cast<Building*>(node);
            if (!building) continue;
            if (building->get_camp() == owner_unit_->get_camp()) continue; // 不打友军
            if (!IsEntityAlive(building)) continue; // 使用 IsEntityAlive 过滤假死目标

            // [修复] 检查能力
            if (!checkCanAttack(building)) continue;

            bool is_valid_fav = false;
            if (favorite_target_type_ == Core::BuildingType::kCannon) {
                // 巨人的特殊逻辑：所有有攻击力的都算防御塔
                if (building->GetDamage() > 0) is_valid_fav = true;
            }
            else {
                if (building->GetBuildingType() == favorite_target_type_) is_valid_fav = true;
            }

            if (is_valid_fav) {
                float dist_sq = owner_unit_->getPosition().getDistanceSq(building->getPosition());
                if (dist_sq < min_dist_sq) {
                    min_dist_sq = dist_sq;
                    best_target = building;
                }
            }
        }
    }

    // 阶段二：如果阶段一没找到 (或者根本没有偏好)，则寻找任意最近建筑
    if (!best_target) {
        min_dist_sq = FLT_MAX; // 重置距离
        for (auto node : all_nodes) {
            auto entity = dynamic_cast<BaseEntity*>(node);
            if (!entity) continue;
            if (entity == owner_unit_) continue;
            if (entity->get_camp() == owner_unit_->get_camp()) continue;
            if (!IsEntityAlive(entity)) continue; // 过滤假死

            // [修复] 检查能力
            if (!checkCanAttack(entity)) continue;

            // 只要是活着的敌人就行
            float dist_sq = owner_unit_->getPosition().getDistanceSq(entity->getPosition());
            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                best_target = entity;
            }
        }
    }

    current_target_ = best_target;

    // [关键修复] 锁定目标后，增加引用计数！
    if (current_target_) {
        current_target_->retain();
        cocos2d::log("PathAgent[%d] locked target[%d]. Retain called.", owner_unit_->get_instance_id(), current_target_->get_instance_id());
    }
}

bool PathAgent::IsTargetValid() const {
    if (!current_target_) return false;
    // 如果目标正在播放死亡动画，或者已经被内存回收，都视为无效
    if (!IsEntityAlive(current_target_)) return false;

    return true;
}