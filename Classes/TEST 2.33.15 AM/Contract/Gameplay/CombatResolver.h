#ifndef Contract_GAMEPLAY_COMBATRESOLVER_H_
#define Contract_GAMEPLAY_COMBATRESOLVER_H_

#include "Core/GameConstants.h"

namespace cocos2d {
class Node;
}

class CombatResolver {
public:
    static CombatResolver* GetInstance();

    void Initialize(cocos2d::Node* layer);

    void ResolveMeleeAttack(cocos2d::Node* attacker, cocos2d::Node* target, int dmg);

    void SpawnProjectile(cocos2d::Node* attacker,
                         cocos2d::Node* target,
                         int dmg,
                         Core::ProjectileType type);

private:
    CombatResolver() = default;

    cocos2d::Node* layer_ = nullptr;
};

#endif  // Contract_GAMEPLAY_COMBATRESOLVER_H_
