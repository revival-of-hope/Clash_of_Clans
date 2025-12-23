# **GamePlay 事件系统接口文档**

版本: 1.1 (新增事件列表速查)  
作者: Developer B (Gameplay)  
模块: Contract/Gameplay/GameEvents.h

## **1\. 概述 (Overview)**

本系统采用 观察者模式 (Observer Pattern)，旨在解耦 Gameplay 层 (逻辑) 与 UI/Audio 层 (表现)。  
UI 层无需每帧轮询逻辑数据的变化，而是通过注册监听器，被动接收逻辑层的状态变更通知。

* **推送机制 (Push)**: 逻辑层发生变化 \-\> GameEventManager 广播 \-\> 所有注册的 IGameEventListener 收到通知。  
* **线程安全**: 目前设计为单线程同步调用 (Main Thread)，请勿在回调中执行耗时操作。

## **2\. 稳定事件列表速查 (Stable Event List)**

这是 Gameplay 层承诺向 UI/Audio 层发送的完整事件契约。请 Dev C 根据此表实现对应的表现逻辑。

| 事件接口名 | 触发时机 | 核心载荷 (Payload) | UI/Dev C 的预期行为 |
| :---- | :---- | :---- | :---- |
| **OnResourceChanged** | 采集资源、造兵消费、升级消费 | Type (金/水/人口), Current, Max, Delta (变化量) | 刷新顶部资源栏数字；如果 Delta \> 0，播放资源飞入动画。 |
| **OnEntitySpawned** | 玩家点击放置单位、地图初始化加载建筑 | InstanceID, Type, Level, Pos, OwnerID | 创建 Sprite 节点；创建血条组件；**必须**维护 ID \-\> Node 的映射表。 |
| **OnEntityDestroyed** | 单位/建筑血量归零 | InstanceID, Type | 播放死亡音效；播放死亡动画；从场景移除节点；清理映射。 |
| **OnBuildingStateChanged** | 开始建造、建造完成、被摧毁 | InstanceID, State, TimeRemaining | Constructing: 显示倒计时/进度条。 Destroyed: 切换为废墟图片。 Idle: 隐藏进度条，恢复正常。 |
| **OnEntityDamaged** | 受到任何攻击伤害时 | TargetID, CurrentHP, MaxHP, DamageAmount | 根据 TargetID 找到血条，更新进度 (Current/Max)；弹出飘血数字。 |
| **OnProjectileFired** | 远程单位攻击前摇结束时 | SourceID, TargetPos, ProjectileType | **Audio**: 播放发射音效 (如弓弦声)。 **VFX**: (可选) 如果逻辑层不负责创建Sprite，UI层需在此创建。 |
| **OnProjectileHit** | 投射物抵达目标或命中消失时 | Pos, ProjectileType | **VFX**: 在 Pos 位置播放爆炸/击中特效。 **Audio**: 播放命中音效。 |
| **OnBattleStarted** | 放置第一个兵 或 进入战斗场景时 | TimeLimit (秒) | 启动屏幕顶部倒计时；播放战斗 BGM。 |
| **OnBattleEnded** | 达成胜负条件 或 时间耗尽 | Result (Win/Loss), Stars, Percentage, Loot | 暂停倒计时；停止 BGM；弹出结算面板显示星级和获取资源。 |

## **3\. 数据载荷定义 (Payload Definitions)**

所有事件都携带特定的结构体数据，用于传递上下文信息。

### **3.1 资源变更 (ResourceUpdateEvent)**

当金币、圣水或人口发生变化时触发。

| 字段名 | 类型 | 说明 |
| :---- | :---- | :---- |
| resource\_type | string | 资源类型字符串 ("Gold", "Elixir", "Population") |
| current\_amount | int | 变更后的当前值 |
| max\_capacity | int | 当前的最大上限 |
| change\_amount | int | 本次变动的差值 (例如 \-200 或 \+50) |

### **3.2 实体生成 (EntitySpawnEvent)**

当单位被部署或地图加载建筑时触发。

| 字段名 | 类型 | 说明 |
| :---- | :---- | :---- |
| instance\_id | int | 实体的全局唯一 ID (用于后续索引) |
| owner\_id | int | 阵营 ID (0: 玩家, 1: 敌方/AI) |
| x, y | float | 初始世界坐标 (像素) |
| level | int | 等级 (决定外观贴图) |
| is\_building | bool | true 为建筑，false 为兵种 |
| troop\_type | enum | 兵种类型枚举 (如果是兵) |
| building\_type | enum | 建筑类型枚举 (如果是建筑) |

### **3.3 实体销毁 (EntityDestroyEvent)**

当单位死亡或建筑完全倒塌后触发。

| 字段名 | 类型 | 说明 |
| :---- | :---- | :---- |
| instance\_id | int | 对应实体的 ID |
| is\_building | bool | 是否为建筑 |

