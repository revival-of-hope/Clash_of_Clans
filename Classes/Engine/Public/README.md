

# Engine Tile Map & Interaction Module

本模块基于 **Cocos2d-x**，提供了以下功能组件：

* TMX 地图加载与像素级适配（`MapLayer`）
* 鼠标拖拽与坐标转换（`MouseController`）
* 瓦片放置控制（`TilePlacementController`）
  -（可选）瓦片高亮显示（`TileHighlighter`）
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

#### 获取 TMX 地图对象

```cpp
TMXTiledMap* getMap() const;
```

用于传递给其他系统（如高亮、放置、鼠标控制）。

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

* 鼠标拖拽地图
* 获取鼠标对应的瓦片坐标
* 提供鼠标移动回调接口

---

### 2.2 接口说明


#### 启用 / 禁用鼠标监听

```cpp
void enable();
void disable();
```

---

#### 获取鼠标对应的瓦片坐标

```cpp
cocos2d::Vec2 getTilePosAtScreenPos(const cocos2d::Vec2& screenPos);
```

> ⚠️ 仅当 `target` 为 `TMXTiledMap` 时有效

---

#### 鼠标移动回调

```cpp
std::function<void(const cocos2d::Vec2&)> onMouseMoveCallback;
```

---

### 2.3 使用示例

```cpp
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



**参数：**

* `owner`：通常为 Scene 或 UI 根节点

---

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

#### 手动开始放置

```cpp
void startPlacement(
    MapLayer* map,
    const std::string& unitSpriteFile,
    const Vec2& worldPos
);
```

---

#### 取消放置

```cpp
void cancelPlacement();
```

---

#### 是否正在放置

```cpp
bool isPlacing() const;
```

---

### 3.3 使用示例

```cpp
tilePlacement = new TilePlacementController(this);

auto menuIcon = Sprite::create("HelloWorld.png");
menuIcon->setScale(0.4f);
menuIcon->setPosition(Vec2(80, 80));
this->addChild(menuIcon, 10);

tilePlacement->bindMenuIcon(
    menuIcon,
    mapLayer,
    "Troops_Icon/Archer.png"
);
```

---

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