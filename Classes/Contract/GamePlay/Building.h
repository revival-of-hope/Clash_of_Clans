// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Building class. Handles construction, resources, and defensive logic.
// [UPDATE] Added animation controller, GameEvents support
//
// Path: Classes/Contract/GamePlay/Building.h

#ifndef CONTRACT_GAMEPLAY_BUILDING_H_
#define CONTRACT_GAMEPLAY_BUILDING_H_

#include "Contract/GamePlay/BaseEntity.h"
#include "Core/GameConfig.h"

// 前向声明
class EntityAnimationController;

/**
 * @brief 建筑实体类
 * 包含建造、升级、资源生产和防御攻击的逻辑。
 */
class Building : public BaseEntity {
public:
    // 静态创建函数，自动处理内存管理 (autorelease)
    static Building* create(Core::BuildingType type, int level, int owner_id);

    // 初始化函数：读取配置、加载图片
    virtual bool init(Core::BuildingType type, int level, int owner_id);

    // 生命周期函数 - 用于障碍物注册和 GameEvents
    virtual void onEnter() override;
    virtual void onExit() override;

    // 每一帧更新：处理攻击冷却、资源产出、建造进度
    virtual void update(float dt) override;

    // ==========================================================================
    // 属性查询接口
    // ==========================================================================

    int GetLevel() const { return level_; }
    Core::BuildingType GetBuildingType() const { return type_; }
    Core::BuildingAnimationState GetState() const { return current_state_; }
    int GetWidthInTiles() const { return stats_.width_; }
    int GetHeightInTiles() const { return stats_.height_; }

    /**
     * @brief 获取占地矩形 (像素坐标)
     */
    cocos2d::Rect GetOccupiedRect() const;

    // ==========================================================================
    // 建造接口
    // ==========================================================================

    /**
     * @brief 检查是否正在建造/升级中
     */
    bool IsConstructing() const {
        return current_state_ == Core::BuildingAnimationState::kConstructing;
    }

    /**
     * @brief 开始建造或升级
     * @param duration 建造所需时间 (秒)
     */
    void StartConstruction(float duration);

    /**
     * @brief 获取建造进度 (0.0 ~ 1.0)
     */
    float GetConstructionProgress() const;

    // ==========================================================================
    // 战斗接口
    // ==========================================================================

    /**
     * @brief 判断是否能攻击目标
     * @param target_type 目标的类型 (地/空)
     */
    bool CanAttack(Core::GeneralType target_type) const {
        return (static_cast<unsigned int>(stats_.target_type_) &
            static_cast<unsigned int>(target_type)) != 0;
    }

    int GetDamage() const { return stats_.damage_; }
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }

    // ==========================================================================
    // 经济接口
    // ==========================================================================

    /**
     * @brief 获取当前存储的资源量 (针对金矿/收集器)
     */
    int GetStoredResource() const { return static_cast<int>(stored_resource_); }

    /**
     * @brief 获取存储百分比 (0.0 ~ 1.0)
     */
    float GetStoragePercentage() const;

    /**
     * @brief 收集资源
     * @param max_amount 此次收集的最大限额 (-1表示不限)
     * @return int 实际收集到的资源量
     */
    int CollectResource(int max_amount = -1);

    // ==========================================================================
    // 动画接口
    // ==========================================================================

    /**
     * @brief 播放攻击动画
     */
    void PlayAttackAnimation();

    /**
     * @brief 播放受伤闪烁效果
     */
    void PlayHurtEffect();

    /**
     * @brief 播放摧毁动画
     */
    void PlayDestroyedAnimation();

private:
    // 核心数据
    Core::BuildingStats stats_;
    Core::BuildingType type_;
    int level_;

    // 状态
    Core::BuildingAnimationState current_state_ = Core::BuildingAnimationState::kIdle;
    float construction_timer_ = 0.0f;
    float construction_duration_ = 0.0f;

    // 经济状态
    float stored_resource_ = 0.0f;

    // 视觉组件
    cocos2d::Sprite* visual_sprite_ = nullptr;
    EntityAnimationController* animation_controller_ = nullptr;

    // 障碍物注册标记
    bool obstacle_registered_ = false;

    // ==========================================================================
    // 辅助函数
    // ==========================================================================

    /**
     * @brief 切换建筑状态
     */
    void SetState(Core::BuildingAnimationState new_state);

    /**
     * @brief 战斗逻辑更新
     */
    void UpdateCombatLogic(float dt);

    /**
     * @brief 资源生产逻辑
     */
    void ProduceResource(float dt);

    /**
     * @brief 根据建筑类型返回精灵图文件名
     */
    static std::string GetSpriteSheetFilename(Core::BuildingType type);

    /**
     * @brief 根据建筑类型获取帧尺寸
     */
    static void GetFrameSize(Core::BuildingType type, int& out_width, int& out_height);

    /**
     * @brief 将建筑类型转换为投射物类型
     */
    static Core::ProjectileType GetProjectileTypeFromBuilding(Core::BuildingType type);
};

#endif // CONTRACT_GAMEPLAY_BUILDING_H_