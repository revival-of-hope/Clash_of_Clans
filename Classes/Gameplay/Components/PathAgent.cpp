// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of PathAgent with Integrated A* Algorithm.
// [FIXED] Optimization V4 (Lag Fix):
// 1. Fixed "Infinite A* Loop" bug caused by unreachable targets (e.g., Wall Breakers).
//    Now strictly enforces repath_timer_, preventing spam when path is empty.
// 2. Added "Failure Penalty": If A* fails, wait longer before retrying.
// [FIXED] Optimization V5 (Attack Range Fix):
// 1. Fixed bug where melee units (Barbarian/Giant) couldn't attack buildings.
//    Passed current_target_ to collision check to allow units to "step into" the target's tile.
// [FIXED] Optimization V6 (Target Reachability Fix):
// 1. Fixed Critical Bug where units (especially Wall Breakers) stood still because A* considered the target (Wall) as blocked.
//    Now A* allows pathing INTO the target node by using IsGridWalkableForUnit with the target exemption.
// [REFACTOR] Adapted to Core::GameConstants changes (UnitAnimationState & Facing).

#include "PathAgent.h"
#include "GamePlay/Public/Unit.h"
#include "GamePlay/Public/Building.h"
#include "Gameplay/Components/AttackComp.h"
#include "GamePlay/Public/HealthComp.h"
#include <cmath>
#include <queue>
#include <unordered_map>
#include <algorithm>

// --- 静态成员定义 ---
std::unordered_set<uint64_t> PathAgent::global_obstacle_set_;

// --- 内部 A* 数据结构 ---
struct PathNode {
    int x, y;
    int g_cost, h_cost;
    PathNode* parent;

    int f_cost() const { return g_cost + h_cost; }

    struct Greater {
        bool operator()(const PathNode* a, const PathNode* b) const {
            return a->f_cost() > b->f_cost();
        }
    };
};

// --- 辅助函数 (Helpers) ---

static inline uint64_t PackCoord(int x, int y) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(y);
}

static cocos2d::Vec2 WorldToGrid(const cocos2d::Vec2& world_pos) {
    int x = static_cast<int>(std::floor(world_pos.x / Core::kTileWidth));
    int y = static_cast<int>(std::floor(world_pos.y / Core::kTileHeight));
    return cocos2d::Vec2(x, y);
}

static cocos2d::Vec2 GridToWorld(int x, int y) {
    return cocos2d::Vec2(
        x * Core::kTileWidth + Core::kTileWidth / 2.0f,
        y * Core::kTileHeight + Core::kTileHeight / 2.0f
    );
}

static bool IsEntityAlive(BaseEntity* entity) {
    if (!entity) return false;
    if (entity->IsMarkedForDestruction()) return false;

    auto hp = dynamic_cast<HealthComp*>(entity->getChildByName("HealthComp"));
    return !(hp && hp->IsDead());
}

static bool IsGridBlockedRaw(int x, int y, const std::unordered_set<uint64_t>& obstacles) {
    return obstacles.find(PackCoord(x, y)) != obstacles.end();
}

// 核心判定：是否可行走（包含目标豁免）
static bool IsGridWalkableForUnit(int x, int y, const std::unordered_set<uint64_t>& obstacles, BaseEntity* target) {
    // 1. 如果格子本身没被阻挡，当然可以走
    if (!IsGridBlockedRaw(x, y, obstacles)) {
        return true;
    }
    // 2. 豁免逻辑：虽然格子被阻挡了，但阻挡者正是我的目标！
    // 允许 A* 规划路径进入这个格子
    if (target) {
        auto b = dynamic_cast<Building*>(target);
        if (b) {
            cocos2d::Rect rect = b->GetOccupiedRect();
            float tileX = x * Core::kTileWidth;
            float tileY = y * Core::kTileHeight;
            cocos2d::Rect tileRect(tileX, tileY, Core::kTileWidth, Core::kTileHeight);

            // 如果这个格子属于目标建筑的一部分，则视为可行走
            if (rect.intersectsRect(tileRect)) {
                return true;
            }
        }
    }
    return false;
}

