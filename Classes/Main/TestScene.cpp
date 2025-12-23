#include "TestScene.h"
#include "Contract/Gameplay/Unit.h"
#include "Contract/Gameplay/Building.h"
#include "Contract/Gameplay/CombatResolver.h"
#include "Core/GameConstants.h"

#include <cmath> 
#include <vector>

USING_NS_CC;

Scene* TEST::createScene()
{
    return TEST::create();
}

bool TEST::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 centerPos = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

    // [修改] 开启战斗逻辑开关
    bool enablebattle = true;

    if (enablebattle) {
        // 2. 初始化战斗系统
        CombatResolver::GetInstance()->Initialize(this);

        // 3. UI 提示
        auto label = Label::createWithTTF("Scenario: Dragon's Keep\nDefender: Walls + Cannons + AIR UNITS!\nAttacker: Combined Arms", "fonts/Marker Felt.ttf", 28);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 60));
        this->addChild(label, 100);

        auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
            CC_CALLBACK_1(TEST::menuCloseCallback, this));
        closeItem->setPosition(Vec2(origin.x + visibleSize.width - 50, origin.y + 50));
        auto menu = Menu::create(closeItem, NULL);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 100);

        // =========================================================================
        //  防守方 (Defender - ID 1)
        //  布局：坚固的城墙 + 内部火力网 + 空中支援
        // =========================================================================
        cocos2d::log("--- Spawning Defenders ---");

        // [核心] 大本营
        auto townHall = Building::create(Core::BuildingType::kTownHall, 1, 1);
        townHall->setPosition(centerPos);
        this->addChild(townHall);

        // [防御] 4个加农炮，镇守四角
        std::vector<Vec2> towerOffsets = {
            Vec2(-200, 150), Vec2(200, 150),
            Vec2(-200, -150), Vec2(200, -150)
        };
        for (auto offset : towerOffsets) {
            auto cannon = Building::create(Core::BuildingType::kCannon, 1, 1);
            cannon->setPosition(centerPos + offset);
            this->addChild(cannon);
        }

        // [城墙] 构建一个巨大的封闭矩形
        float wallW = 400.0f; // 宽
        float wallH = 300.0f; // 高
        float step = 64.0f;   // 瓦片尺寸

        auto createWall = [&](float x, float y) {
            auto wall = Building::create(Core::BuildingType::kWall, 1, 1);
            wall->setPosition(Vec2(x, y));
            this->addChild(wall);
            };

        // 上下墙
        for (float x = centerPos.x - wallW; x <= centerPos.x + wallW; x += step) {
            createWall(x, centerPos.y + wallH);
            createWall(x, centerPos.y - wallH);
        }
        // 左右墙
        for (float y = centerPos.y - wallH + step; y < centerPos.y + wallH; y += step) {
            createWall(centerPos.x - wallW, y);
            createWall(centerPos.x + wallW, y);
        }

        // [防守方部队 - 空中] 2只 飞龙宝宝 (Baby Dragon)
        // 这是测试关键：进攻方的近战单位（野蛮人/巨人）应该无视它们，因为打不到
        for (int i = 0; i < 2; ++i) {
            auto dragon = Unit::create(Core::TroopType::kBabyDragon, 1, 1); // ID 1 = Defender
            // 它们盘旋在大本营上方
            dragon->setPosition(centerPos + Vec2((i == 0 ? -100 : 100), 200));
            this->addChild(dragon);
        }

        // [防守方部队 - 地面] 5个 弓箭手 (Archer)
        // 站在城墙内侧，测试隔墙射击
        for (int i = 0; i < 5; ++i) {
            auto defArch = Unit::create(Core::TroopType::kArcher, 1, 1); // ID 1
            defArch->setPosition(centerPos + Vec2(-100 + i * 50, -50));
            this->addChild(defArch);
        }

        // =========================================================================
        // 进攻方 (Attacker - ID 0)
        // 多兵种协同作战
        // =========================================================================
        cocos2d::log("--- Spawning Attackers ---");

        Vec2 spawnBase = centerPos + Vec2(-600, -500); // 左下角集结

        // 1. [先锋] 炸弹人 (Wall Breakers) x5
        // 任务：炸开城墙
        for (int i = 0; i < 15; ++i) {
            auto wb = Unit::create(Core::TroopType::kWallBreaker, 1, 0);
            wb->setPosition(spawnBase + Vec2(100 + i * 40, 100));
            this->addChild(wb);
        }

        // 2. [肉盾] 巨人 (Giants) x4
        // 任务：吸引加农炮火力。注意：它们打不到飞龙。
        for (int i = 0; i < 4; ++i) {
            auto giant = Unit::create(Core::TroopType::kGiant, 1, 0);
            giant->setPosition(spawnBase + Vec2(i * 80, 0));
            this->addChild(giant);
        }

        // 3. [防空主力] 弓箭手 (Archers) x15
        // 任务：只有她们能处理防守方的飞龙宝宝！
        // 如果这些弓箭手死了，进攻方就输定了（会被飞龙白嫖）。
        for (int i = 0; i < 20; ++i) {
            auto arch = Unit::create(Core::TroopType::kArcher, 1, 0);
            float row = (i / 5) * 40;
            float col = (i % 5) * 40;
            arch->setPosition(spawnBase + Vec2(col - 100, row - 100)); // 站在后排
            this->addChild(arch);
        }

        // 4. [杂兵] 野蛮人 (Barbarians) x10
        // 任务：一旦墙破了，冲进去拆建筑
        for (int i = 0; i < 50; ++i) {
            auto barb = Unit::create(Core::TroopType::kBarbarian, 1, 0);
            barb->setPosition(spawnBase + Vec2(i * 30 + 200, -50));
            this->addChild(barb);
        }

    }
    return true;
}

void TEST::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}