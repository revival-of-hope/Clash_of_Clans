Gameplay Module API Reference

Version: 1.0 (Draft)
Maintainer: Dev B (Gameplay)

本文档详细说明了 Gameplay 模块对外暴露的所有可用接口。Dev A (Engine) 和 Dev C (UI) 请以此为准进行接入。

目录

核心系统 (Systems)

EconomySystem (经济系统)

CombatResolver (战斗仲裁)

GameConfig (数值配置)

实体交互 (Entities)

Unit (兵种单位)

Building (建筑单位)

组件访问 (Components)

集成示例 (Integration Examples)

1. 核心系统 (Systems)

这些是全局单例类，负责管理游戏的宏观逻辑。

1.1 EconomySystem

主要用户: Dev C (UI)
用途: 获取资源数值显示 UI、处理商店购买逻辑、收集资源。

接口方法

参数

返回值

说明

GetInstance()

无

EconomySystem*

获取单例指针

Reset()

无

void

重置所有资源（新游戏开始时调用）

GetCurrentGold()

无

int

获取当前金币数量

GetMaxGold()

无

int

获取当前金币上限

GetCurrentElixir()

无

int

获取当前圣水数量

GetMaxElixir()

无

int

获取当前圣水上限

GetCurrentPopulation()

无

int

获取当前已占用人口

GetMaxPopulation()

无

int

获取当前最大人口上限

CanAfford(...)

int gold, int elixir

bool

检查是否买得起（不扣款，用于UI按钮置灰）

SpendGold(...)

int amount

bool

尝试消费金币。成功返回 true 并扣款；失败返回 false

SpendElixir(...)

int amount

bool

尝试消费圣水。同上

TryCollectResource(...)

Building*

int

点击金矿时调用。尝试收集资源，自动加到库中

RecalculateLimits(...)

Vector<Building*>

void

重要：当建筑升级/建造完成时调用，重新计算容量上限

⚠️ 注意事项:

不要直接修改资源变量，必须通过 Spend 或 Add 系列函数，以触发边界检查（上限截断）。

RecalculateLimits 必须传入当前场景中所有建筑的列表，否则计算的上限会出错。

1.2 CombatResolver

主要用户: Dev A (Engine) / Scene
用途: 战斗系统的初始化、投射物管理。

接口方法

参数

返回值

说明

GetInstance()

无

CombatResolver*

获取单例指针

Initialize(...)

Node* battle_layer

void

必须调用。传入战斗场景的 Layer，用于挂载投射物和 Update 调度

SpawnProjectile(...)

attacker, target, dmg, type

void

(通常由 Unit 内部调用) 生成一个飞行投射物

ResolveMeleeAttack(...)

attacker, target, dmg

void

(通常由 Unit 内部调用) 立即结算近战伤害

⚠️ 注意事项:

Initialize(this) 必须在 Scene::init() 中尽早调用。

该系统会自动在传入的 battle_layer 上开启 schedule，无需手动调用 Update。

1.3 GameConfig

主要用户: Dev C (UI)
用途: 获取静态配置数据（如造价、属性），用于商店详情页或信息面板。

接口方法

参数

返回值

说明

GetInstance()

无

GameConfig*

获取单例指针

GetTroopStats(...)

TroopType, int level

UnitStats

获取指定等级兵种的详细属性

GetBuildingStats(...)

BuildingType, int level

BuildingStats

获取指定等级建筑的详细属性

GetUpgradeCost(...)

BuildingType, int level

int

获取升级/建造所需的资源消耗

2. 实体交互 (Entities)

所有实体均继承自 BaseEntity (它是 cocos2d::Node 的子类)。

2.1 Unit

主要用户: Dev A (LevelManager), Dev C (Gameplay Input)
用途: 生成士兵、投放士兵。

接口方法

参数

返回值

说明

create(...)

TroopType, level, owner_id

Unit*

静态工厂。创建并初始化士兵对象

GetTroopType()

无

TroopType

获取兵种枚举类型

GetState()

无

State

获取当前状态 (Idle/Move/Attack/Dead)

使用范例 (投放士兵):

// 1. 检查人口
if (EconomySystem::GetInstance()->AddTroopPopulation(housing_space)) {
    // 2. 创建单位 (owner_id: 0=玩家, 1=敌人)
    auto unit = Unit::create(Core::TroopType::kBarbarian, 1, 0);
    unit->setPosition(touchPos);
    
    // 3. 添加到场景
    battleLayer->addChild(unit);
}


2.2 Building

主要用户: Dev A (MapSystem), Dev C (Construction UI)
用途: 建造建筑、收集资源、点击交互。

接口方法

参数

返回值

说明

create(...)

BuildingType, level, owner_id

Building*

静态工厂。创建建筑对象

StartConstruction(...)

