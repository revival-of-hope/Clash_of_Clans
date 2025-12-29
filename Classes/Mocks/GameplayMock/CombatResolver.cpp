#include "Classes/Contract/Gameplay/CombatResolver.h"

#include <string>

CombatResolver* CombatResolver::GetInstance() {
    static CombatResolver instance;
    return &instance;
}

void CombatResolver::Initialize(cocos2d::Node* battle_layer) {
    battle_layer_ = battle_layer;
    projectiles_.clear();
}

void CombatResolver::Update(float dt) {
    (void)dt;
    projectiles_.clear();
}

void CombatResolver::ResolveMeleeAttack(BaseEntity* /*attacker*/, BaseEntity* /*target*/, int /*damage*/) {
    // Mock does not mutate targets.
}

void CombatResolver::SpawnProjectile(BaseEntity* attacker, BaseEntity* target, int damage, Core::ProjectileType type) {
    ProjectileObject projectile{};
    projectile.id = static_cast<int>(projectiles_.size()) + 1;
    projectile.target = target;
    projectile.damage = damage;
    projectile.speed = GetProjectileSpeed(type);
    projectile.type = type;
    if (attacker) {
        projectile.last_known_pos = attacker->getPosition();
    }
    projectile.target_lost = target == nullptr;
    projectiles_.push_back(projectile);
}

std::string CombatResolver::GetProjectileFilename(Core::ProjectileType type) {
    switch (type) {
        case Core::ProjectileType::kFireBall:
            return "fireball.png";
        case Core::ProjectileType::kCannonBall:
            return "cannon_ball.png";
        default:
            return "projectile.png";
    }
}

float CombatResolver::GetProjectileSpeed(Core::ProjectileType type) {
    switch (type) {
        case Core::ProjectileType::kFireBall:
            return 2.5f;
        case Core::ProjectileType::kCannonBall:
            return 2.0f;
        default:
            return 1.0f;
    }
}

void CombatResolver::PlayImpactVFX(const cocos2d::Vec2& /*pos*/, Core::ProjectileType /*type*/) {
    // Visual effects are not simulated in the mock.
}

void CombatResolver::ApplyDamage(BaseEntity* /*target*/, int /*damage*/) {
    // Damage resolution is not simulated in the mock.
}

