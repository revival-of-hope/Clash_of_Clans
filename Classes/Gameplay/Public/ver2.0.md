# **Gameplay 模组开发手册与行为契约 (V2.0)**

版本: 2.0 (Release Candidate)  
负责人: Dev B (Gameplay)  
适用方: Dev C (UI/App), Dev A (Engine)  
最后更新: 2025-12-19

## **1\. 物理架构与公共头文件 (Public Headers)**

为了实现模块间的“物理隔离”，Gameplay 模块的所有实现细节（.cpp）对外隐藏。外部模块（UI、Engine）**仅允许** 引用 Classes/Public/Gameplay 目录下的头文件。

**引用路径规范：**

// ✅ 正确：引用公共接口  
\#include "Gameplay/Public/Unit.h"  
\#include "Gameplay/Public/EconomySystem.h"

// ❌ 错误：直接引用私有实现 (严禁跨模块引用 cpp，会导致链接错误)  
\#include "../../Gameplay/Entities/Unit.cpp" 



## **2\. 行为契约 (Behavior Contracts)**

调用 Gameplay 模块接口时，**必须**遵守以下契约。这是保证游戏逻辑正确、内存安全的前提。

### **2.1 坐标系契约 (Coordinate System)**

游戏逻辑与渲染层混用两种坐标系，请务必区分：

1. **逻辑网格 (Grid Coordinate):**  
   * **类型:** int  
   * **范围:** 0 \<= x \< 40, 0 \<= y \< 40  
   * **原点:** 地图逻辑**左下角** (0,0)  
   * **用途:** Building 的建造位置、A\* 寻路算法、GameStructs 数据存储。  
2. **世界像素 (Pixel Coordinate):**  
   * **类型:** float (Cocos2d 标准)  
   * **原点:** 屏幕/Layer 左下角 (0,0)  
   * **换算公式:**  
     * Pixel X \= Grid X \* 64 \+ 32 (中心点对齐)  
     * Pixel Y \= Grid Y \* 64 \+ 32  
   * **用途:** Unit 的移动、cocos2d::Node::setPosition、投射物飞行。

**⚠️ 约束:** 所有 Unit 和 Building 的锚点 (AnchorPoint) 均锁定为 (0.5, 0.5)。

### **2.2 生命周期与所有权契约 (Lifecycle & Ownership)**

* **创建 (Factory Pattern):**  
  * 禁止使用 new。必须使用静态工厂 create()。  
  * **返回值:** autorelease 对象。  
  * **义务:** 调用方（Scene/Layer）必须在创建后**立即**调用 addChild() 或 retain()，否则对象将在本帧结束时自动销毁。  
* **销毁 (Destruction):**  
  * **禁止 delete:** 严禁手动 delete 实体指针。  
  * **外部主动销毁:** 若 UI 需要移除实体（如取消建造），请调用 entity-\>MarkForDestruction() 或 entity-\>removeFromParent()。  
  * **内部被动销毁:** 当 HP \<= 0 时，Gameplay 内部会自动播放死亡动画并移除节点。  
  * **悬空指针警告:** UI 层切勿长期持有 Unit\* 指针。建议每一帧通过 IsMarkedForDestruction() 检查有效性。

### **2.3 时序与驱动契约 (Timing & Sequencing)**

* **初始化时序:**  
  * CombatResolver::Initialize(Layer\*) 必须在场景 init() 阶段调用，且早于任何单位生成。  
  * EconomySystem::RecalculateLimits(...) 必须在**每次**建筑建造/升级完成后调用。  
* **帧更新 (Update):**  
  * UI 层**不需要**手动驱动实体的 update。它们继承自 Node，在 addChild 后会自动由 Cocos 调度器驱动。  
  * CombatResolver 会自动接管 battle\_layer 的调度器。

### **2.4 事件与数据契约 (Events & Data)**

* **资源数据:** 是**只读**的。  
  * ❌ 禁止: economy-\>current\_gold\_ \+= 100;  
  * ✅ 允许: economy-\>AddGold(100); 或 economy-\>SpendGold(50);  
* **UI 刷新机制:**  
  * 目前采用**轮询 (Polling)** 模式。UI 层应在 update(dt) 中查询 EconomySystem 和 HealthComp 的状态来刷新界面。

## **3\. 核心 API 详解 (Substantial API Reference)**

### **3.1 EconomySystem (经济系统单例)**

**头文件:** Gameplay/Public/EconomySystem.h

