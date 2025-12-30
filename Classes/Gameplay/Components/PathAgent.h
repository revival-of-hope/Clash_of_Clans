// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// AI Component for target selection and movement.
// INTEGRATED PATHFINDING: Contains static map data and A* logic.
// [FIXED] Added Separation Force to prevent unit overlapping/clipping.

#ifndef GAMEPLAY_COMPONENTS_PATH_AGENT_H_
#define GAMEPLAY_COMPONENTS_PATH_AGENT_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"
#include <vector>
#include <unordered_set>
#include <mutex>

class BaseEntity;
class Unit;

/**
 * @brief 寻路与 AI 代理组件
 * * 职责:
 * 1. [静态] 管理全局障碍物网格 (无限制大小)。
 * 2. [实例] 负责单个单位的索敌和移动控制。
 * 3. [新增] 负责单位之间的动态避让 (Separation)。
 */
class PathAgent : public cocos2d::Node {
public:
    CREATE_FUNC(PathAgent);

    virtual ~PathAgent();

    virtual bool init() override;
    virtual void update(float dt) override;

    // --- 实例方法 (Instance Methods) ---

    void InitStats(float move_speed, float attack_range, Core::BuildingType fav_target);
    void Stop();
    void RequestPathRecalculation();

    // --- 静态全地图管理接口 (Static Map Management) ---

    /**
     * @brief 更新全局地图的障碍物信息
     * 由 Building 在初始化或销毁时调用
     */
    static void UpdateObstacle(const cocos2d::Rect& rect, bool is_blocked);

    /**
     * @brief 重置全局地图 (场景切换时调用)
     */
    static void ResetMap();

private:
    // --- 实例属性 ---
    float move_speed_;
    float attack_range_sq_;
    Core::BuildingType favorite_target_type_;

    BaseEntity* current_target_;
    Unit* owner_unit_;

    float target_search_timer_;
    const float kSearchInterval = 0.5f;

    // 路径数据
    std::vector<cocos2d::Vec2> current_path_;
    int current_path_index_;
    float repath_timer_ = 0.0f;

    // --- 内部逻辑 ---
    void FindNewTarget();
    void UpdateMovement(float dt);
    bool IsTargetValid() const;

    // 执行 A* 寻路
    void CalculatePathTo(const cocos2d::Vec2& target_pos);

    // [新增] 计算分离力：返回一个向量，代表应该往哪个方向躲避队友
    cocos2d::Vec2 ComputeSeparationForce() const;

    // --- 静态地图数据 (Shared Map Data) ---
    // 使用哈希集合存储障碍物坐标 (key = x << 32 | y)，支持无限地图
    static std::unordered_set<uint64_t> global_obstacle_set_;
    static std::mutex global_grid_mutex_;
};

#endif // GAMEPLAY_COMPONENTS_PATH_AGENT_H_