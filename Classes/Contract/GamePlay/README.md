## 1. Building.h（建筑实体）

### 1.1 模块定位与职责

`Building` 是游戏中**所有建筑类对象的统一实现**，
用于表示具备固定占地、可建造、可升级、可攻击或可产出资源的实体。

它的设计目标是：

* 在 `BaseEntity` 的基础上，封装**建筑专属逻辑**
* 将建筑的 **数值、状态、表现、功能** 集中管理
* 明确区分「建造中 / 正常工作 / 失效」等状态
* 为战斗、经济、寻路系统提供清晰、稳定的接口

`Building` 继承自 `BaseEntity`，
因此天然具备实体 ID、阵营、生命周期管理与全局注册能力。

---

### 1.2 核心功能

`Building` 当前负责以下核心功能：

* **建筑的创建、初始化与销毁**
* **建筑状态管理（空闲 / 建造中等）**
* **基于配置表的数值加载**
* **防御建筑的攻击逻辑**
* **资源建筑的生产与收集**
* **建筑占地信息与障碍物注册**
* **与动画状态系统解耦的状态表达**

---

### 1.3 生命周期与所有权说明

#### 1.3.1 生命周期继承关系

* `Building` 的生命周期遵循 `BaseEntity` 规则
* 进入场景 → 自动注册为活跃实体
* 离开场景 → 自动从实体列表中移除

#### 1.3.2 onEnter / onExit 的特殊意义

```cpp
virtual void onEnter() override;
virtual void onExit() override;
```

**设计说明：**

* 建筑通常需要在**世界坐标稳定后**才能：

  * 注册为地图障碍物
  * 参与寻路或碰撞判断
* 因此障碍物注册逻辑放在 `onEnter()` 中执行
* `onExit()` 中负责安全注销

该设计避免了：

* 坐标尚未设置就注册障碍
* 重复注册 / 遗漏注销的问题

---

### 1.4 接口说明

---

#### 1.4.1 创建建筑实例

```cpp
static Building* create(Core::BuildingType type, int level, int owner_id);
```

**参数：**

* `type`
  建筑类型（由 `Core::BuildingType` 定义）
* `level`
  初始等级
* `owner_id`
  所属阵营 / 玩家 ID

**返回值：**

* 成功：返回已初始化的 `Building*`
* 失败：返回 `nullptr`

**说明：**

* 使用 `autorelease` 机制管理内存
* 内部会调用对应的 `init()` 完成配置加载

---

#### 1.4.2 初始化接口

```cpp
virtual bool init(Core::BuildingType type, int level, int owner_id);
```

**功能：**

* 读取建筑配置（数值、攻击属性、尺寸等）
* 初始化等级、阵营、状态
* 加载并创建建筑贴图

---

#### 1.4.3 每帧更新接口

```cpp
virtual void update(float dt) override;
```

**说明：**
* 仅用于内部刷新
* 统一的建筑逻辑更新入口
* 内部可能处理：

  * 攻击冷却
  * 目标检测
  * 资源生产
  * 建造倒计时

---

### 1.5 状态与建造逻辑

#### 1.5.1 查询建筑等级

```cpp
int GetLevel() const;
```

---

#### 1.5.2 建造 / 升级状态判断

```cpp
bool IsConstructing() const;
```

**说明：**

* 当返回 `true` 时：

  * 建筑通常不攻击
  * 不产出资源
  * 不提供功能效果

---

#### 1.5.3 开始建造或升级

```cpp
void StartConstruction(float duration);
```

**参数：**

* `duration`
  建造或升级所需时间（秒）

---

### 1.6 战斗相关接口（Combat Interface）

#### 1.6.1 是否可以攻击某类目标

```cpp
bool CanAttack(Core::GeneralType target_type) const;
```

**说明：**

* 使用位掩码判断建筑攻击能力
* 支持地面 / 空中 / 混合目标判定

---

#### 1.6.2 获取攻击数值

```cpp
int GetDamage() const;
float GetRangeInPixels() const;
```

* 攻击范围已自动转换为像素单位
* 内部基于瓦片大小计算

---

### 1.7 经济与功能接口（Economy & Utility）

#### 1.7.1 获取当前存储资源

```cpp
int GetStoredResource() const;
```

---

#### 1.7.2 收集资源

```cpp
int CollectResource(int max_amount = -1);
```

**参数：**