static bool IsPositionValid(const cocos2d::Vec2& pos, float radius, const std::unordered_set<uint64_t>& obstacles, BaseEntity* target) {
    float r = radius * 0.8f;

    cocos2d::Vec2 corners[4];
    corners[0] = pos + cocos2d::Vec2(-r, -r);
    corners[1] = pos + cocos2d::Vec2(r, -r);
    corners[2] = pos + cocos2d::Vec2(r, r);
    corners[3] = pos + cocos2d::Vec2(-r, r);

    for (int i = 0; i < 4; ++i) {
        cocos2d::Vec2 gridPos = WorldToGrid(corners[i]);
        if (!IsGridWalkableForUnit((int)gridPos.x, (int)gridPos.y, obstacles, target)) {
            return false;
        }
    }
    return true;
}

// 螺旋搜索 - 寻找最近的可达点
static cocos2d::Vec2 FindNearestWalkableNode(int targetX, int targetY, int startX, int startY, const std::unordered_set<uint64_t>& obstacles, BaseEntity* target) {

    // 1. 如果目标点本身对于该单位是"可行走"的（因为豁免），直接返回目标点
    if (IsGridWalkableForUnit(targetX, targetY, obstacles, target)) {
        return cocos2d::Vec2(targetX, targetY);
    }

    // 2. 否则，螺旋搜索周围的空地
    for (int r = 1; r <= 6; ++r) {
        std::vector<cocos2d::Vec2> candidates;
        for (int dx = -r; dx <= r; ++dx) {
            for (int dy = -r; dy <= r; ++dy) {
                if (std::abs(dx) != r && std::abs(dy) != r) continue;
                int nx = targetX + dx;
                int ny = targetY + dy;
                // 这里也使用宽容判定：如果是目标占据的周边格子，也可以作为终点
                if (IsGridWalkableForUnit(nx, ny, obstacles, target)) {
                    candidates.push_back(cocos2d::Vec2(nx, ny));
                }
            }
        }
        if (!candidates.empty()) {
            cocos2d::Vec2 best_node = candidates[0];
            float min_dist_sq = FLT_MAX;
            for (const auto& node : candidates) {
                float dist_sq = std::pow(node.x - startX, 2) + std::pow(node.y - startY, 2);
                if (dist_sq < min_dist_sq) {
                    min_dist_sq = dist_sq;
                    best_node = node;
                }
            }
            return best_node;
        }
    }
    return cocos2d::Vec2(targetX, targetY);
}

// --- PathAgent 实现 ---

bool PathAgent::init() {
    if (!cocos2d::Node::init()) return false;
    this->scheduleUpdate();

    move_speed_ = 0.0f;
    attack_range_sq_ = 0.0f;
    favorite_target_type_ = Core::BuildingType::kNone;
    current_target_ = nullptr;
    owner_unit_ = nullptr;

    target_search_timer_ = cocos2d::RandomHelper::random_real(0.0f, kSearchInterval);
    repath_timer_ = cocos2d::RandomHelper::random_real(0.0f, 0.2f);

    current_path_index_ = 0;

    return true;
}

PathAgent::~PathAgent() {
    if (current_target_) {
        current_target_->release();
    }
}

// --- 静态地图管理接口 ---

void PathAgent::UpdateObstacle(const cocos2d::Rect& rect, bool is_blocked) {
    int start_x = static_cast<int>(std::floor(rect.getMinX() / Core::kTileWidth));
    int start_y = static_cast<int>(std::floor(rect.getMinY() / Core::kTileHeight));
    int end_x = static_cast<int>(std::floor(rect.getMaxX() / Core::kTileWidth));
    int end_y = static_cast<int>(std::floor(rect.getMaxY() / Core::kTileHeight));

    for (int x = start_x; x < end_x; ++x) {
        for (int y = start_y; y < end_y; ++y) {
            uint64_t key = PackCoord(x, y);
            if (is_blocked) global_obstacle_set_.insert(key);
            else global_obstacle_set_.erase(key);
        }
    }
}

void PathAgent::ResetMap() {
    global_obstacle_set_.clear();
}

// --- 实例方法 ---

void PathAgent::InitStats(float move_speed, float attack_range, Core::BuildingType fav_target) {
    move_speed_ = move_speed * Core::kTileWidth;
    attack_range_sq_ = attack_range * attack_range;
    favorite_target_type_ = fav_target;
}

void PathAgent::Stop() {
    if (current_target_) {
        current_target_->release();
        current_target_ = nullptr;
    }
    current_path_.clear();
}

void PathAgent::RequestPathRecalculation() {
    current_path_.clear();
    current_path_index_ = 0;
    repath_timer_ = 0.0f;
}

