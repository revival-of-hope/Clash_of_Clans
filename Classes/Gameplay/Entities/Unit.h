// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Unit class representing movable troops.
// Manages State Machine (Idle, Move, Attack) and Components.

#ifndef GAMEPLAY_ENTITIES_UNIT_H_
#define GAMEPLAY_ENTITIES_UNIT_H_

#include "BaseEntity.h"
#include "Core/GameConfig.h"

/**
 * @brief 兵种单位类
 * 继承自 BaseEntity，拥有移动、攻击和状态机逻辑。
 */
class Unit : public BaseEntity {
public:
	/**
	 * @brief 静态工厂方法，创建 Unit 实例
	 * @param type 兵种类型
	 * @param level 等级
	 * @param owner_id 所有者 ID
	 * @return Unit* 创建成功的对象，autorelease
	 */
	static Unit* create(Core::TroopType type, int level, int owner_id);

	virtual bool init(Core::TroopType type, int level, int owner_id);
	virtual void update(float dt) override;

	Core::GeneralType GetGeneralType() const { return stats_.unit_type_; }

	// 公开兵种类型，供 CombatResolver 识别是否为炸弹人
	Core::TroopType GetTroopType() const { return type_; }

	// 获取单次攻击伤害
	int GetDamage() const { return stats_.damage_per_shot_; }

	// 获取攻击范围 (像素单位)
	float GetRangeInPixels() const { return stats_.range_ * Core::kTileWidth; }

	/**
	 * @brief 核心接口：判断我能否攻击某种类型的目标
	 * @param target_type 目标的 GeneralType (Ground/Air)
	 * @return true 如果可以攻击，false 则不能
	 */
	bool CanAttack(Core::GeneralType target_type) const;

	// 自定义状态机枚举
	enum class State {
		kIdle,   // 待机
		kMove,   // 移动
		kAttack, // 攻击
		kDead    // 死亡
	};

	/**
	 * @brief 切换单位状态
	 * 会处理状态切换时的逻辑，如播放动画、停止寻路等
	 * @param new_state 新状态
	 */
	void SetState(State new_state);

	// 获取当前状态
	State GetState() const { return current_state_; }

private:
	Core::UnitStats stats_;                    ///< 核心数值
	Core::TroopType type_;                     ///< 兵种标记
	cocos2d::Sprite* visual_sprite_ = nullptr; ///< 外观精灵

	State current_state_ = State::kIdle;       ///< 当前状态

	/**
	 * @brief 辅助函数：根据兵种类型返回图片文件名
	 */
	std::string GetSpriteFilename(Core::TroopType type);
};
#endif // GAMEPLAY_ENTITIES_UNIT_H_