* `max_amount`

  * `-1` 表示不限
  * 用于限制单次收集上限

**返回值：**

* 实际收集到的资源数量

---

### 1.8 占地与空间接口

#### 1.8.1 获取占地矩形

```cpp
cocos2d::Rect GetOccupiedRect() const;
```

**用途：**

* 建造放置合法性检测
* 寻路边界判断
* 地图障碍注册

---

#### 1.8.2 获取占地尺寸（瓦片单位）

```cpp
int GetWidthInTiles() const;
int GetHeightInTiles() const;
```

---

### 1.9 类型与状态查询

```cpp
Core::BuildingType GetBuildingType() const;
Core::BuildingAnimationState GetState() const;
```

---

### 1.10 使用示例

```cpp
auto cannon = Building::create(
    Core::BuildingType::kCannon,
    1,
    playerId
);

cannon->setPosition(tileCenter);
mapLayer->addChild(cannon);
```

---

### 1.11 设计说明（给维护者）

* `Building` 是**强规则实体**
* 它整合了：

  * 数值系统
  * 状态机
  * 战斗与经济逻辑
* 外部系统应：

  * 查询状态
  * 触发行为
  * **而不是**直接修改内部字段

该类是后续以下系统的基础：

* 建筑升级系统
* 攻防 AI
* 放置与碰撞检测
* 编辑器 / 预览模式

---

## 2. Unit.h（兵种单位）

### 2.1 功能职责

`Unit` 表示**可移动、可攻击的兵种实体**，是战斗系统中的核心执行者之一。
它在 `BaseEntity` 提供的通用实体能力之上，扩展了：

* 兵种数值（攻击力、攻击范围、目标类型等）
* 单位状态机（Idle / Move / Attack）
* 朝向管理（Facing）
* 攻击合法性判定（能否攻击空中/地面）
* 每帧逻辑更新（战斗与行为驱动）

`Unit` 是**纯逻辑实体**，UI 表现（动画、特效）仅作为内部实现细节存在，不对外暴露。

---

### 2.2 继承关系

```
cocos2d::Node
  └── BaseEntity
        └── Unit
```

---

### 2.3 生命周期与所有权

* 通过 `Unit::create(...)` 创建，返回 `autorelease` 对象
* 自动参与 `BaseEntity` 的全局实体注册表
* 生命周期由场景（Scene）管理
* 销毁应使用 `MarkForDestruction()`，而非立即移除

---

### 2.4 创建与初始化

#### 2.4.1 工厂方法

```cpp
static Unit* create(Core::TroopType type, int level, int owner_id);
```

**参数说明：**

* `type`：兵种类型（如 Barbarian、Archer、Bomber）
* `level`：兵种等级，用于加载对应数值
* `owner_id`：所属阵营 / 玩家 ID

**行为约定：**

* 内部会加载 `Core::UnitStats`
* 初始化基础状态为 `Idle`
* 设置阵营归属（owner_id）

---

### 2.5 核心接口说明

#### 2.5.1 每帧更新

```cpp
virtual void update(float dt) override;
```

**职责：**

* 驱动状态机逻辑
* 执行攻击判定与冷却
* 响应目标变化或销毁标记

---

#### 2.5.2 单位类型与数值访问

```cpp
Core::GeneralType GetGeneralType() const;
Core::TroopType GetTroopType() const;
int GetDamage() const;
float GetRangeInPixels() const;
```

**说明：**

* `GetGeneralType`：返回 Ground / Air，用于目标筛选
* `GetTroopType`：返回具体兵种枚举，供战斗规则识别
* `GetRangeInPixels`：统一以**像素单位**返回攻击范围，避免上层重复换算

---

#### 2.5.3 攻击合法性判定

```cpp
bool CanAttack(Core::GeneralType target_type) const;
```

**职责：**

* 判断该单位是否可以攻击某种目标类型
* 用于 CombatResolver / AI 决策层

**设计原则：**

* Unit 只回答“能不能”
* 不关心目标是谁、不执行攻击

---

#### 2.5.4 状态机控制

```cpp
void SetState(Core::UnitAnimationState new_state);
Core::UnitAnimationState GetState() const;
```

**状态示例：**

* `kIdle`
* `kMove`
* `kAttack`

**行为约定：**

* 状态切换会处理必要的内部清理

  * 停止当前行为
  * 重置计时器
  * 切换动画（内部）

---

