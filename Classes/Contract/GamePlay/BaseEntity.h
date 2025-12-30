// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Base class for all game entities (Units, Buildings).
// Inherits from cocos2d::Node to support rendering and hierarchy.
//
// Path: Classes/Contract/GamePlay/BaseEntity.h

#ifndef CONTRACT_GAMEPLAY_BASE_ENTITY_H_
#define CONTRACT_GAMEPLAY_BASE_ENTITY_H_

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

    // 生命周期管理，用于维护全局注册表
    virtual void onEnter() override;
    virtual void onExit() override;

    /**
     * @brief 每一帧的逻辑更新
     * @param dt 上一帧到当前帧的时间间隔 (秒)
     */
    virtual void update(float dt) override;

    // ==========================================================================
    // ID 管理
    // ==========================================================================
    
    void set_instance_id(int id) { instance_id_ = id; }
    int get_instance_id() const { return instance_id_; }

    void set_owner_id(int id) { owner_id_ = id; }
    int get_owner_id() const { return owner_id_; }

    // ==========================================================================
    // 阵营判断
    // ==========================================================================
    
    /**
     * @brief 判断目标是否为盟友
     * @param other 另一个实体
     * @return true 如果是同一阵营 (ID相同)
     */
    bool IsAlly(const BaseEntity* other) const {
        if (!other) return false;
        return this->owner_id_ == other->get_owner_id();
    }

    // ==========================================================================
    // 生命周期管理
    // ==========================================================================
    
    /**
     * @brief 标记为待销毁
     * 实体不会立即消失，而是在下一帧 update 中安全移除
     */
    void MarkForDestruction() { is_marked_for_destruction_ = true; }

    /**
     * @brief 检查是否已被标记销毁
     */
    bool IsMarkedForDestruction() const { return is_marked_for_destruction_; }

    /**
     * @brief 获取中心点世界坐标
     */
    cocos2d::Vec2 GetCenterPosition() const;

    // ==========================================================================
    // 全局实体注册表
    // ==========================================================================
    
    /**
     * @brief 获取当前场景中所有活跃的 BaseEntity
     */
    static cocos2d::Vector<BaseEntity*>& GetAllEntities() { return global_entities_; }

protected:
    int instance_id_ = -1;            ///< 唯一 ID
    int owner_id_ = 0;                ///< 所有者 ID (0=玩家, 1=敌方)
    bool is_marked_for_destruction_ = false;  ///< 销毁标记

private:
    static cocos2d::Vector<BaseEntity*> global_entities_;
};

#endif // CONTRACT_GAMEPLAY_BASE_ENTITY_H_