| 接口方法 | 参数 | 说明 | 契约约束 |
| :---- | :---- | :---- | :---- |
| GetInstance() | 无 | 获取单例 | 全局唯一入口 |
| Reset() | 无 | 重置所有资源 | 新游戏开始时必须调用 |
| SpendGold() | int amount | 消费金币 | 若余额不足返回 false 且不扣款 |
| SpendElixir() | int amount | 消费圣水 | 若余额不足返回 false 且不扣款 |
| TryCollectResource() | Building\* | 收集资源 | 自动计算仓库剩余空间，**不会溢出**，返回实际收集量 |
| RecalculateLimits() | Vector\<Building\*\> | 重算上限 | **必须传入场景中所有建筑**。建造完成时必须调用此函数以更新人口/资源上限。 |
| AddTroopPopulation() | int space | 占用人口 | 造兵前必须检查返回值，true 表示人口足够 |

### **3.2 CombatResolver (战斗仲裁单例)**

**头文件:** Gameplay/Public/CombatResolver.h

| 接口方法 | 参数 | 说明 | 契约约束 |
| :---- | :---- | :---- | :---- |
| Initialize() | Node\* layer | 初始化 | **必须在 Scene::init 中调用**。传入的 layer 用于承载投射物。 |
| ResolveMeleeAttack() | attacker, target, dmg | 近战结算 | 瞬间造成伤害。特殊：炸弹人对墙造成 40 倍伤害并自杀。 |
| SpawnProjectile() | attacker, target, dmg, type | 远程攻击 | 生成投射物。伤害在投射物**命中时**结算。 |

### **3.3 Unit (兵种实体)**

**头文件:** Gameplay/Public/Unit.h

| 接口方法 | 参数 | 说明 |
| :---- | :---- | :---- |
| create(...) | TroopType, level, owner\_id | **静态工厂**。返回 Unit\*。owner\_id: 0=玩家, 1=敌人。 |
| GetTroopType() | 无 | 获取兵种枚举类型。 |
| CanAttack(...) | GeneralType target\_type | 查询是否能攻击目标（例如：野蛮人不能打 kAir）。 |
| SetFacing(...) | Facing | 设置朝向（影响 Sprite 翻转）。 |

### **3.4 Building (建筑实体)**

**头文件:** Gameplay/Public/Building.h

| 接口方法 | 参数 | 说明 |
| :---- | :---- | :---- |
| create(...) | BuildingType, level, owner\_id | **静态工厂**。返回 Building\*。 |
| StartConstruction(...) | float duration | 开始建造。期间 IsConstructing() 为 true，功能暂停。 |
| CollectResource(...) | int max | **底层接口**。UI 建议使用 EconomySystem::TryCollectResource。 |
| GetOccupiedRect() | 无 | 获取世界坐标下的占地矩形 (Pixels)。用于碰撞/点击检测。 |

## **4\. 标准集成示例 (Standard Integration Example)**

以下代码展示了如何在一个空白场景中正确集成 Gameplay 模块。该代码符合所有契约，并可直接编译。

**场景功能：**

1. 初始化战斗与经济系统。  
2. 放置大本营（确立逻辑网格）。  
3. 放置加农炮（演示建造流程）。  
4. 生成野蛮人（演示单位投放与战斗）。  
5. UI 轮询刷新资源。

### **4.1 头文件 (GameplayIntegrationScene.h)**

```cpp
\#ifndef INTEGRATION\_TEST\_SCENE\_H\_  
\#define INTEGRATION\_TEST\_SCENE\_H\_

\#include "cocos2d.h"

// \[契约 1\] 仅引用 Public 头文件  
\#include "Gameplay/Public/Unit.h"  
\#include "Gameplay/Public/Building.h"  
\#include "Gameplay/Public/EconomySystem.h"  
\#include "Gameplay/Public/CombatResolver.h"

class GameplayIntegrationScene : public cocos2d::Scene {  
public:  
    static cocos2d::Scene\* createScene();  
    virtual bool init() override;  
    virtual void update(float dt) override;  
      
    // 模拟用户操作  
    void OnPlaceTownHall();  
    void OnBuildCannon();  
    void OnSpawnBarbarian();

    CREATE\_FUNC(GameplayIntegrationScene);

private:  
    cocos2d::Node\* game\_layer\_ \= nullptr; // 游戏层 (ZOrder 0\)  
    cocos2d::Label\* resource\_label\_ \= nullptr; // UI 层 (ZOrder 100\)  
      
    // 简单的辅助函数：网格转像素  
    cocos2d::Vec2 GridToPixel(int x, int y) {  
        return cocos2d::Vec2(x \* 64 \+ 32, y \* 64 \+ 32);  
    }  
};

\#endif // INTEGRATION\_TEST\_SCENE\_H\_
```