void PathAgent::update(float dt) {
    if (!owner_unit_) {
        owner_unit_ = dynamic_cast<Unit*>(this->getParent());
        if (!owner_unit_) return;
    }

    if (!IsTargetValid()) {
        if (current_target_) {
            current_target_->release();
            current_target_ = nullptr;
        }
        current_path_.clear();
        target_search_timer_ -= dt;
        if (target_search_timer_ <= 0.0f) {
            target_search_timer_ = kSearchInterval;
            FindNewTarget();
        }
        if (!current_target_) {
            // [REFACTOR] Use Core::UnitAnimationState
            owner_unit_->SetState(Core::UnitAnimationState::kIdle);

            // 待机分离力
            cocos2d::Vec2 sep = ComputeSeparationForce();
            if (sep.getLengthSq() > 0.1f) {
                cocos2d::Vec2 next_pos = owner_unit_->getPosition() + sep * dt;
                if (IsPositionValid(next_pos, 10.0f, global_obstacle_set_, nullptr)) {
                    owner_unit_->setPosition(next_pos);
                }
            }
            return;
        }
    }

    UpdateMovement(dt);
}

// 计算分离力
cocos2d::Vec2 PathAgent::ComputeSeparationForce() const {
    if (!owner_unit_) return cocos2d::Vec2::ZERO;

    cocos2d::Vec2 force = cocos2d::Vec2::ZERO;
    int neighbor_count = 0;
    const float kSeparationRadius = 30.0f;
    const float kSeparationRadiusSq = kSeparationRadius * kSeparationRadius;
    const float kOptimizationDistanceSq = 50.0f * 50.0f;

    auto parent = owner_unit_->getParent();
    if (!parent) return cocos2d::Vec2::ZERO;

    cocos2d::Vec2 my_pos = owner_unit_->getPosition();

    for (auto node : parent->getChildren()) {
        if (node == owner_unit_) continue;

        if (my_pos.getDistanceSq(node->getPosition()) > kOptimizationDistanceSq) {
            continue;
        }

        auto unit = dynamic_cast<Unit*>(node);
        if (!unit) continue;
        if (!IsEntityAlive(unit)) continue;

        cocos2d::Vec2 other_pos = unit->getPosition();
        float dist_sq = my_pos.getDistanceSq(other_pos);

        if (dist_sq > 0.1f && dist_sq < kSeparationRadiusSq) {
            cocos2d::Vec2 push_dir = my_pos - other_pos;
            push_dir.normalize();
            float weight = 1.0f - (std::sqrt(dist_sq) / kSeparationRadius);
            force += push_dir * weight;
            neighbor_count++;
        }
    }

    if (neighbor_count > 0) {
        float factor = 100.0f / static_cast<float>(neighbor_count);
        force *= factor;
    }

    return force;
}