### **3.4 建筑状态变更 (BuildingStateEvent)**

用于处理建造进度条、废墟状态切换。

| 字段名 | 类型 | 说明 |
| :---- | :---- | :---- |
| instance\_id | int | 建筑 ID |
| type | enum | 建筑类型 |
| new\_state | enum | kConstructing (建造中), kIdle (正常), kDestroyed (废墟) |
| time\_remaining | float | 剩余建造时间 (秒) |
| total\_build\_time | float | 总建造时间 (用于计算进度条百分比) |

### **3.5 伤害与血量 (DamageEvent)**

当实体受到攻击时触发。

| 字段名 | 类型 | 说明 |
| :---- | :---- | :---- |
| target\_instance\_id | int | 受害者 ID |
| damage\_amount | int | 本次受到的伤害值 |
| current\_hp | int | 剩余血量 |
| max\_hp | int | 最大血量 (UI 可据此计算血条百分比) |
| is\_critical | bool | 是否暴击 (预留) |

### **3.6 投射物事件**

* **发射 (ProjectileEvent)**: 远程单位攻击前摇结束，发射子弹时触发。  
* **命中 (ProjectileHitEvent)**: 子弹击中目标或地面时触发。

### **3.7 战斗结算 (BattleEndEvent)**

战斗结束时触发。

| 字段名 | 类型 | 说明 |
| :---- | :---- | :---- |
| result | enum | kVictory (胜利), kDefeat (失败), kTimeOut (超时) |
| stars\_earned | int | 获得的星数 (0-3) |
| destruction\_percent | int | 摧毁率 (0-100) |
| gold\_stolen | int | 掠夺金币总数 |
| elixir\_stolen | int | 掠夺圣水总数 |

## **4\. 接口用法 (Usage Guide)**

### **4.1 对于 Dev C (UI / Audio) \- 如何监听事件**

您需要让您的类（如 HUDLayer 或 AudioManager）继承 Gameplay::IGameEventListener，并实现感兴趣的回调函数。

**示例代码 (HUDLayer.h):**

```cpp
#include "Contract/Gameplay/GameEvents.h"

class HUDLayer : public cocos2d::Layer, public Gameplay::IGameEventListener {
public:
    virtual bool init() override {
        // ... 初始化代码 ...
        
        // [关键步骤 1] 注册监听器
        Gameplay::GameEventManager::GetInstance()->AddListener(this);
        return true;
    }

    virtual void onExit() override {
        // [关键步骤 2] 移除监听器 (必须做，否则会崩溃)
        Gameplay::GameEventManager::GetInstance()->RemoveListener(this);
        cocos2d::Layer::onExit();
    }

    // [关键步骤 3] 实现回调接口
    
    // 资源变动 -> 更新 UI 数字
    virtual void OnResourceChanged(const Gameplay::ResourceUpdateEvent& evt) override {
        if (evt.resource_type == "Gold") {
            this->UpdateGoldLabel(evt.current_amount);
            if (evt.change_amount > 0) {
                this->PlayResourceGainEffect("Gold");
            }
        }
    }

    // 战斗结束 -> 弹窗
    virtual void OnBattleEnded(const Gameplay::BattleEndEvent& evt) override {
        this->ShowResultPopup(evt.stars_earned, evt.destruction_percent);
    }
    
    // 其他不关心的接口可以不实现 (基类有默认空实现)
};
```

### **4.2 对于 Dev B (Gameplay) \- 如何广播事件**

在逻辑代码中，通过 Broadcast... 方法发送通知。

**示例代码 (EconomySystem.cpp):**

```cpp
void EconomySystem::AddGold(int amount) {
    int old_val = current_gold_;
    current_gold_ += amount;
    // ... 上限处理 ...
    
    // [广播事件]
    Gameplay::ResourceUpdateEvent evt;
    evt.resource_type = "Gold";
    evt.current_amount = current_gold_;
    evt.max_capacity = max_gold_;
    evt.change_amount = amount; // 这里是 +50
    
    Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
}
```

## **5\. 注意事项 (Best Practices)**

1. **生命周期管理**:  
   * 务必在 onExit() 或析构函数中调用 RemoveListener(this)。如果对象被销毁了但还在监听列表中，下一次广播会导致野指针崩溃。  
2. **UI 映射维护**:  
   * 收到 OnEntitySpawned 时，UI 层应该建立一个 std::map\<int, Node\*\> entity\_map\_。  
   * 收到 OnEntityDestroyed 或 OnEntityDamaged 时，使用 evt.instance\_id 去 map 里查找对应的节点进行操作。  
3. **不要在回调中修改逻辑**:  
   * IGameEventListener 的回调主要用于**只读**显示。尽量避免在回调里反过来调用 EconomySystem::AddGold 等修改逻辑状态的函数，这可能导致死循环或状态不一致。