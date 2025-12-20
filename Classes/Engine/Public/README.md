# Engine Tile Map & Interaction Module

本模块基于 **Cocos2d-x**，提供了以下功能组件：

* TMX 地图加载与像素级适配（`MapLayer`）
* 鼠标拖拽与坐标转换（`MouseController`）
* 瓦片放置控制（`TilePlacementController`）
* 瓦片高亮显示（`TileHighlighter`）
---
## 1. MapLayer（地图层）

### 1.1 功能说明

`MapLayer` 封装了 `TMXTiledMap`，负责：

* 加载 TMX 地图
* 作为地图显示节点
* 提供对底层 `TMXTiledMap` 的访问
* 地图像素级适配（内部处理）

---

### 1.2 接口说明

#### 创建地图

```cpp
static MapLayer* create(const std::string& tmxFile);
```

**参数：**

* `tmxFile`：TMX 地图文件路径

**返回值：**

* 成功返回 `MapLayer*`，失败返回 `nullptr`

---

### 1.3 使用示例

```cpp
auto mapLayer = MapLayer::create("maps/test2.tmx");
this->addChild(mapLayer);
```

---

## 2. MouseController（鼠标控制器）

### 2.1 功能说明

`MouseController` 用于处理鼠标交互，主要功能包括：

* 鼠标拖放和放缩
---

### 2.2 接口说明


#### 启用 / 禁用鼠标拖放和放缩

```cpp
void enable();
void disable();
```



### 2.3 使用示例

```cpp
mapLayer = MapLayer::create("maps/test2.tmx");
auto mouseController = new MouseController(mapLayer);
mouseController->enable();
```

---

## 3. TilePlacementController（瓦片放置控制器）

### 3.1 功能说明

`TilePlacementController` 负责处理：

* 从 UI 菜单触发单位放置
* 单位精灵跟随鼠标移动
* 松开鼠标后对齐到最近瓦片中心
* 放置取消与状态管理

---

### 3.2 接口说明


#### 绑定菜单图标（推荐方式）

```cpp
void bindMenuIcon(
    Sprite* menuIcon,
    MapLayer* map,
    const std::string& unitSpriteFile
);
```

**功能：**
* 点击菜单图标后进入放置模式
---

### 3.3 使用示例

```cpp
//创建对象
tilePlacement = new TilePlacementController(this);
//创建菜单
auto menuIcon = Sprite::create("HelloWorld.png");
menuIcon->setScale(0.4f);
menuIcon->setPosition(Vec2(80, 80));
this->addChild(menuIcon, 10);
//直接使用
tilePlacement->bindMenuIcon(
    menuIcon,
    mapLayer,
    "Troops_Icon/Archer.png"
);
```
## 4. TileHighlighter.h(只在瓦片地图里高亮)

### 使用方法
```cpp
    // Tile 高亮（可视化鼠标所指瓦片）
    mapLayer = MapLayer::create("maps/test2.tmx");
    tileHighlighter = new TileHighlighter(mapLayer->getMap());
    tileHighlighter->enable();
    tileHighlighter->disable();//去除高亮
``` 
## 5. 测试场景（EngineTestScene）

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

## 7. 行为规范约束（Behavior Contracts）

本模块遵循以下 **行为契约（Behavior Contract）**，用于约束生命周期、所有权、坐标系统与状态行为，防止模块被误用。

---

### 7.1 生命周期规范

| 对象 | 生命周期 | 说明 |
|----|----|----|
| `MapLayer` | Scene 子节点 | 由 Cocos2d-x 自动管理 |
| `MouseController` | Scene 生命周期 | 必须手动 `enable / disable` |
| `TilePlacementController` | Scene 生命周期 | 逻辑对象，不参与渲染 |
| `TileHighlighter` | Scene 生命周期 | 可随时 `enable / disable` |

---

### 7.2 enable / disable 规范（强制）

| 模块 | 规则 |
|----|----|
| `MouseController` | 仅在 `enable()` 后才响应输入 |
| `TileHighlighter` | `disable()` 必须完全移除高亮 |
| `TilePlacementController` | `cancelPlacement()` 必须清理全部状态 |

---

### 7.3 所有权规范（Ownership Rules）

#### Scene 拥有所有 Controller

```text
Scene
 ├── MapLayer (Node)
 ├── MouseController (逻辑对象)
 ├── TilePlacementController (逻辑对象)
 └── TileHighlighter (逻辑对象)
