// Source: Classes/Integration/Docs/Gameplay Module API Reference.md.md (HealthComp)
#ifndef Contract_GAMEPLAY_HEALTHCOMP_H_
#define Contract_GAMEPLAY_HEALTHCOMP_H_

#include "cocos2d.h"

class HealthComp {
public:
    float GetHealthPercentage() const;

    int GetCurrentHP() const;

    int GetMaxHP() const;

    bool IsDead() const;

    void SetHealthBarOffset(cocos2d::Vec2 offset);

    bool TakeDamage(int amount);

    void InitStats(int max_hp);
};

#endif  // Contract_GAMEPLAY_HEALTHCOMP_H_
