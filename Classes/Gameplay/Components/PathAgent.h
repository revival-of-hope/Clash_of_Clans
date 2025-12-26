// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// AI Component for target selection and movement.
//
// Path: Classes/Gameplay/Components/PathAgent.h

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
 */
class PathAgent : public cocos2d::Node {
public:
    CREATE_FUNC(PathAgent);

    virtual ~PathAgent();

    virtual bool init() override;
    virtual void update(float dt) override;

    void InitStats(float move_speed, float attack_range, Core::BuildingType fav_target);
    void Stop();
    void RequestPathRecalculation();

    static void UpdateObstacle(const cocos2d::Rect& rect, bool is_blocked);
    static void ResetMap();

private:
    float move_speed_;
    float attack_range_sq_;
    Core::BuildingType favorite_target_type_;

    BaseEntity* current_target_;
    Unit* owner_unit_;

    float target_search_timer_;
    const float kSearchInterval = 0.5f;

    std::vector<cocos2d::Vec2> current_path_;
    int current_path_index_;
    float repath_timer_ = 0.0f;

    void FindNewTarget();
    void UpdateMovement(float dt);
    bool IsTargetValid() const;
    void CalculatePathTo(const cocos2d::Vec2& target_pos);
    cocos2d::Vec2 ComputeSeparationForce() const;

    static std::unordered_set<uint64_t> global_obstacle_set_;
    static std::mutex global_grid_mutex_;
};

#endif // GAMEPLAY_COMPONENTS_PATH_AGENT_H_