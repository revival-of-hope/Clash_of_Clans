#include "EngineTestScene.h"

/*
#include "Engine/Logic/EngineTestScene.h"
在appdelegate里加一行后用
auto scene = EngineTestScene::createScene();
Director::getInstance()->runWithScene(scene);
启动测试样例
*/
USING_NS_CC;
Scene* EngineTestScene::createScene()
{
    return EngineTestScene::create();
}
bool EngineTestScene::init()
{
    if (!Scene::init())
        return false;

    // 创建地图
    mapLayer = MapLayer::create("maps/test2.tmx");
    this->addChild(mapLayer);

    // Tile 高亮（可视化鼠标所指瓦片）
    tileHighlighter = new TileHighlighter(mapLayer->getMap());
    tileHighlighter->enable();
    

    //鼠标控制（拖拽地图）
    mouseController = new MouseController(mapLayer);
    mouseController->enable();

    // 放置控制器
    tilePlacement = new TilePlacementController(this);

    //菜单图标（触发放置）
    auto menuIcon = Sprite::create("HelloWorld.png");
    menuIcon->setScale(0.4f);
    menuIcon->setPosition(Vec2(80, 80));
    this->addChild(menuIcon, 10);

    tilePlacement->bindMenuIcon(
        menuIcon,
        mapLayer,
        "Troops_Icon/Archer.png"
    );

    return true;
}

