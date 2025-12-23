// Building class. Handles construction, resources, and defensive logic.
// [REFACTOR] Adopted Core::BuildingAnimationState for state management.

#ifndef GAMEPLAY_ENTITIES_BUILDING_H_
#define GAMEPLAY_ENTITIES_BUILDING_H_

#include "BaseEntity.h"
#include "Core/GameConfig.h"

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

    // [新增] 覆盖生命周期函数，确保在坐标设置完成后才注册障碍物
    virtual void onEnter() override;
    virtual void onExit() override;

    // 每一帧更新：处理攻击冷却、资源产出
    virtual void update(float dt) override;

    // 状态查询接口 

    // 获取当前等级 (用于数值计算)
    int GetLevel() const { return level_; }

    /**
     * @brief 检查是否正在建造/升级中
     * 如果为 true，该建筑通常失效 (不攻击、不产出、不提供容量)
     */
    bool IsConstructing() const { return current_state_ == Core::BuildingAnimationState::kConstructing; }

    /**
     * @brief 开始建造或升级
     * @param duration 建造所需时间 (秒)
     */
    void StartConstruction(float duration);

    // 核心战斗接口 (Combat Interface)

    /**
     * @brief 判断是否能攻击目标
     * @param target_type 目标的类型 (地/空)
     */
    bool CanAttack(Core::GeneralType target_type) const {
        // stats_.target_type_ 建筑的属性
        // target_type 敌人的属性 
        return (static_cast<unsigned int>(stats_.target_type_) &
            static_cast<unsigned int>(target_type)) != 0;
    }

    // 获取单次伤害
    int GetDamage() const { return stats_.damage_; }

    // 获取攻击范围 (像素单位)
    float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }

    // 经济与功能接口 (Economy & Utility)

    // 获取当前存储的资源量 (针对金矿/收集器)
    int GetStoredResource() const { return static_cast<int>(stored_resource_); }

    /**
     * @brief 收集资源
     * 玩家点击时调用，清空暂存并返回数量
     * @param max_amount 此次收集的最大限额 (-1表示不限)
     * @return int 实际收集到的资源量
     */
    int CollectResource(int max_amount = -1);

    /**
     * @brief 获取占地矩形
     * 用途：在建造时，判断这个矩形内是否已经有别的建筑了；寻路时判断边缘
     */
    cocos2d::Rect GetOccupiedRect() const;

    // 获取建筑占地尺寸 (用于网格碰撞逻辑)
    // 如 3 表示 3x3 格子
    int GetWidthInTiles() const { return stats_.width_; }
    int GetHeightInTiles() const { return stats_.height_; }

    // 获取建筑类型
    Core::BuildingType GetBuildingType() const { return type_; }

    // 获取当前状态
    Core::BuildingAnimationState GetState() const { return current_state_; }

private:
    // [数据] 从 GameConfig 读取的静态属性
    Core::BuildingStats stats_;
    Core::BuildingType type_;

    // [外观] 建筑贴图 (例如 "Cannon.png")
    cocos2d::Sprite* visual_sprite_ = nullptr;

    // 状态数据
    int level_;                 // 当前等级
    float construction_timer_;  // 建造倒计时

    // [Refactor] 使用全局状态枚举替代布尔标记
    Core::BuildingAnimationState current_state_ = Core::BuildingAnimationState::kIdle;

    // [经济状态]
    // 当前累积的未收集资源 (如果是金矿的话)
    float stored_resource_ = 0.0f;

    // [新增] 防止重复注册障碍物的标记
    bool obstacle_registered_ = false;

    // 辅助函数

    // 根据建筑类型获取图片文件名
    std::string GetSpriteFilename(Core::BuildingType type);

    // 战斗逻辑，使用组件
    void UpdateCombatLogic(float dt);

    // 辅助函数：将建筑类型转换为投射物类型
    Core::ProjectileType GetProjectileTypeFromBuilding(Core::BuildingType type);
    // 生产资源 (矿机逻辑)
    void ProduceResource(float dt);

    // 切换状态
    void SetState(Core::BuildingAnimationState new_state);
};
#endif // GAMEPLAY_ENTITIES_BUILDING_H_