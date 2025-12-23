# Engine Tile Map & Interaction Module

本模块基于 **Cocos2d-x**，提供了以下功能组件：

* TMX 地图加载与像素级适配（`MapLayer`）
* 鼠标拖拽与坐标转换（`MouseController`）
* 瓦片放置控制（`TilePlacementController`）


## 1. MapLayer（地图层）

### 1.1 模块定位与职责

`MapLayer` 是对 `TMXTiledMap` 的一层**功能封装与抽象**，用于作为游戏中的「地图显示与交互基础层」。

它的设计目标是：

* 隐藏 `TMXTiledMap` 的初始化与显示细节
* 统一管理与地图强相关的功能组件
* 对外提供**简洁、稳定、可扩展**的接口
* 避免外部模块直接操作底层地图对象

`MapLayer` 本身是一个 `Node`，可以直接添加到 `Scene` 或其他节点中。

---

### 1.2 核心功能

`MapLayer` 当前负责以下功能：

* **加载并管理 TMX 地图**
* **作为地图的显示节点**
* **对底层 `TMXTiledMap` 提供只读访问**
* **地图的像素级适配与缩放（内部处理）**
* **内建瓦片高亮功能（默认开启，可随时开关）**

---

### 1.3 生命周期与所有权说明

* `TMXTiledMap` 的生命周期 **完全由 `MapLayer` 管理**
* 地图高亮组件（`TileHighlighter`）同样由 `MapLayer` 创建、启用与销毁
* 外部模块 **不需要，也不应该**：

  * `new / delete` 地图相关组件
  * 手动绑定或解绑地图高亮逻辑

当 `MapLayer` 被销毁时，其内部所有地图相关组件会一并释放。

---

### 1.4 接口说明

### 1.4.1 创建地图

```cpp
static MapLayer* create(const std::string& tmxFile);
```

**参数：**

* `tmxFile`
  TMX 地图文件路径（相对于资源目录）

**返回值：**

* 成功：返回已初始化的 `MapLayer*`
* 失败：返回 `nullptr`

**说明：**

* 内部会加载 TMX 文件并创建 `TMXTiledMap`
* 地图会自动进行像素级适配
* 地图高亮功能会在创建时**默认开启**

---

### 1.4.2 获取底层 TMXTiledMap

```cpp
TMXTiledMap* getMap() const;
```

**返回值：**

* 返回底层 `TMXTiledMap` 指针

**使用建议：**

* 仅用于：

  * 查询地图尺寸
  * 获取瓦片大小
  * 访问图层数据（如障碍、地形信息）
* **不建议**外部修改其缩放、位置或生命周期

---

### 1.4.3 地图高亮控制

```cpp
void setHighlightEnabled(bool enabled);
```

**参数：**

* `enabled`

  * `true`：开启瓦片高亮
  * `false`：关闭瓦片高亮

**说明：**

* 高亮功能用于显示当前鼠标指向的瓦片
* 高亮逻辑由 `MapLayer` 内部管理
* 可在运行时随时开启或关闭
* 多次调用是安全的

**默认行为：**

* 地图创建完成后，高亮功能默认开启

---

## 1.5 使用示例

### 1.5.1 创建并显示地图

```cpp
auto mapLayer = MapLayer::create("maps/test2.tmx");
this->addChild(mapLayer);
```

---

### 1.5.2 关闭 / 开启地图高亮

```cpp
// 关闭瓦片高亮（例如进入纯游戏模式）
mapLayer->setHighlightEnabled(false);

// 再次开启（例如进入编辑 / 放置模式）
mapLayer->setHighlightEnabled(true);
```

---

## 1.6 设计说明（给维护者）

* `MapLayer` 对外只暴露**必要接口**
* 地图高亮作为「地图能力」而非「控制器能力」
* UI、放置控制、鼠标逻辑等模块：

  * **不直接依赖 `TileHighlighter`**
  * 只与 `MapLayer` 交互

这种设计可以确保：

* UI 切换不会影响地图内部状态
* 地图相关逻辑集中管理，便于维护
* 后续可以无侵入地扩展地图能力（如阻挡显示、路径预览等）

