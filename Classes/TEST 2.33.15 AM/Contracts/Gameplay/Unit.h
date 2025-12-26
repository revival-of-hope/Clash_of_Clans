// Source: Classes/Integration/Docs/Gameplay Module API Reference.md.md (Unit)
#ifndef CONTRACTS_GAMEPLAY_UNIT_H_
#define CONTRACTS_GAMEPLAY_UNIT_H_

#include "Core/GameConstants.h"
#include "Contracts/Gameplay/BaseEntity.h"

class Unit : public BaseEntity {
public:
    static Unit* create(Core::TroopType type, int level, int owner_id);

    Core::TroopType GetTroopType() const;

    Core::GeneralType GetGeneralType() const;

    Core::UnitAnimationState GetState() const;

    int GetLevel() const;

    int GetDamage() const;

    cocos2d::Rect GetOccupiedRect() const;

    bool CanAttack(Core::GeneralType target_type) const;

    void SetFacing(Core::Facing facing);

private:
    Unit(Core::TroopType type, int level, int owner_id);

    Core::TroopType type_;
    int level_ = 0;
    int owner_id_ = 0;
    Core::Facing facing_ = Core::Facing::kDown;
};

#endif  // CONTRACTS_GAMEPLAY_UNIT_H_
