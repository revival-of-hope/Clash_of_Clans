// GameConfig.h
// Singleton Database for balancing game statistics.
// Hardcoded (instead of using individual classes) for Compilation Safety, Speed, and the "Flyweight" concept.

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H
#include "GameStructs.h"
#include <map>

namespace Core {
class GameConfig {
public:
    // Singleton access
    static GameConfig* GetInstance();

    // The Lookup Functions
    UnitStats GetTroopStats(TroopType type, int level);

    BuildingStats GetBuildingStats(BuildingType type, int level);
    
    // Resource Costs(inline)
    int GetUpgradeCost(BuildingType type, int level) {
        // Return gold/elixir cost
        return 500 * level; 
    }
};
} // namespace Core

#endif // GAME_CONFIG_H