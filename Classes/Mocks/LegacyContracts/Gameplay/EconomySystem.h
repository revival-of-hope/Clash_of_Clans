// Source: Classes/Integration/Docs/Gameplay Module API Reference.md.md (EconomySystem)
#ifndef Contract_GAMEPLAY_ECONOMYSYSTEM_H_
#define Contract_GAMEPLAY_ECONOMYSYSTEM_H_

namespace cocos2d {
template <class T>
class Vector;
}

class Building;

class EconomySystem {
public:
    static EconomySystem* GetInstance();

    void Reset();

    bool SpendGold(int amount);
    bool SpendElixir(int amount);

    void AddGold(int amount);
    void AddElixir(int amount);

    int TryCollectResource(Building* building);

    void RecalculateLimits(const cocos2d::Vector<Building*>& buildings);

    bool AddTroopPopulation(int space);

    int GetCurrentGold() const;
    int GetMaxGold() const;

    int GetCurrentElixir() const;
    int GetMaxElixir() const;

    int GetCurrentPopulation() const;
    int GetMaxPopulation() const;

private:
    EconomySystem() = default;

    int current_gold_ = 0;
    int max_gold_ = 0;
    int current_elixir_ = 0;
    int max_elixir_ = 0;
    int current_population_ = 0;
    int max_population_ = 0;
};

#endif  // Contract_GAMEPLAY_ECONOMYSYSTEM_H_