---
很好，这里我会**严格根据你给出的 `MouseController.h`** 来完善项目说明文档，
不“脑补不存在的功能”，也不改变你现在的设计定位，
目标是：**别人只看文档 + 头文件，就能正确使用、不踩坑**。

---

## 2. MouseController（鼠标控制器）

### 2.1 模块定位与职责

`MouseController` 是一个**轻量级的输入控制组件**，用于处理与鼠标相关的交互行为。
它并不直接参与渲染，而是通过事件监听的方式，对目标节点（通常是地图层）进行操作。

该控制器通常用于：

* 地图的鼠标拖拽移动
* 地图位置的约束（防止拖出屏幕）
* 提供基于鼠标位置的辅助查询能力

`MouseController` 本身**不继承自 `Node`**，需要由外部模块负责创建与销毁。

---

### 2.2 核心功能

`MouseController` 当前提供以下能力：

* **鼠标拖拽控制**

  * 按下并拖动鼠标以移动目标节点
* **拖拽边界限制**

  * 自动限制目标节点不被拖出可视区域
* **鼠标位置回调**

  * 在鼠标移动时向外部通知当前鼠标位置
* **瓦片坐标辅助查询**

  * 当控制目标为 `TMXTiledMap` 时，可根据屏幕坐标获取对应瓦片坐标

---

### 2.3 生命周期与使用约定

* `MouseController` 需要一个外部节点作为控制目标（`target`）
* 控制器不会自动启用，必须显式调用 `enable()`
* 禁用或销毁前，应先调用 `disable()`（推荐）
* 控制器的生命周期通常与 Scene 或其所属系统一致

---

### 2.4 接口说明

### 2.4.1 构造函数

```cpp
MouseController(cocos2d::Node* target);
```

**参数：**

* `target`
  鼠标操作的目标节点
  通常为 `MapLayer` 或 `TMXTiledMap`，也可以是其他 `Node`

**说明：**

* `MouseController` 会对该节点添加鼠标事件监听
* 所有拖拽与位置计算均基于该节点

---

### 2.4.2 启用 / 禁用鼠标控制

```cpp
void enable();
void disable();
```

**说明：**

* `enable()`

  * 注册鼠标事件监听器
  * 开始响应鼠标拖拽与移动
* `disable()`

  * 移除鼠标事件监听器
  * 停止所有鼠标交互行为

**注意事项：**

* 重复调用 `enable()` / `disable()` 应是安全的（推荐保持幂等）
* 在 Scene 切换或控制器不再使用时，建议调用 `disable()`

---

### 2.4.3 获取鼠标对应的瓦片坐标

```cpp
cocos2d::Vec2 getTilePosAtScreenPos(const cocos2d::Vec2& screenPos);
```

**参数：**

* `screenPos`
  屏幕坐标（通常来自鼠标事件）

**返回值：**

* 返回瓦片坐标（`Vec2(tileX, tileY)`）

**使用前提：**

* `target` 必须是 `TMXTiledMap` 或包含 `TMXTiledMap` 的节点
* 若目标节点不支持瓦片转换，返回值未定义（由调用者自行保证）

**典型用途：**

* 编辑器模式下的瓦片选中
* 调试或工具型功能

---

### 2.4.4 鼠标移动回调

```cpp
std::function<void(const cocos2d::Vec2&)> onMouseMoveCallback;
```

**说明：**

* 当鼠标在屏幕上移动时触发
* 回调参数为当前鼠标的屏幕坐标
* 可用于：

  * 显示坐标信息
  * 驱动高亮或提示逻辑
  * 与其他控制器联动

**示例：**

```cpp
mouseController->onMouseMoveCallback = [](const Vec2& pos) {
    CCLOG("Mouse at: %.1f, %.1f", pos.x, pos.y);
};
```

---

## 2.5 使用示例

### 2.5.1 基本用法（控制地图拖拽）

```cpp
// 在类中声明
// MouseController* mouseController = nullptr;

auto mapLayer = MapLayer::create("maps/test2.tmx");
this->addChild(mapLayer);

mouseController = new MouseController(mapLayer);
mouseController->enable();
```

