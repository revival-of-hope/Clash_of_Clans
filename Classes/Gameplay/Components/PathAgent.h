// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// AI Component for target selection and movement.

#ifndef GAMEPLAY_COMPONENTS_PATH_AGENT_H_
#define GAMEPLAY_COMPONENTS_PATH_AGENT_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"

// 前向声明，减少头文件依赖
class BaseEntity;
class Unit;

/**
 * @brief 寻路与 AI 代理组件
 * 作用:
 * 1. 索敌 (寻找最近的/偏好的目标)
 * 2. 移动控制 (控制 Unit 的坐标)
 * 3. 状态切换 (通知 Unit 什么时候该跑，什么时候该打)
 */
class PathAgent : public cocos2d::Node {
public:
    CREATE_FUNC(PathAgent);

    virtual ~PathAgent();

    virtual bool init() override;
    virtual void update(float dt) override;

    //初始化代理属性
    void InitStats(float move_speed, float attack_range, Core::BuildingType fav_target);

    // 强制停止/重置
    void Stop();

private:
    // --- 属性 ---
    float move_speed_;
    float attack_range_sq_; // 范围平方
    Core::BuildingType favorite_target_type_;

    // 当前锁定的目标
    BaseEntity* current_target_;

    // 缓存父节点指针，避免每帧 dynamic_cast
    Unit* owner_unit_;

    // 性能优化: 索敌计时器
    // 避免每一帧都遍历全图寻找目标 (CPU 杀手)
    float target_search_timer_;
    const float kSearchInterval = 0.5f; // 每 0.5 秒搜索一次

    // 寻找新目标
    void FindNewTarget();

    // 移动逻辑 (目前是直线移动，预留 A* 接口)
    void UpdateMovement(float dt);

    // 检查目标是否依然活着且在场上
    bool IsTargetValid() const;
};

#endif // GAMEPLAY_COMPONENTS_PATH_AGENT_H_