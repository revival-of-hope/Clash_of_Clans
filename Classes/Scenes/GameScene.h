#ifndef CLASH_OF_CLANS_LOCAL_CLASSES_SCENES_GAMESCENE_H_
#define CLASH_OF_CLANS_LOCAL_CLASSES_SCENES_GAMESCENE_H_

#include "Classes/Contract/Gameplay/GameEvents.h"
#include "cocos2d.h"

class GameScene : public cocos2d::Scene, public Gameplay::IGameEventListener {
public:
    static cocos2d::Scene* createScene();
    bool init() override;
    void onExit() override;

    CREATE_FUNC(GameScene);

    void OnBattleEnded(const Gameplay::BattleEndEvent& evt) override;

private:
    cocos2d::Label* status_label_ = nullptr;
};

#endif  // CLASH_OF_CLANS_LOCAL_CLASSES_SCENES_GAMESCENE_H_