#### 2.5.5 朝向控制

```cpp
void SetFacing(Core::Facing facing);
```

**说明：**

* 用于决定单位面朝方向
* 主要影响视觉表现与攻击方向
* 不对外暴露 Sprite 细节

---

### 2.6 内部组成（实现细节）

* `Core::UnitStats stats_`
  单位完整数值来源，来自配置系统

* `cocos2d::Sprite* visual_sprite_`
  单位外观，仅限 Unit 内部使用

* `current_state_`
  当前逻辑状态（权威状态）

* `current_facing_`
  当前朝向状态

---

### 2.7 设计约束（Contract）

* Unit **不直接操作 UI 层**
* Unit **不管理其他实体的生命周期**
* Unit **不参与全局调度，仅响应 update**
* 所有战斗判定应通过显式接口（如 `CanAttack`）完成
* 外部系统不得直接修改内部状态字段

---

### 2.8 典型使用场景

* 战斗系统遍历 `BaseEntity::GetAllEntities()`，筛选 `Unit`
* AI 系统查询 `CanAttack` 决定目标
* 动画系统监听状态变化（非反向依赖）
* CombatResolver 使用 `GetDamage`、`GetRangeInPixels` 进行结算

## 3. HealthComp.h（生命值组件）

### 3.1 功能职责

`HealthComp` 是一个**可复用的组件节点**，用于管理实体的生命值状态，并提供一个**轻量级的血条显示**。
通常挂载在 `Unit` 或 `Building` 上，为其提供以下能力：

* 生命值（HP）与最大生命值管理
* 伤害结算与死亡判定
* 简易血条的绘制与更新
* 与实体逻辑解耦的状态查询接口

该组件**不关心战斗规则来源**，只对“数值变化 → 状态变化”负责。

---

### 3.2 设计定位

* **组件而非实体**：不参与战斗决策、不注册全局实体表
* **数据权威**：HP 与死亡状态只由 `HealthComp` 维护
* **表现内聚**：血条绘制逻辑封装在组件内部
* **低成本**：使用 `DrawNode`，无需贴图资源

---

### 3.3 继承关系

```
cocos2d::Node
  └── HealthComp
```

---

### 3.4 生命周期与使用方式

* 通过 `CREATE_FUNC(HealthComp)` 创建
* 作为子节点挂载到 `Unit` / `Building`
* 生命周期完全跟随父节点
* 不允许独立存在于场景中

典型挂载方式：

```cpp
auto hp = HealthComp::create();
hp->InitStats(100);
entity->addChild(hp);
```

---

### 3.5 初始化接口

#### 3.5.1 初始化组件本身

```cpp
bool init() override;
```

**职责：**

* 创建内部 `DrawNode`
* 初始化基础状态（未死亡）
* 不设置具体数值

---

#### 3.5.2 初始化生命值数据

```cpp
void InitStats(int max_hp);
```

**参数说明：**

* `max_hp`：最大生命值

**行为约定：**

* `current_hp_` 会被设置为 `max_hp`
* 自动刷新血条显示
* 可在组件创建后立即调用

---

### 3.6 核心功能接口

#### 3.6.1 承受伤害

```cpp
bool TakeDamage(int amount);
```

**职责：**

* 扣减当前生命值
* 自动处理下限（不小于 0）
* 更新血条显示
* 在生命值归零时标记死亡状态

**返回值语义：**

* `true`：本次伤害导致死亡
* `false`：仍然存活

> ⚠️ HealthComp **只负责判定死亡**，不负责移除实体

---

#### 3.6.2 治疗接口（预留）

```cpp
void Heal(int amount);
```

**说明：**

* 增加当前生命值
* 不会超过 `max_hp_`
* 适用于后续扩展治疗单位、维修建筑等机制

---

### 3.7 状态查询接口

```cpp
bool IsDead() const;
float GetHealthPercentage() const;
```

**说明：**

* `IsDead`：用于逻辑层快速判断
* `GetHealthPercentage`：

  * 返回范围 `[0.0, 1.0]`
  * 适用于 UI 或 AI 阈值判断

---

### 3.8 血条显示控制

#### 3.8.1 设置血条偏移

```cpp
void SetHealthBarOffset(cocos2d::Vec2 offset);
```

**用途：**

* 控制血条相对于父节点中心的位置
* 适配不同体型（单位 / 建筑）

---

#### 3.8.2 内部绘制机制

