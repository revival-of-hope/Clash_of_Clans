// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Component to manage Health and display a Health Bar.
//
// Path: Classes/Contract/Gameplay/HealthComp.h

#ifndef CONTRACT_GAMEPLAY_HEALTH_COMP_H_
#define CONTRACT_GAMEPLAY_HEALTH_COMP_H_

#include "cocos2d.h"

/**
 * @brief Health Component
 * Mounted on Unit or Building, responsible for managing health, judging death, and drawing simple health bar.
 */
class HealthComp : public cocos2d::Node {
public:
    CREATE_FUNC(HealthComp);

    bool init() override;

    /**
     * @brief Initialize stats
     * @param max_hp Maximum Health
     */
    void InitStats(int max_hp);

    /**
     * @brief Take Damage
     * @param amount Damage amount
     * @return bool If this damage caused death, return true
     */
    bool TakeDamage(int amount);

    /**
     * @brief Heal
     * @param amount Heal amount
     */
    void Heal(int amount);

    // State Query
    bool IsDead() const { return is_dead_; }
    float GetHealthPercentage() const;
    float GetCurrentHealth() const { return current_hp_; }
    float GetMaxHealth() const { return max_hp_; }

    /**
     * @brief Set Health Bar Position
     * @param offset Offset relative to parent node center
     */
    void SetHealthBarOffset(cocos2d::Vec2 offset);

protected:
    void UpdateHealthBar();

private:
    float current_hp_ = 100.0f;
    float max_hp_ = 100.0f;
    bool is_dead_ = false;

    cocos2d::DrawNode* health_bar_node_ = nullptr;
    cocos2d::Vec2 bar_offset_;
};

#endif // CONTRACT_GAMEPLAY_HEALTH_COMP_H_