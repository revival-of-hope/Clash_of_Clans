// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of TroopCommandService.
// This service acts as a command interface between UI and Gameplay.
// All state changes are broadcast via GameEvents.h.
//
// Path: Classes/Gameplay/Logic/TroopCommandService.cpp

#include "Contract/GamePlay/TroopCommandService.h"
#include "Contract/GamePlay/EconomySystem.h"
#include "Contract/GamePlay/CostQuery.h"
#include "Contract/GamePlay/GameEvents.h"
#include "Contract/GamePlay/Unit.h"
#include "Core/GameConfig.h"
#include "cocos2d.h"
#include <vector>

namespace Gameplay {

// =============================================================================
// 内部数据结构 (不暴露给外部)
// =============================================================================

namespace {

/**
 * @brief 训练中的兵种数据
 */
struct TrainingSlot {
    Core::TroopType type;
    int level = 1;
    int owner_id = 0;
    float time_total = 0.0f;
    float time_remaining = 0.0f;
};

/**
 * @brief 已完成训练的兵种库存
 */
struct ReadyTroopSlot {
    Core::TroopType type;
    int level = 1;
    int count = 0;
};

// 内部状态
std::vector<TrainingSlot> g_training_queue;
std::vector<ReadyTroopSlot> g_ready_troops;
int g_reserved_population = 0;
bool g_initialized = false;

// =============================================================================
// 辅助函数
// =============================================================================

float GetTrainingTime(Core::TroopType type, int /*level*/) {
    switch (type) {
    case Core::TroopType::kBarbarian:   return 5.0f;
    case Core::TroopType::kArcher:      return 6.0f;
    case Core::TroopType::kGiant:       return 30.0f;
    case Core::TroopType::kWallBreaker: return 30.0f;
    case Core::TroopType::kBabyDragon:  return 45.0f;
    default:                            return 10.0f;
    }
}

int GetHousingSpace(Core::TroopType type) {
    auto stats = Core::GameConfig::GetInstance()->GetTroopStats(type, 1);
    return stats.housing_space_;
}

int GetReadyCount(Core::TroopType type) {
    for (const auto& slot : g_ready_troops) {
        if (slot.type == type) {
            return slot.count;
        }
    }
    return 0;
}

void AddReadyTroop(Core::TroopType type, int level) {
    for (auto& slot : g_ready_troops) {
        if (slot.type == type && slot.level == level) {
            slot.count++;
            return;
        }
    }
    
    ReadyTroopSlot new_slot;
    new_slot.type = type;
    new_slot.level = level;
    new_slot.count = 1;
    g_ready_troops.push_back(new_slot);
}

bool RemoveReadyTroop(Core::TroopType type, int* out_level) {
    for (auto it = g_ready_troops.begin(); it != g_ready_troops.end(); ++it) {
        if (it->type == type && it->count > 0) {
            if (out_level) *out_level = it->level;
            it->count--;
            
            if (it->count <= 0) {
                g_ready_troops.erase(it);
            }
            return true;
        }
    }
    return false;
}

void OnTrainingComplete(const TrainingSlot& slot) {
    AddReadyTroop(slot.type, slot.level);
    
    cocos2d::log("TroopCommandService: Training complete! Type=%d, Level=%d",
                 static_cast<int>(slot.type), slot.level);
    
    // 广播兵种数量更新事件 (UI 通过此事件获取状态)
    TroopCountUpdateEvent evt;
    evt.owner_id = slot.owner_id;
    evt.troop_type = slot.type;
    evt.remaining_count = GetReadyCount(slot.type);
    GameEventManager::GetInstance()->BroadcastTroopCountUpdated(evt);
}

void UpdateTraining(float dt) {
    if (g_training_queue.empty()) return;
    
    TrainingSlot& current = g_training_queue.front();
    current.time_remaining -= dt;
    
    if (current.time_remaining <= 0.0f) {
        OnTrainingComplete(current);
        g_training_queue.erase(g_training_queue.begin());
    }
}

void EnsureInitialized() {
    if (g_initialized) return;
    
    auto director = cocos2d::Director::getInstance();
    auto scheduler = director->getScheduler();
    
    // 使用全局调度器进行训练更新
    scheduler->schedule([](float dt) {
        UpdateTraining(dt);
    }, const_cast<void*>(static_cast<const void*>(&g_initialized)),  // 使用静态变量地址作为key
       0.0f, false, "TroopCommandService_Update");
    
    g_initialized = true;
    cocos2d::log("TroopCommandService: Initialized");
}

}  // anonymous namespace

// =============================================================================
// TroopCommandService 实现
// =============================================================================

TroopCommandService* TroopCommandService::GetInstance() {
    static TroopCommandService instance;
    return &instance;
}

bool TroopCommandService::RequestTrainTroop(const TroopTrainRequest& request) {
    EnsureInitialized();
    
    cocos2d::log("TroopCommandService: RequestTrainTroop Type=%d, Level=%d, Count=%d",
                 static_cast<int>(request.troop_type), request.level, request.count);
    
    // 获取训练费用
    ResourceCost cost = CostQuery::GetInstance()->GetTroopTrainingCost(
        request.troop_type, request.level);
    
    // 获取人口占用
    int housing = GetHousingSpace(request.troop_type);
    
    // 逐个训练
    int trained_count = 0;
    
    for (int i = 0; i < request.count; i++) {
        // 检查资源
        if (!EconomySystem::GetInstance()->CanAffordCost(cost, false)) {
            cocos2d::log("TroopCommandService: Not enough resources");
            break;
        }
        
        // 检查人口空间
        int current_pop = EconomySystem::GetInstance()->GetCurrentPopulation();
        int max_pop = EconomySystem::GetInstance()->GetMaxPopulation();
        
        if (current_pop + g_reserved_population + housing > max_pop) {
            cocos2d::log("TroopCommandService: Not enough housing space");
            break;
        }
        
        // 扣除资源
        if (!EconomySystem::GetInstance()->SpendCost(cost)) {
            break;
        }
        
        // 预留人口
        g_reserved_population += housing;
        
        // 加入训练队列
        TrainingSlot slot;
        slot.type = request.troop_type;
        slot.level = request.level;
        slot.owner_id = request.owner_id;
        slot.time_total = GetTrainingTime(request.troop_type, request.level);
        slot.time_remaining = slot.time_total;
        
        g_training_queue.push_back(slot);
        trained_count++;
    }
    
    // 返回是否至少接受了一个训练请求
    return trained_count > 0;
}

bool TroopCommandService::RequestDeployTroop(const TroopDeployRequest& request) {
    EnsureInitialized();
    
    cocos2d::log("TroopCommandService: RequestDeployTroop Type=%d at (%.0f, %.0f)",
                 static_cast<int>(request.troop_type), request.world_x, request.world_y);
    
    // 检查是否有可用兵种
    int ready_count = GetReadyCount(request.troop_type);
    if (ready_count <= 0) {
        cocos2d::log("TroopCommandService: No ready troops of type %d",
                     static_cast<int>(request.troop_type));
        return false;
    }
    
    // 从库存中移除
    int level = request.level;
    if (!RemoveReadyTroop(request.troop_type, &level)) {
        return false;
    }
    
    // 释放预留人口，转为实际占用
    int housing = GetHousingSpace(request.troop_type);
    g_reserved_population -= housing;
    if (g_reserved_population < 0) g_reserved_population = 0;
    
    EconomySystem::GetInstance()->AddTroopPopulation(housing);
    
    // 创建 Unit 实体
    Unit* unit = Unit::create(request.troop_type, level, request.owner_id);
    if (!unit) {
        cocos2d::log("TroopCommandService: Failed to create Unit");
        return false;
    }
    
    unit->setPosition(cocos2d::Vec2(request.world_x, request.world_y));
    
    // 添加到场景
    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    if (scene) {
        auto battle_layer = scene->getChildByName("BattleLayer");
        if (battle_layer) {
            battle_layer->addChild(unit);
        } else {
            scene->addChild(unit);
        }
    }
    
    cocos2d::log("TroopCommandService: Deployed successfully");
    
    // 广播兵种数量更新 (UI 通过此事件更新显示)
    TroopCountUpdateEvent evt;
    evt.owner_id = request.owner_id;
    evt.troop_type = request.troop_type;
    evt.remaining_count = GetReadyCount(request.troop_type);
    GameEventManager::GetInstance()->BroadcastTroopCountUpdated(evt);
    
    // Unit::onEnter() 会自动广播 EntitySpawnEvent
    
    return true;
}

}  // namespace Gameplay