---

### 2.5.2 配合其他系统使用

```cpp
mouseController->onMouseMoveCallback = [&](const Vec2& screenPos) {
    // 可在此联动 TileHighlighter / 调试显示等功能
};
```

---

## 2.6 设计说明（给维护者）

* `MouseController` 专注于 **“输入 → 节点行为”**
* 不直接依赖具体游戏逻辑
* 不负责 UI、单位放置或高亮显示
* 可被多个系统复用（地图、编辑器、调试工具）

这种设计使得：

* 鼠标逻辑集中管理
* 不同功能模块之间解耦
* 后续可扩展滚轮缩放、快捷键等输入行为

---

如果你愿意，下一步我可以帮你继续补：

* **TilePlacementController 的完整模块说明**
* **三个 Controller 之间的协作流程图**
* **编辑器模式 vs 游戏模式的输入控制规范**

你现在这个项目说明已经很接近「真正引擎文档」了。

好，这里我**完全以你给出的 `TilePlacementController.h` 为准**，
对 **第 3 章说明文档进行一次“工程级重写”**，做到：

* 不虚构不存在的功能
* 清楚区分 **对外接口 / 内部逻辑 / 使用约束**
* 明确生命周期、状态机和安全使用方式
* 让第一次接手项目的人也能“照着文档用对”

---

## 3. TilePlacementController（瓦片放置控制器）

### 3.1 模块定位与职责

`TilePlacementController` 是一个**交互状态型控制器**，用于管理“单位 / 建筑”等对象在地图上的放置流程。

它主要负责：

* 从 UI 层触发放置流程
* 创建并管理“跟随鼠标移动”的拖拽精灵
* 将世界坐标转换为地图瓦片中心坐标
* 在鼠标释放时完成放置
* 维护放置状态，并支持随时取消

该控制器**不负责单位逻辑本身**，仅负责“放到哪里、什么时候放”。

---

### 3.2 设计特性概览

* ❌ 不继承 `Node`，不参与渲染
* ✅ 通过事件监听驱动行为
* ✅ 明确的放置状态（placing / idle）
* ✅ UI 层与地图层解耦
* ⚠ 同一时间只支持一次放置流程

---

### 3.3 生命周期与使用约定

* 控制器通常在 `Scene` 初始化时创建
* 构造时需传入一个 **事件所有者节点**（通常是 Scene 或 UI Root）
* 鼠标监听在构造时初始化，在析构时释放
* 控制器不自动销毁，需由外部 `delete`

---

## 3.4 接口说明

### 3.4.1 构造 / 析构

```cpp
TilePlacementController(Node* owner);
~TilePlacementController();
```

**参数：**

* `owner`
  事件监听器的归属节点（通常为 Scene 或 UI 根节点）

**说明：**

* 所有鼠标事件监听均挂载在 `owner` 上
* `owner` 的生命周期必须 ≥ `TilePlacementController`

---

### 3.4.2 绑定菜单图标（快速接口，测试用）

```cpp
void bindMenuIcon(
    Sprite* menuIcon,
    MapLayer* map,
    const std::string& unitSpriteFile
);
```

**功能：**

* 为指定 UI 图标绑定点击事件
* 点击图标后自动进入放置模式

**参数说明：**

* `menuIcon`
  UI 菜单中的图标精灵
* `map`
  目标地图层
* `unitSpriteFile`
  放置时使用的单位精灵资源路径

**注意事项（重要）：**

⚠ **该接口为封装接口，仅推荐测试或简单场景使用**

* 多次绑定不同图标时，可能出现监听覆盖或状态冲突
* 不适合复杂 UI 或正式编辑器逻辑
* 内部实际调用的是 `startPlacement()`

---

### 3.4.3 启动放置流程（推荐方式）

```cpp
void startPlacement(
    MapLayer* map,
    const std::string& unitSpriteFile,
    const Vec2& worldPos
);
```

**功能：**

