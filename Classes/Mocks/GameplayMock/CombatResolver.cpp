#include "Classes/Contract/Gameplay/CombatResolver.h"

CombatResolver* CombatResolver::GetInstance() {
    static CombatResolver instance;
    return &instance;
}

void CombatResolver::Initialize(cocos2d::Node* layer) {
    layer_ = layer;
}

void CombatResolver::ResolveMeleeAttack(cocos2d::Node* /*attacker*/, cocos2d::Node* /*target*/, int /*dmg*/) {}

void CombatResolver::SpawnProjectile(cocos2d::Node* /*attacker*/,
                                     cocos2d::Node* /*target*/,
                                     int /*dmg*/,
                                     Core::ProjectileType /*type*/) {}