void PathAgent::UpdateMovement(float dt) {
    if (!owner_unit_ || !current_target_) return;

    cocos2d::Vec2 my_pos = owner_unit_->getPosition();
    cocos2d::Vec2 target_pos = current_target_->getPosition();

    // 1. 射程判定
    bool is_in_range = false;
    auto target_building = dynamic_cast<Building*>(current_target_);
    if (target_building) {
        cocos2d::Rect rect = target_building->GetOccupiedRect();
        float dx = 0.0f, dy = 0.0f;
        if (my_pos.x < rect.getMinX()) dx = rect.getMinX() - my_pos.x;
        else if (my_pos.x > rect.getMaxX()) dx = my_pos.x - rect.getMaxX();
        if (my_pos.y < rect.getMinY()) dy = rect.getMinY() - my_pos.y;
        else if (my_pos.y > rect.getMaxY()) dy = my_pos.y - rect.getMaxY();
        if ((dx * dx + dy * dy) <= attack_range_sq_) is_in_range = true;
    }
    else {
        float dist_sq = my_pos.getDistanceSq(target_pos);
        float effective_range = std::sqrt(attack_range_sq_) + 20.0f;
        if (dist_sq <= effective_range * effective_range) is_in_range = true;
    }

    if (is_in_range) {
        // [REFACTOR] Use Core::UnitAnimationState
        owner_unit_->SetState(Core::UnitAnimationState::kAttack);
        auto attack_comp = dynamic_cast<AttackComp*>(owner_unit_->getChildByName("AttackComp"));
        if (attack_comp) attack_comp->TryAttack(current_target_);
        if (!current_path_.empty()) current_path_.clear();
        return;
    }

    // 2. 寻路计算控制
    if (repath_timer_ > 0.0f) {
        repath_timer_ -= dt;
    }
    else {
        if (current_path_.empty() || current_path_index_ >= current_path_.size()) {

            CalculatePathTo(target_pos);

            repath_timer_ = 0.5f;

            if (current_path_.empty()) {
                repath_timer_ = 1.0f; // 失败惩罚
                // [REFACTOR] Use Core::UnitAnimationState
                owner_unit_->SetState(Core::UnitAnimationState::kIdle);
                return;
            }
        }
    }

    // 3. 执行移动
    if (current_path_.empty()) {
        return;
    }

    const float kUnitRadius = 10.0f;
    cocos2d::Vec2 desired_velocity = cocos2d::Vec2::ZERO;

    if (current_path_index_ < current_path_.size()) {
        cocos2d::Vec2 next_wp = current_path_[current_path_index_];
        if (my_pos.getDistanceSq(next_wp) <= 25.0f) {
            current_path_index_++;
        }
        else {
            desired_velocity = (next_wp - my_pos).getNormalized() * move_speed_;
        }
    }

    if (desired_velocity.isZero()) {
        // [REFACTOR] Use Core::UnitAnimationState
        owner_unit_->SetState(Core::UnitAnimationState::kIdle);
        return;
    }

    // 4. 融合分离力
    cocos2d::Vec2 separation = ComputeSeparationForce();
    cocos2d::Vec2 final_velocity = desired_velocity + separation;
    cocos2d::Vec2 velocity_vector = final_velocity * dt;
    cocos2d::Vec2 next_pos = my_pos + velocity_vector;

    // [REFACTOR] 更新朝向 (Facing Logic)
    if (desired_velocity.x < -0.1f) {
        owner_unit_->SetFacing(Core::Facing::kLeft);
    }
    else if (desired_velocity.x > 0.1f) {
        owner_unit_->SetFacing(Core::Facing::kRight);
    }

    // 5. 碰撞检测与滑动逻辑 (Sliding)
    bool is_safe_to_move = false;

    if (IsPositionValid(next_pos, kUnitRadius, global_obstacle_set_, current_target_)) {
        is_safe_to_move = true;
    }
    else {
        cocos2d::Vec2 next_pos_x = my_pos + cocos2d::Vec2(velocity_vector.x, 0);
        if (std::abs(velocity_vector.x) > 0.01f && IsPositionValid(next_pos_x, kUnitRadius, global_obstacle_set_, current_target_)) {
            next_pos = next_pos_x;
            is_safe_to_move = true;
        }
        else {
            cocos2d::Vec2 next_pos_y = my_pos + cocos2d::Vec2(0, velocity_vector.y);
            if (std::abs(velocity_vector.y) > 0.01f && IsPositionValid(next_pos_y, kUnitRadius, global_obstacle_set_, current_target_)) {
                next_pos = next_pos_y;
                is_safe_to_move = true;
            }
        }
    }

    if (is_safe_to_move) {
        owner_unit_->setPosition(next_pos);
        // [REFACTOR] Use Core::UnitAnimationState
        owner_unit_->SetState(Core::UnitAnimationState::kMove);
    }
    else {
        current_path_.clear();
        // [REFACTOR] Use Core::UnitAnimationState
        owner_unit_->SetState(Core::UnitAnimationState::kIdle);
    }
}

