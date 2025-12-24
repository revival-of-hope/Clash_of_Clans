// Source: Classes/Integration/Docs/Gameplay Module API Reference.md.md (Building)
#ifndef CONTRACTS_GAMEPLAY_BUILDING_H_
#define CONTRACTS_GAMEPLAY_BUILDING_H_

#include "Core/GameConstants.h"
#include "Contracts/Gameplay/BaseEntity.h"

class Building : public BaseEntity {
public:
    static Building* create(Core::BuildingType type, int level, int owner_id);

    void StartConstruction(float duration);

    int CollectResource(int max);

    cocos2d::Rect GetOccupiedRect() const;

    int GetLevel() const;

    Core::BuildingType GetBuildingType() const;

    int GetStoredResource() const;

    bool IsConstructing() const;

private:
    Building(Core::BuildingType type, int level, int owner_id);

    Core::BuildingType type_;
    int level_ = 0;
    int owner_id_ = 0;
    bool is_constructing_ = false;
    float construction_time_remaining_ = 0.0f;
};

#endif  // CONTRACTS_GAMEPLAY_BUILDING_H_