* 使用 `cocos2d::DrawNode`
* 每次 HP 变化都会调用 `UpdateHealthBar`
* 血条长度与当前 HP 百分比线性对应
* 不依赖任何图片资源

---

### 3.9 内部状态说明

* `current_hp_`：当前生命值
* `max_hp_`：最大生命值
* `is_dead_`：死亡标记（权威状态）
* `health_bar_node_`：内部绘制节点
* `bar_offset_`：血条相对偏移

---

### 3.10 设计约束（Contract）

* HealthComp **不主动销毁父节点**
* HealthComp **不参与攻击、防御、AI 决策**
* 外部系统不得直接修改 HP 成员变量
* 所有生命变化必须通过接口完成
* 表现逻辑不得反向依赖战斗系统

---

### 3.11 典型使用场景

* CombatResolver 调用 `TakeDamage`
* Entity 在 `update` 中检测 `IsDead`
* UI / Debug 层读取 `GetHealthPercentage`
* 建筑与单位共用同一生命值组件

## 4. BaseEntity.h（游戏实体基类）

### 4.1 模块定位与职责

`BaseEntity` 是游戏中**所有可参与逻辑运算的实体对象的统一基类**，
包括但不限于：

* 单位（Units）
* 建筑（Buildings）
* 后续可能扩展的可交互对象

它的设计目标是：

* 统一实体的**生命周期管理方式**
* 为所有实体提供**一致的身份与阵营模型**
* 建立一个**权威、集中、只读的实体注册入口**
* 避免各系统各自维护实体列表而导致的混乱

`BaseEntity` 继承自 `cocos2d::Node`，
因此既是**场景节点**，也是**逻辑实体**。

---

### 4.2 核心功能

`BaseEntity` 当前提供以下核心能力：

* **统一的实体生命周期入口**
* **自动维护全局实体注册表**
* **实例唯一 ID 管理**
* **阵营 / 所有权标识**
* **安全的延迟销毁机制**
* **基础的敌我关系判断**
* **通用的中心点坐标获取接口**

---

### 4.3 生命周期与所有权说明

#### 4.3.1 生命周期管理原则

* `BaseEntity` 的生命周期**完全依附于 Scene Graph**
* 实体是否“存活”，由其是否存在于场景树中决定

#### 4.3.2 自动注册 / 注销机制

* 当实体进入场景（`onEnter`）时：

  * 自动加入全局实体注册表
* 当实体离开场景（`onExit`）时：

  * 自动从注册表中移除

**外部模块不需要、也不应该：**

* 手动维护实体列表
* 手动注册或注销实体

---

### 4.4 接口说明

#### 4.4.1 初始化接口

```cpp
virtual bool init() override;
```

**说明：**

* 所有派生类都应调用 `BaseEntity::init()`
* 用于初始化实体的基础状态

---

#### 4.4.2 生命周期回调（内部使用）

```cpp
virtual void onEnter() override;
virtual void onExit() override;
```

**说明：**

* 用于维护全局实体注册表
* 子类若 override，必须调用父类实现

---

#### 4.4.3 每帧更新接口

```cpp
virtual void update(float dt) override;
```

**参数：**

* `dt`
  上一帧到当前帧的时间间隔（秒）

**说明：**

* 实体的核心逻辑更新入口
* 子类可 override 实现 AI、状态更新等逻辑
* 不建议在此函数中直接删除实体

---

#### 4.4.4 实例 ID 管理

```cpp
void set_instance_id(int id);
int get_instance_id() const;
```

**说明：**

* `instance_id` 用于唯一标识一个实体实例
* 通常由上层系统（如工厂或管理器）分配
* `BaseEntity` 本身不负责生成 ID

---

#### 4.4.5 阵营 / 所有权管理

```cpp
void set_owner_id(int id);
int get_owner_id() const;
```

**说明：**

* `owner_id` 表示实体所属的阵营或玩家
* 具体 ID 的含义由游戏规则定义

---

#### 4.4.6 敌我关系判断

```cpp
bool IsAlly(const BaseEntity* other) const;
```

**参数：**

* `other`
  另一个实体指针

**返回值：**

* `true`：同一阵营（`owner_id` 相同）
* `false`：不同阵营或无效对象

---

#### 4.4.7 延迟销毁机制

```cpp
void MarkForDestruction();
bool IsMarkedForDestruction() const;
```

