// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Base class for all game entities (Units, Buildings).
// Inherits from cocos2d::Node to support rendering and hierarchy.

#ifndef GAMEPLAY_ENTITIES_BASE_ENTITY_H_
#define GAMEPLAY_ENTITIES_BASE_ENTITY_H_

#include "cocos2d.h"
#include "Core/GameConstants.h"

/**
 * @brief 游戏实体的基类
 * 所有场景中的对象（建筑、单位）都应继承此类。
 * 提供了 ID 管理、阵营管理和生命周期标记。
 */
class BaseEntity : public cocos2d::Node {
public:
    BaseEntity();
    virtual ~BaseEntity();

    // 核心初始化
    virtual bool init() override;

    /**
     * @brief 每一帧的逻辑更新
     * override 表示：我要覆盖 cocos2d::Node 里的 update(float dt)
     * @param dt 上一帧到当前帧的时间间隔 (秒)
     */
    virtual void update(float dt) override;

    // 唯一实例 ID
    void set_instance_id(int id) { instance_id_ = id; }
    int get_instance_id() const { return instance_id_; }

    void set_owner_id(int id) { owner_id_ = id; }
    int get_owner_id() const { return owner_id_; }

    /**
     * @brief 设置阵营 (用于区分敌我)
     * @param camp 阵营枚举 (kPlayer, kEnemy)
     */
    void set_camp(Core::CampType camp) { camp_ = camp; }
    Core::CampType get_camp() const { return camp_; }

    /**
     * @brief 标记为待销毁
     * 实体不会立即消失，而是在下一帧 update 中安全移除，防止迭代器失效。
     */
    void MarkForDestruction() { is_marked_for_destruction_ = true; }

    /**
     * @brief 检查是否已被标记销毁
     */
    bool IsMarkedForDestruction() const { return is_marked_for_destruction_; }

    /**
     * @brief 获取中心点世界坐标
     * 方便计算距离，对齐判定
     */
    cocos2d::Vec2 GetCenterPosition() const;

protected:
    int instance_id_;                 ///< 唯一 ID
    Core::CampType camp_;             ///< 阵营
    bool is_marked_for_destruction_;  ///< 销毁标记
    int owner_id_;                    ///< 所有者 ID
};

#endif // GAMEPLAY_ENTITIES_BASE_ENTITY_H_