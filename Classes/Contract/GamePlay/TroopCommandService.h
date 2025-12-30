// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Source: Classes/Integration/Docs/Gameplay Troop Command API.md
//
// TroopCommandService - UI → Gameplay command surface for troop operations.
// This API is command-only; it does not expose Gameplay state beyond success/failure.
// Resulting changes must be observed via GameEvents.h.
//
// Path: Classes/Contract/GamePlay/TroopCommandService.h

#ifndef CONTRACT_GAMEPLAY_TROOP_COMMAND_SERVICE_H_
#define CONTRACT_GAMEPLAY_TROOP_COMMAND_SERVICE_H_

#include "Core/GameConstants.h"

namespace Gameplay {

/**
 * @brief 训练兵种请求
 */
struct TroopTrainRequest {
    Core::TroopType troop_type;  // 兵种类型 (必须指定)
    int level = 1;               // 等级
    int count = 1;               // 训练数量
    int owner_id = 0;            // 所有者 (0=玩家)
};

/**
 * @brief 部署兵种请求
 */
struct TroopDeployRequest {
    Core::TroopType troop_type;  // 兵种类型 (必须指定)
    int level = 1;               // 等级
    int owner_id = 0;            // 所有者
    float world_x = 0.0f;        // 部署位置 X (世界坐标)
    float world_y = 0.0f;        // 部署位置 Y (世界坐标)
};

/**
 * @brief 兵种命令服务 (单例)
 * 
 * 这是 UI → Gameplay 的命令接口。
 * 
 * 特性:
 * - Command-only: 不暴露 Gameplay 内部状态
 * - 异步结果: 命令不保证立即完成，结果通过 GameEvents.h 广播
 * 
 * 使用方式:
 * 
 *   // 训练兵种
 *   TroopTrainRequest req;
 *   req.troop_type = Core::TroopType::kBarbarian;
 *   req.count = 5;
 *   bool accepted = TroopCommandService::GetInstance()->RequestTrainTroop(req);
 *   // accepted=true 表示请求被接受，但训练完成需要监听 TroopCountUpdateEvent
 * 
 *   // 部署兵种
 *   TroopDeployRequest deploy;
 *   deploy.troop_type = Core::TroopType::kBarbarian;
 *   deploy.world_x = 100.0f;
 *   deploy.world_y = 200.0f;
 *   bool deployed = TroopCommandService::GetInstance()->RequestDeployTroop(deploy);
 */
class TroopCommandService {
public:
    static TroopCommandService* GetInstance();

    /**
     * @brief 请求训练兵种
     * 将训练请求加入队列，自动检查资源和人口
     * @param request 训练请求
     * @return true 请求被接受处理; false 请求被拒绝 (如资源不足)
     * @note 命令不保证完成，结果通过 TroopCountUpdateEvent 广播
     */
    bool RequestTrainTroop(const TroopTrainRequest& request);

    /**
     * @brief 请求部署兵种
     * 从已完成训练的库存中取出兵种，在指定位置创建 Unit
     * @param request 部署请求 (world_x/y 为世界坐标，由 UI 层转换)
     * @return true 请求被接受处理; false 请求被拒绝 (如无可用兵种)
     * @note 部署结果通过 EntitySpawnEvent 广播
     */
    bool RequestDeployTroop(const TroopDeployRequest& request);

private:
    TroopCommandService() = default;
};

}  // namespace Gameplay

#endif  // CONTRACT_GAMEPLAY_TROOP_COMMAND_SERVICE_H_