### **4.2 实现文件 (GameplayIntegrationScene.cpp)**
```cpp
#include "GameplayIntegrationScene.h"

USING\_NS\_CC;

Scene\* GameplayIntegrationScene::createScene() {  
    return GameplayIntegrationScene::create();  
}

bool GameplayIntegrationScene::init() {  
    if (\!Scene::init()) return false;

    // 1\. 分层架构：逻辑层与UI层分离  
    game\_layer\_ \= Node::create();  
    this-\>addChild(game\_layer\_, 0);

    auto ui\_layer \= Node::create();  
    this-\>addChild(ui\_layer, 100);

    // 2\. \[契约 2.3\] 初始化核心系统  
    // 必须传入 game\_layer\_ 用于承载投射物  
    CombatResolver::GetInstance()-\>Initialize(game\_layer\_);  
    // 重置经济  
    EconomySystem::GetInstance()-\>Reset(); 

    // 3\. 搭建简单 UI  
    resource\_label\_ \= Label::createWithSystemFont("Gold: 0/0 | Pop: 0/0", "Arial", 24);  
    resource\_label\_-\>setPosition(Vec2(400, 500));  
    ui\_layer-\>addChild(resource\_label\_);

    // 4\. 执行模拟流程  
    OnPlaceTownHall();    // 放置大本营  
    OnBuildCannon();      // 建造加农炮  
    OnSpawnBarbarian();   // 投放野蛮人

    this-\>scheduleUpdate();  
    return true;  
}

void GameplayIntegrationScene::OnPlaceTownHall() {  
    // \[契约 2.2\] 创建建筑 (大本营, Lv1, 玩家所有)  
    auto townhall \= Building::create(Core::BuildingType::kTownHall, 1, 0);

    // \[契约 2.1\] 坐标设置 (逻辑网格 10,10)  
    townhall-\>setPosition(GridToPixel(10, 10));

    // \[契约 2.2\] 立即加入场景  
    game\_layer\_-\>addChild(townhall);  
      
    // \[契约 2.3\] 通知经济系统重算上限 (大本营提供资源容量)  
    cocos2d::Vector\<Building\*\> buildings;  
    buildings.pushBack(townhall);  
    EconomySystem::GetInstance()-\>RecalculateLimits(buildings);  
}

void GameplayIntegrationScene::OnBuildCannon() {  
    // 假设花费 200 金币 (为了测试先强行加钱)  
    EconomySystem::GetInstance()-\>AddGold(1000); 

    int cost \= 200;  
    if (EconomySystem::GetInstance()-\>SpendGold(cost)) {  
        auto cannon \= Building::create(Core::BuildingType::kCannon, 1, 0);  
        cannon-\>setPosition(GridToPixel(15, 10)); // 放在大本营右边  
          
        // 开始建造：5秒内无法攻击  
        cannon-\>StartConstruction(5.0f);  
          
        game\_layer\_-\>addChild(cannon);  
        cocos2d::log("Test: Cannon construction started.");  
    }  
}

void GameplayIntegrationScene::OnSpawnBarbarian() {  
    // 假设野蛮人占 1 人口  
    if (EconomySystem::GetInstance()-\>AddTroopPopulation(1)) {  
        // 创建野蛮人 (Lv1, 玩家所有)  
        auto barb \= Unit::create(Core::TroopType::kBarbarian, 1, 0);  
          
        // 放在稍远的地方 (像素坐标)  
        barb-\>setPosition(Vec2(200, 200));  
          
        game\_layer\_-\>addChild(barb);  
        cocos2d::log("Test: Barbarian spawned.");  
    } else {  
        cocos2d::log("Test: Not enough population\!");  
    }  
}

void GameplayIntegrationScene::update(float dt) {  
    // 5\. \[契约 2.4\] UI 轮询数据  
    auto eco \= EconomySystem::GetInstance();  
    resource\_label\_-\>setString(StringUtils::format(  
        "Gold: %d/%d | Pop: %d/%d",   
        eco-\>GetCurrentGold(), eco-\>GetMaxGold(),  
        eco-\>GetCurrentPopulation(), eco-\>GetMaxPopulation()  
    ));  
}  
