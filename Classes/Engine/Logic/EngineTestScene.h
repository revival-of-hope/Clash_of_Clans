#ifndef ENGINE_TEST_SCENE_H
#define ENGINE_TEST_SCENE_H

#include "cocos2d.h"

// Gameplay
#include "Gameplay/Entities/Unit.h"
#include "Gameplay/Entities/Building.h"
#include "Gameplay/Logic/CombatResolver.h"

#include "Core/GameConstants.h"

// Engine 模组
#include "Engine/Public/MapLayer.h"
#include "Engine/Public/TilePlacementController.h"
#include "Engine/Public/MouseController.h"
#include "Engine/Public/TileHighlighter.h"

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
