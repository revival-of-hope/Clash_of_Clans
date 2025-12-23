#ifndef ENGINE_TEST_SCENE_H
#define ENGINE_TEST_SCENE_H

#include "cocos2d.h"

// Gameplay
#include "Gameplay/Public/Unit.h"
#include "Gameplay/Public/Building.h"
#include "Gameplay/Public/CombatResolver.h"

#include "Core/GameConstants.h"

// Engine 模组
#include "Engine/Public/MapLayer.h"
#include "Engine/Public/TilePlacementController.h"
#include "Engine/Public/MouseController.h"


class EngineTestScene : public cocos2d::Scene
{
public:
    CREATE_FUNC(EngineTestScene);
    static cocos2d::Scene* createScene();
    virtual bool init() override;

private:
    // === 所有权：EngineTestScene ===
    MapLayer* mapLayer = nullptr;

    TilePlacementController* tilePlacement = nullptr;
    TileHighlighter* tileHighlighter = nullptr;
    MouseController* mouseController = nullptr;
};

#endif // ENGINE_TEST_SCENE_H