**说明：**

* 调用 `MarkForDestruction()` 只会打上销毁标记
* 实体不会立即被移除或释放
* 适用于在遍历实体列表时安全请求销毁

**设计目的：**

* 避免在 `update()` 或遍历过程中直接删除节点
* 防止迭代器失效和未定义行为

---

#### 4.4.8 获取实体中心点坐标

```cpp
cocos2d::Vec2 GetCenterPosition() const;
```

**返回值：**

* 实体中心点的**世界坐标**

**使用场景：**

* 距离计算
* 攻击或技能范围判断
* 对齐与定位逻辑

---

#### 4.4.9 全局实体访问接口

```cpp
static cocos2d::Vector<BaseEntity*>& GetAllEntities();
```

**返回值：**

* 当前场景中所有**活跃的 `BaseEntity` 实例**

**说明：**

* 该列表由 `BaseEntity` 内部维护
* 外部模块只允许读取，不应修改内容

**典型用途：**

* AI 搜索目标
* 全局逻辑遍历
* UI 展示实体信息

---

### 4.5 使用示例

#### 4.5.1 基本继承方式

```cpp
class Soldier : public BaseEntity
{
public:
    CREATE_FUNC(Soldier);

    virtual bool init() override
    {
        if (!BaseEntity::init()) return false;
        scheduleUpdate();
        return true;
    }

    virtual void update(float dt) override
    {
        if (IsMarkedForDestruction()) return;
        // 单位逻辑
    }
};
```

---

#### 4.5.2 请求安全销毁实体

```cpp
entity->MarkForDestruction();
```

随后由上层逻辑在合适时机：

```cpp
entity->removeFromParent();
```

---

### 4.6 设计说明（给维护者）

* `BaseEntity` 是**逻辑层的基础抽象**
* 它不负责：

  * AI 决策
  * 战斗规则
  * UI 表现
* 它只解决三个问题：

  > **我是谁？**
  > **我属于谁？**
  > **我是否还活着？**

通过将这些能力集中在 `BaseEntity` 中：

* 所有实体行为具有统一语义
* 全局系统不再依赖零散的实体列表
* 后续扩展（EntityManager、战斗系统）可以自然建立在其之上

## 5. CombatResolver（战斗仲裁系统）

### 5.1 模块定位与职责

`CombatResolver` 是游戏中的**战斗仲裁中枢（Authority）**，以**单例**形式存在，统一管理所有攻击行为与伤害结算。

它的核心设计目标是：

* 将「谁攻击谁、如何扣血」从单位 / 建筑中剥离
* 统一管理所有投射物的生命周期
* 确保伤害结算的**唯一性与一致性**
* 为未来扩展复杂战斗公式提供集中入口

---

### 5.2 核心职责划分

`CombatResolver` 当前承担以下职责：

* **投射物管理**

  * 创建、更新、命中、销毁所有飞行类攻击物
* **伤害仲裁**

  * 决定最终由谁对谁造成伤害
  * 避免多系统重复扣血
* **伤害执行**

  * 调用目标实体的生命组件进行扣血
* **战斗扩展点**

  * 防御减免、暴击、属性克制等统一在此扩展

---

### 5.3 系统定位约束

* CombatResolver **不属于任何实体**
* 不继承 `Node`，但依赖外部 `Node` 进行更新调度
* 是唯一允许直接触发“扣血行为”的逻辑模块
* 单局游戏中只应存在一个实例

---

### 5.4 单例访问

```cpp
static CombatResolver* GetInstance();
```

**说明：**

* 返回全局唯一实例
* 生命周期通常与战斗场景一致
* 不允许外部手动 `new / delete`

---

### 5.5 初始化接口

```cpp
void Initialize(cocos2d::Node* battle_layer);
```

**参数说明：**

* `battle_layer`：

  * 战斗场景中的节点（通常是 Layer）
  * 用于：

    * 挂载投射物 Sprite
    * 参与 `Update(float dt)` 的调度

**行为约定：**

* 必须在战斗开始前调用
* 未初始化时不应生成投射物

---

### 5.6 帧更新接口

```cpp
void Update(float dt);
```

**职责：**

* 更新所有飞行中的投射物位置
* 检测是否命中目标或到达终点
* 处理目标已死亡 / 丢失的情况
* 在命中时触发伤害结算与特效

**调用方约定：**