// --- 核心 A* 算法实现 ---
void PathAgent::CalculatePathTo(const cocos2d::Vec2& target_pos) {
    current_path_.clear();
    current_path_index_ = 0;

    cocos2d::Vec2 start_pos = owner_unit_->getPosition();
    cocos2d::Vec2 grid_start_v = WorldToGrid(start_pos);
    cocos2d::Vec2 grid_target_v = WorldToGrid(target_pos);

    int startX = (int)grid_start_v.x;
    int startY = (int)grid_start_v.y;
    int rawTargetX = (int)grid_target_v.x;
    int rawTargetY = (int)grid_target_v.y;

    cocos2d::Vec2 actual_end = FindNearestWalkableNode(rawTargetX, rawTargetY, startX, startY, global_obstacle_set_, current_target_);
    int endX = (int)actual_end.x;
    int endY = (int)actual_end.y;

    if (startX == endX && startY == endY) return;

    std::priority_queue<PathNode*, std::vector<PathNode*>, PathNode::Greater> open_list;
    std::unordered_set<uint64_t> closed_set;
    std::unordered_map<uint64_t, PathNode*> all_nodes;
    std::vector<PathNode*> node_pool;

    PathNode* start_node = new PathNode{ startX, startY, 0, 0, nullptr };
    start_node->h_cost = (std::abs(startX - endX) + std::abs(startY - endY)) * 10;

    open_list.push(start_node);
    node_pool.push_back(start_node);
    all_nodes[PackCoord(startX, startY)] = start_node;

    PathNode* final_node = nullptr;
    int max_steps = 3000;

    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { 1, -1, 0, 0 };
    int costs[] = { 10, 10, 10, 10 };

    while (!open_list.empty()) {
        PathNode* current = open_list.top();
        open_list.pop();

        if (max_steps-- <= 0) break;

        if (current->x == endX && current->y == endY) {
            final_node = current;
            break;
        }

        uint64_t c_key = PackCoord(current->x, current->y);
        if (closed_set.count(c_key)) continue;
        closed_set.insert(c_key);

        for (int i = 0; i < 4; ++i) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];
            uint64_t n_key = PackCoord(nx, ny);

            bool is_walkable = IsGridWalkableForUnit(nx, ny, global_obstacle_set_, current_target_);
            if (!is_walkable) continue;

            if (closed_set.count(n_key)) continue;

            int new_g = current->g_cost + costs[i];

            auto it = all_nodes.find(n_key);
            if (it != all_nodes.end()) {
                PathNode* existing = it->second;
                if (new_g < existing->g_cost) {
                    existing->g_cost = new_g;
                    existing->parent = current;
                    open_list.push(existing);
                }
            }
            else {
                PathNode* neighbor = new PathNode{ nx, ny, new_g, 0, current };
                neighbor->h_cost = (std::abs(nx - endX) + std::abs(ny - endY)) * 10;
                open_list.push(neighbor);
                node_pool.push_back(neighbor);
                all_nodes[n_key] = neighbor;
            }
        }
    }

    if (final_node) {
        PathNode* temp = final_node;
        while (temp) {
            current_path_.push_back(GridToWorld(temp->x, temp->y));
            temp = temp->parent;
        }
        std::reverse(current_path_.begin(), current_path_.end());
        if (current_path_.size() > 1) {
            current_path_index_ = 1;
        }
    }

    for (auto n : node_pool) delete n;
}

void PathAgent::FindNewTarget() {
    if (current_target_) {
        current_target_->release();
        current_target_ = nullptr;
    }
    auto scene = owner_unit_->getParent();
    if (!scene) return;

    const auto& all_nodes = scene->getChildren();

    BaseEntity* best_target = nullptr;
    float min_dist_sq = FLT_MAX;

    BaseEntity* best_fav_target = nullptr;
    float min_fav_dist_sq = FLT_MAX;

    bool has_favorite = (favorite_target_type_ != Core::BuildingType::kNone);
    cocos2d::Vec2 my_pos = owner_unit_->getPosition();

    const float kVisionRangeSq = 1000.0f * 1000.0f;

    auto checkCanAttack = [&](BaseEntity* entity) -> bool {
        Core::GeneralType target_type = Core::GeneralType::kGround;
        auto u = dynamic_cast<Unit*>(entity);
        if (u) target_type = u->GetGeneralType();
        return owner_unit_->CanAttack(target_type);
        };

    for (auto node : all_nodes) {
        float dist_sq = my_pos.getDistanceSq(node->getPosition());
        if (dist_sq > kVisionRangeSq) continue;

        auto entity = dynamic_cast<BaseEntity*>(node);
        if (!entity) continue;

        if (entity == owner_unit_) continue;
        if (entity->get_camp() == owner_unit_->get_camp()) continue;
        if (!IsEntityAlive(entity)) continue;
        if (!checkCanAttack(entity)) continue;

        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            best_target = entity;
        }

        if (has_favorite) {
            auto building = dynamic_cast<Building*>(entity);
            if (building) {
                bool is_valid_fav = false;
                if (favorite_target_type_ == Core::BuildingType::kCannon) {
                    if (building->GetDamage() > 0) is_valid_fav = true;
                }
                else {
                    if (building->GetBuildingType() == favorite_target_type_) is_valid_fav = true;
                }

                if (is_valid_fav && dist_sq < min_fav_dist_sq) {
                    min_fav_dist_sq = dist_sq;
                    best_fav_target = building;
                }
            }
        }
    }

    if (best_fav_target) {
        current_target_ = best_fav_target;
    }
    else {
        current_target_ = best_target;
    }

    if (current_target_) {
        current_target_->retain();
        RequestPathRecalculation();
    }
}

bool PathAgent::IsTargetValid() const {
    return current_target_ && IsEntityAlive(current_target_);
}