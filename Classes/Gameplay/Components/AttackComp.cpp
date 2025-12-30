// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of AttackComp.

#include "AttackComp.h"
#include "Contract/Gameplay/HealthComp.h"
#include "Contract/Gameplay/Building.h"
#include "Core/GameConstants.h"
#include "Contract/Gameplay/CombatResolver.h"

bool AttackComp::init() {
    if (!cocos2d::Node::init()) {
        return false;
    }

    // ���� Update �Ա��Զ�������ȴʱ��
    this->scheduleUpdate();

    damage_ = 0;
    range_sq_ = 0.0f;
    attack_speed_ = 1.0f;
    attack_timer_ = 0.0f;
    projectile_type_ = Core::ProjectileType::kNone; // Ĭ�Ͻ�ս

    return true;
}

void AttackComp::InitStats(int damage, float range_pixels, float attack_speed, Core::ProjectileType projectile_type) {
    damage_ = damage;
    range_sq_ = range_pixels * range_pixels;
    attack_speed_ = attack_speed;
    projectile_type_ = projectile_type;

    attack_timer_ = 0.0f;
}


void AttackComp::update(float dt) {
    // ��ȴ����ʱ
    if (attack_timer_ > 0.0f) {
        attack_timer_ -= dt;
        if (attack_timer_ < 0.0f) {
            attack_timer_ = 0.0f;
        }
    }
}

bool AttackComp::IsTargetInRange(BaseEntity* target) const {
    if (!target) return false;

    // 1. ��ȡ��������
    // getParent() �ǹ����������Ĺ����� (Unit)
    auto attacker = this->getParent();
    if (!attacker) return false;

    cocos2d::Vec2 my_pos = attacker->getPosition();
    cocos2d::Vec2 target_pos = target->getPosition();

    // ���Խ�Ŀ��ת��Ϊ����
    auto target_building = dynamic_cast<Building*>(target);
    if (target_building) {
        // --- ��Խ����ı�Ե��� (Edge-to-Edge) ---
        // ����������ĵ�����Զ������
        cocos2d::Rect rect = target_building->GetOccupiedRect();
        float dx = 0.0f;
        float dy = 0.0f;

        // ���� Unit ���ĵ��������ıߵľ���
        if (my_pos.x < rect.getMinX()) dx = rect.getMinX() - my_pos.x;
        else if (my_pos.x > rect.getMaxX()) dx = my_pos.x - rect.getMaxX();

        if (my_pos.y < rect.getMinY()) dy = rect.getMinY() - my_pos.y;
        else if (my_pos.y > rect.getMaxY()) dy = my_pos.y - rect.getMaxY();

        float dist_to_edge_sq = dx * dx + dy * dy;

        return dist_to_edge_sq <= range_sq_;
    }
    else {
        // --- ��Ե�λ������/�뾶��� ---
        float target_radius = 20.0f;
        float dist_sq = my_pos.getDistanceSq(target_pos);

        // ��Ч��� = (�������� + Ŀ��뾶)^2
        float range_val = std::sqrt(range_sq_);
        float effective_range = range_val + target_radius;

        return dist_sq <= effective_range * effective_range;
    }
}

bool AttackComp::TryAttack(BaseEntity* target) {
    // 1. ����У��
    if (!target) return false;
    if (target->IsMarkedForDestruction()) return false; // Ŀ���Ѿ�����

    // 2. ��ȴ���
    if (attack_timer_ > 0.0f) {
        return false; // ���ܻ�����ȴ��
    }

    // 3. ��Χ���
    if (!IsTargetInRange(target)) {
        return false; // ������
    }

    // ������ȴ
    attack_timer_ = attack_speed_;

    // ����˺�
    DealDamage(target);

    // ���� true��֪ͨ�ⲿ(Unit)���Ź�������
    return true;
}

void AttackComp::DealDamage(BaseEntity* target) {
    if (!target) return;

    // ��ȡ���������� (������������ Unit)
    auto attacker = dynamic_cast<BaseEntity*>(this->getParent());
    if (!attacker) return;

    // [����ת���߼�]
    // ���� projectile_type_ ����������·

    if (projectile_type_ == Core::ProjectileType::kNone) {
        // ��֧ A: ��ս / ˲���˺�
        // ������: Ұ���� (kNone), ���� (kNone), ը���� (kNone)
        // ע��: ը���˵��Ա��߼����� ResolveMeleeAttack �ﴦ���ģ����Ա���������
        CombatResolver::GetInstance()->ResolveMeleeAttack(attacker, target, damage_);
    }
    else {
        // ��֧ B: Զ�� / Ͷ����
        // ������: ������ (kArrow), ���� (kFireBall)
        // �������һ�����е� Sprite�������ɵ�Ŀ������ʱ��CombatResolver ���Զ����� ApplyDamage
        CombatResolver::GetInstance()->SpawnProjectile(attacker, target, damage_, projectile_type_);
    }
}