* 由 Scene 或 Layer 的 `schedule` 调用
* CombatResolver 自身不管理调度注册

---

### 5.7 攻击接口

#### 5.7.1 近战攻击（即时结算）

```cpp
void ResolveMeleeAttack(
    BaseEntity* attacker,
    BaseEntity* target,
    int damage
);
```

**语义说明：**

* 不生成投射物
* 立即执行伤害结算
* 适用于：

  * 近战单位
  * 爆炸、触碰类攻击

**设计约束：**

* 实体本身不得直接调用目标的扣血接口
* 所有即时伤害必须通过 CombatResolver

---

#### 5.7.2 远程攻击（投射物）

```cpp
void SpawnProjectile(
    BaseEntity* attacker,
    BaseEntity* target,
    int damage,
    Core::ProjectileType type
);
```

**职责：**

* 创建一个投射物对象
* 根据类型加载外观与速度
* 锁定目标并开始飞行
* 在命中时结算伤害

**说明：**

* 伤害不是立即结算
* 命中判定由 CombatResolver 在 Update 中完成

---

### 5.8 投射物内部模型

```cpp
struct ProjectileObject
```

**字段语义：**

* `id`：唯一标识（用于调试或扩展）
* `sprite`：投射物的可视表现
* `target`：锁定的目标实体（弱引用）
* `damage`：携带的伤害数值
* `speed`：飞行速度（像素 / 秒）
* `type`：投射物类型（决定表现与规则）
* `last_known_pos`：

  * 目标消失时的最后位置
* `target_lost`：

  * 标记目标是否已失效（死亡 / 移除）

**行为约定：**

* 若目标死亡：

  * 投射物飞向最后已知位置
  * 到达后直接销毁（不结算伤害）

---

### 5.9 伤害执行流程

```cpp
void ApplyDamage(BaseEntity* target, int damage);
```

**职责：**

* 查找目标的生命组件（HealthComp）
* 执行扣血
* 处理死亡返回值

**设计原则：**

* CombatResolver 是唯一调用扣血的模块
* 目标实体只对“被攻击结果”负责，不对来源负责

---

### 5.10 表现与数值辅助接口

```cpp
std::string GetProjectileFilename(Core::ProjectileType type);
float GetProjectileSpeed(Core::ProjectileType type);
void PlayImpactVFX(const cocos2d::Vec2& pos, Core::ProjectileType type);
```

**说明：**

* 表现逻辑集中管理，避免散落在单位 / 建筑中
* 不影响伤害权威逻辑
* 可被替换为更复杂的特效系统

---

### 5.11 设计契约（Contract）

* CombatResolver 是**战斗结果的最终裁决者**
* 实体不得绕过 CombatResolver 直接扣血
* 投射物不具备独立伤害判定权
* Update 中必须处理目标失效的安全情况
* 不在 CombatResolver 中做 AI 决策或寻路逻辑

---

### 5.12 典型调用关系

* `Unit / Building`

  * 只负责判断“是否能攻击”
  * 调用 CombatResolver 发起攻击
* `CombatResolver`

  * 决定攻击如何执行
  * 决定伤害何时生效
* `HealthComp`

  * 仅响应伤害结果，不关心来源

这一设计确保了战斗系统的**集中控制、可维护性与可扩展性**。

## 6. EconomySystem（经济系统）

### 6.1 模块定位与职责

`EconomySystem` 是游戏中的**全局经济与人口权威系统（Authority）**，以**单例**形式存在，统一管理所有资源与人口相关规则。

其设计目标是：

* 集中管理金币、圣水与人口数据
* 统一资源的增加、消费与上限裁剪逻辑
* 避免资源规则分散在建筑或 UI 中
* 为数值平衡与规则修改提供唯一入口

---

### 6.2 核心职责

`EconomySystem` 当前承担以下职责：

* **资源管理**

  * 金币（Gold）
  * 圣水（Elixir）
* **人口管理**

  * 当前人口占用
  * 最大人口上限（由建筑决定）
* **资源流转**

  * 收集（Collect）
  * 消费（Spend）
* **上限动态计算**

  * 根据场上建筑实时调整最大容量

---

### 6.3 系统定位约束

* EconomySystem 是**纯逻辑系统**
* 不继承 `Node`，不参与渲染
* 不直接依赖 UI
* 是唯一允许修改“全局资源数值”的模块
* 单局游戏只应存在一个实例

---

### 6.4 单例访问

