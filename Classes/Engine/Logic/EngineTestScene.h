#ifndef ENGINE_TEST_SCENE_H
#define ENGINE_TEST_SCENE_H

#include "cocos2d.h"

// Gameplay
#include "Contract/Gameplay/Unit.h"
#include "Contract/Gameplay/Building.h"
#include "Contract/Gameplay/CombatResolver.h"

#include "Core/GameConstants.h"

// Engine 模组
#include "Contract/Engine/MapLayer.h"
#include "Contract/Engine/TilePlacementController.h"
#include "Contract/Engine/MouseController.h"


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