float duration

void

开始建造倒计时。期间建筑功能暂停

IsConstructing()

无

bool

检查是否正在建造/升级中

GetStoredResource()

无

int

获取当前暂存的资源量（用于 UI 显示气泡）

GetOccupiedRect()

无

Rect

获取建筑在世界坐标下的占地矩形

⚠️ 注意事项:

Z-Order: 建筑内部默认 ZOrder=10，单位默认 ZOrder=20。请确保 MapLayer 的 ZOrder 小于 10。

瓦片对齐: Building 的位置应该是瓦片的中心点。请配合 MapSystem 使用。

3. 组件访问 (Components)

UI 层可能需要获取实体身上的组件来更新 HUD（如血条、进度条）。
获取方式: entity->getChildByName("ComponentName")

HealthComp

用途: 获取血量信息。

接口方法

说明

GetHealthPercentage()

返回 0.0f ~ 1.0f 的浮点数

IsDead()

返回是否死亡

SetHealthBarOffset(Vec2)

调整血条显示位置（相对于实体中心）

UI 获取血量范例:

auto hpComp = dynamic_cast<HealthComp*>(selectedEntity->getChildByName("HealthComp"));
if (hpComp) {
    float pct = hpComp->GetHealthPercentage();
    updateUiProgressBar(pct);
}


4. 集成示例 (Integration Examples)

场景 A：玩家在商店购买并放置一个加农炮

// 假设在 UIManager 中处理点击事件
void UIManager::OnBuildCannonClicked() {
    auto economy = EconomySystem::GetInstance();
    auto config = GameConfig::GetInstance();
    
    int cost = config->GetUpgradeCost(Core::BuildingType::kCannon, 1);
    
    // 1. 检查钱够不够
    if (economy->CanAfford(cost, 0)) {
        
        // 2. 扣款
        if (economy->SpendGold(cost)) {
            
            // 3. 创建建筑实体 (Owner=0 玩家)
            auto cannon = Building::create(Core::BuildingType::kCannon, 1, 0);
            
            // 4. 设置位置 (需由 MapSystem 转换为网格坐标)
            cannon->setPosition(currentTilePosition); 
            
            // 5. 开始建造 (耗时 10秒)
            cannon->StartConstruction(10.0f);
            
            // 6. 添加到场景
            gameScene->addChild(cannon);
            
            // 7. [重要] 通知经济系统重新计算上限 (虽然加农炮不加容量，但好习惯)
            // 注意：这里需要传入场景中所有建筑的列表，实际代码中通常由 LevelManager 维护这个列表
            // economy->RecalculateLimits(allBuildings); 
        }
    } else {
        ShowError("Not enough Gold!");
    }
}


场景 B：玩家点击金矿收集金币

// 假设在 TouchHandler 中检测到了点击 Building
void OnBuildingTouched(Building* building) {
    if (building->GetBuildingType() == Core::BuildingType::kGoldMine) {
        
        // 调用 EconomySystem 处理收集逻辑
        // 它会自动检查仓库剩余空间，并返回实际收集到的数量
        int collectedAmount = EconomySystem::GetInstance()->TryCollectResource(building);
        
        if (collectedAmount > 0) {
            PlaySound("collect_coin.mp3");
            ShowFloatingText("+" + std::to_string(collectedAmount));
        }
    }
}


场景 C：初始化战斗场景

// HelloWorldScene::init()
bool HelloWorldScene::init() {
    if (!Scene::init()) return false;
    
    // 1. 初始化战斗仲裁系统 (传入当前 Layer)
    CombatResolver::GetInstance()->Initialize(this);
    
    // 2. 重置经济数据 (如果是新关卡)
    EconomySystem::GetInstance()->Reset();
    
    // 3. 加载地图...
    // 4. 生成单位...
    
    return true;
}


🧠 Dev B 的特别叮嘱 (致队友)

关于瓦片地图 (TileMap):
目前的 GameConstants.h 中 kTileWidth 和 kTileHeight 分别是 64 和 32 (2:1 比例)。
如果我们要改成绝对俯视 (Top-Down)，请 Dev A 务必通知我修改这些常量为正方形 (如 64x64)，否则我的 PathAgent 距离计算和 IsTargetInRange 判定可能会产生视觉偏差。

关于内存管理:
Unit::create 和 Building::create 返回的对象都是 autorelease 的。如果你不立刻 addChild 它们，它们会在下一帧自动销毁。请务必注意持有引用。

关于 UI 刷新:
UI 不需要每一帧都去查询 EconomySystem。建议使用 Observer Pattern (观察者模式)。虽然目前我还没写 GameEvents 的具体实现，但建议预留 OnResourceChanged 的回调接口。目前阶段，你们可以在 update 中每隔几帧查询一次单例。