```cpp
static EconomySystem* GetInstance();
```

**说明：**

* 返回全局唯一实例
* 生命周期通常与游戏进程一致
* 外部模块不得自行创建或销毁实例

---

### 6.5 初始化与重置

```cpp
void Reset();
```

**职责：**

* 重置所有资源与人口数值
* 通常在：

  * 新游戏开始
  * 重新加载存档
  * 调试模式下调用

---

### 6.6 资源操作接口（Resource Operations）

#### 6.6.1 增加金币

```cpp
void AddGold(int amount);
```

**行为约定：**

* 增加当前金币数量
* 自动执行 **上限裁剪（Clamp to Max）**
* 常见来源：

  * 建筑收集
  * 任务奖励
  * 调试指令

---

#### 6.6.2 增加圣水

```cpp
void AddElixir(int amount);
```

**说明：**

* 行为与 `AddGold` 对称
* 同样自动执行上限裁剪

---

#### 6.6.3 消费金币

```cpp
bool SpendGold(int amount);
```

**返回值：**

* `true`：余额充足，成功扣除
* `false`：余额不足，未发生任何修改

**使用场景：**

* 造兵
* 建筑升级
* 科技研究

---

#### 6.6.4 消费圣水

```cpp
bool SpendElixir(int amount);
```

**说明：**

* 行为与 `SpendGold` 对称
* 不允许出现负数资源

---

#### 6.6.5 可支付性检查

```cpp
bool CanAfford(int gold_cost, int elixir_cost) const;
```

**说明：**

* 仅进行判断，不修改任何状态
* 主要供 UI 或建造逻辑使用
* 用于：

  * 按钮灰化
  * 提前校验建造条件

---

### 6.7 资源收集逻辑（Collection Logic）

```cpp
int TryCollectResource(Building* building);
```

**参数：**

* `building`：

  * 目标建筑（如金矿、圣水收集器）

**返回值：**

* 实际收集到的资源数量
* 若仓库已满，返回 `0`

**职责说明：**

* 调用建筑的资源结算接口
* 自动处理仓库上限
* 避免建筑直接修改全局资源

---

### 6.8 人口管理（Population / Housing Space）

#### 6.8.1 增加人口占用

```cpp
bool AddTroopPopulation(int housing_space);
```

**返回值：**

* `true`：人口充足，成功占用
* `false`：人口已满，操作失败

**使用场景：**

* 造兵完成时调用

---

#### 6.8.2 释放人口

```cpp
void FreeTroopPopulation(int housing_space);
```

**使用场景：**

* 单位死亡
* 单位被移除或捐赠

**说明：**

* 不做下限校验，调用方需保证逻辑正确

---

### 6.9 上限动态计算

```cpp
void RecalculateLimits(
    const cocos2d::Vector<Building*>& buildings
);
```

**职责：**

* 根据当前场上建筑重新计算：

  * 金币上限
  * 圣水上限
  * 最大人口上限

**典型触发时机：**

* 游戏加载完成
* 储金罐 / 收集器 / 兵营 建造完成
* 建筑升级完成

**设计原则：**

* 上限只由建筑决定
* EconomySystem 不主动维护建筑列表
* 上层系统负责传入当前建筑集合

---

### 6.10 Getter 接口（UI 显示用）

```cpp
int GetCurrentGold() const;
int GetMaxGold() const;

int GetCurrentElixir() const;
int GetMaxElixir() const;

int GetCurrentPopulation() const;
int GetMaxPopulation() const;
```

**说明：**

* 只读接口
* 专供 UI、调试面板、数值展示使用
* 不允许通过 Getter 修改任何状态

---

### 6.11 设计契约（Contract）

* EconomySystem 是**全局经济规则的唯一权威**
* 建筑与单位不得直接修改全局资源
* 所有资源变动必须通过 EconomySystem
* UI 只读取，不参与计算
* 上限变化必须显式触发重新计算
* 不在 EconomySystem 中处理动画、特效或 UI 行为

---

### 6.12 典型调用关系

* `Building`

  * 提供资源产出数据
  * 不直接修改金币 / 圣水
* `Unit / Training Logic`

  * 请求消费资源与人口
* `EconomySystem`

  * 判断是否允许
  * 执行最终修改
* `UI`

  * 只读取当前值与上限进行展示

该设计确保了经济系统的**一致性、可预测性与可维护性**。