* 进入“放置模式”
* 创建一个拖拽精灵
* 从指定世界坐标开始跟随鼠标移动

**参数说明：**

* `map`
  当前游戏地图
* `unitSpriteFile`
  拖拽精灵的资源路径
* `worldPos`
  初始鼠标世界坐标（通常来自 `Touch` 或 `MouseEvent`）

**行为说明：**

* 若当前已在放置状态，会先取消旧的放置
* 拖拽精灵会实时跟随鼠标
* 松开鼠标后自动对齐到最近瓦片中心

---

### 3.4.4 取消放置

```cpp
void cancelPlacement();
```

**功能：**

* 立即退出放置模式
* 移除拖拽精灵
* 重置内部状态

**典型使用场景：**

* UI 关闭
* 切换模式
* 玩家右键取消（可自行扩展）

---

### 3.4.5 查询当前状态

```cpp
bool isPlacing() const;
```

**返回值：**

* `true`：当前处于放置模式
* `false`：当前空闲

---

## 3.5 使用示例

### 3.5.1 推荐用法（UI 层显式控制）

```cpp
// 头文件中声明
// TilePlacementController* tilePlacement = nullptr;

// 创建控制器
tilePlacement = new TilePlacementController(this);

// 创建 UI 图标
auto icon = Sprite::create("unitA.png");
icon->setPosition(Vec2(80, 80));
this->addChild(icon, 10);

// 手动绑定触发逻辑
auto listener = EventListenerTouchOneByOne::create();
listener->onTouchBegan = [this, icon](Touch* t, Event*)
{
    if (icon->getBoundingBox().containsPoint(t->getLocation()))
    {
        tilePlacement->startPlacement(
            mapLayer,
            "unitA.png",
            t->getLocation()
        );
        return true;
    }
    return false;
};

icon->getEventDispatcher()
    ->addEventListenerWithSceneGraphPriority(listener, icon);
```

---

#### 3.5.2 快速测试用法（不推荐正式项目）

```cpp
tilePlacement->bindMenuIcon(
    menuIcon,
    mapLayer,
    "Troops_Icon/Archer.png"
);
```

---

## 3.6 内部逻辑说明（给维护者）

### 内部关键成员

```cpp
Node* _owner;                // 事件归属节点
MapLayer* _gameMap;          // 当前地图层
TMXTiledMap* _map;           // 原始 TMX 地图

Sprite* _dragSprite;         // 跟随鼠标的拖拽精灵
bool _placing;               // 是否处于放置状态

EventListenerMouse* _mouseListener;
```

### 内部流程概览

1. `startPlacement()`

   * 初始化地图引用
   * 创建拖拽精灵
   * 设置 `_placing = true`
2. 鼠标移动

   * 更新拖拽精灵位置
3. 鼠标释放

   * 世界坐标 → 瓦片中心坐标
   * 精灵对齐
   * 放置完成，退出放置模式
4. `cancelPlacement()`

   * 清理精灵
   * 重置状态

---

## 3.7 模块边界说明

`TilePlacementController` **不会负责**：

* 单位逻辑（血量、阵营、AI）
* 放置是否合法（资源 / 冷却）
* 地图规则判断（可否建造）

这些应由更高层系统处理。


## 4. 测试场景（EngineTestScene）

本项目提供了一个完整测试场景，用于验证所有模块功能。

---

### 4.1 启动方式

在 `AppDelegate.cpp` 中添加：

```cpp
auto scene = EngineTestScene::createScene();
Director::getInstance()->runWithScene(scene);
```

---

### 4.2 示例代码

```cpp
bool EngineTestScene::init()
{
    if (!Scene::init())
        return false;

    // 创建地图
    mapLayer = MapLayer::create("maps/test2.tmx");
    this->addChild(mapLayer);

    // Tile 高亮
    tileHighlighter = new TileHighlighter(mapLayer->getMap());
    tileHighlighter->enable();

    // 鼠标拖拽控制
    mouseController = new MouseController(mapLayer);
    mouseController->enable();

    // 放置控制器
    tilePlacement = new TilePlacementController(this);

    // 菜单图标
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
```

