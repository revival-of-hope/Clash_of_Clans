# Engine Tile Map & Interaction Module

This module is based on **Cocos2d-x** and provides the following functional components:

*   TMX map loading and pixel-level adaptation (`MapLayer`)
*   Mouse dragging and coordinate conversion (`MouseController`)
*   Tile placement control (`TilePlacementController`)
*   Tile highlighting (`TileHighlighter`)

---

## 1. MapLayer (Map Layer)

### 1.1 Functional Description

`MapLayer` encapsulates `TMXTiledMap` and is responsible for:

*   Loading TMX maps
*   Acting as the map display node
*   Providing access to the underlying `TMXTiledMap`
*   Pixel-level map adaptation (internal processing)

---

### 1.2 Interface Description

#### Create Map

```cpp
static MapLayer* create(const std::string& tmxFile);
```

**Parameters:**

*   `tmxFile`: Path to the TMX map file

**Return Value:**

*   Returns `MapLayer*` on success, `nullptr` on failure.

---

### 1.3 Usage Example

```cpp
auto mapLayer = MapLayer::create("maps/test2.tmx");
this->addChild(mapLayer);
```

---

## 2. MouseController

### 2.1 Functional Description

`MouseController` is used to handle mouse interactions; key features include:

*   Mouse dragging, dropping, and zooming.

---

### 2.2 Interface Description

#### Enable / Disable Mouse Drag, Drop, and Zoom

```cpp
void enable();
void disable();
```

### 2.3 Usage Example

```cpp
mapLayer = MapLayer::create("maps/test2.tmx");
auto mouseController = new MouseController(mapLayer);
mouseController->enable();
```

---

## 3. TilePlacementController (Tile Placement Controller)

### 3.1 Functional Description

`TilePlacementController` is responsible for handling:

*   Triggering unit placement from the UI menu
*   Unit sprite following mouse movement
*   Aligning to the nearest tile center upon mouse release
*   Placement cancellation and state management

---

### 3.2 Interface Description

#### Start Placement (Programmatic)

```cpp
void startPlacement(
    MapLayer* map,
    const std::string& unitSpriteFile,
    const Vec2& worldPos
);
```

**Function:**
*   Enters placement mode and creates a preview sprite at `worldPos`.

#### Cancel Placement

```cpp
void cancelPlacement();
```

**Function:**
*   Cancels placement mode and clears preview state if active.

#### Query Placement State

```cpp
bool isPlacing() const;
```

**Function:**
*   Returns `true` if currently in placement mode.

#### Placement Validation

```cpp
cocos2d::Vec2 SnapToValidTile(const cocos2d::Vec2& worldPos) const;
bool CanPlaceAt(const cocos2d::Vec2& worldPos) const;
```

*   `SnapToValidTile` snaps to the nearest tile center using the map's tile size.
*   `CanPlaceAt` returns `false` when the snapped tile is marked `blocked=true` (or `walkable=false`) on the `collision` layer.
*   Default tile size follows Core grid constants (`kTileWidth`/`kTileHeight` = 64). Override only if TMX metadata and Core co
nstants match.

#### Bind Menu Icon (Recommended)

```cpp
void bindMenuIcon(
    Sprite* menuIcon,
    MapLayer* map,
    const std::string& unitSpriteFile
);
```

**Function:**
*   Clicking the menu icon enters placement mode.

---

### 3.3 Usage Example

```cpp
// Create object
tilePlacement = new TilePlacementController(this);
// Create menu
auto menuIcon = Sprite::create("HelloWorld.png");
menuIcon->setScale(0.4f);
menuIcon->setPosition(Vec2(80, 80));
this->addChild(menuIcon, 10);
// Direct usage
tilePlacement->bindMenuIcon(
    menuIcon,
    mapLayer,
    "Troops_Icon/Archer.png"
);
```

## 4. TileHighlighter.h (Highlighting only within Tile Map)

### Usage

```cpp
    // Tile Highlighting (Visualize the tile pointed at by the mouse)
    mapLayer = MapLayer::create("maps/test2.tmx");
    tileHighlighter = new TileHighlighter(mapLayer->getMap());
    tileHighlighter->enable();
    tileHighlighter->disable(); // Remove highlight
```

## 5. Test Scene (EngineTestScene)

This project provides a complete test scene to verify the functionality of all modules.

---

### 5.1 Startup Method

Add the following in `AppDelegate.cpp`:

```cpp
auto scene = EngineTestScene::createScene();
Director::getInstance()->runWithScene(scene);
```

---

### 5.2 Example Code

```cpp
bool EngineTestScene::init()
{
    if (!Scene::init())
        return false;

    // Create map
    mapLayer = MapLayer::create("maps/test2.tmx");
    this->addChild(mapLayer);

    // Tile highlighting
    tileHighlighter = new TileHighlighter(mapLayer->getMap());
    tileHighlighter->enable();

    // Mouse drag control
    mouseController = new MouseController(mapLayer);
    mouseController->enable();

    // Placement controller
    tilePlacement = new TilePlacementController(this);

    // Menu icon
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

## 7. Behavior Contract

This module follows the **Behavior Contract** below to constrain lifecycles, ownership, coordinate systems, and state behaviors, preventing module misuse.

---

### 7.1 Lifecycle Specifications

| Object | Lifecycle | Description |
|----|----|----|
| `MapLayer` | Scene child node | Managed automatically by Cocos2d-x |
| `MouseController` | Scene lifecycle | Must manually `enable / disable` |
| `TilePlacementController` | Scene lifecycle | Logic object, does not participate in rendering |
| `TileHighlighter` | Scene lifecycle | Can `enable / disable` at any time |

---

### 7.2 Enable / Disable Specifications (Mandatory)

| Module | Rule |
|----|----|
| `MouseController` | Responds to input only after `enable()` |
| `TileHighlighter` | `disable()` must completely remove highlighting |
| `TilePlacementController` | `cancelPlacement()` must clear all states |

---

### 7.3 Ownership Rules

#### Scene Owns All Controllers

```text
Scene
 ├── MapLayer (Node)
 ├── MouseController (Logic Object)
 ├── TilePlacementController (Logic Object)
 └── TileHighlighter (Logic Object)
```
