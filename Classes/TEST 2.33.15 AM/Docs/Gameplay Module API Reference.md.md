# **Gameplay Module Development Manual & Behavior Contracts (V2.0)**

Version: 2.0 (Release Candidate)
Owner: Dev B (Gameplay)
Audience: Dev C (UI/App), Dev A (Engine)
Last Updated: 2025-12-19

## **1. Physical Architecture & Public Headers**

To achieve "physical isolation" between modules, all implementation details (.cpp) of the Gameplay module are hidden from the outside. External modules (UI, Engine) are **only allowed** to reference header files located in the `Classes/Contracts/Gameplay` directory.

**Include Path Standard (Dev C):**

```cpp
// ✅ Correct: Include Contracts interfaces
#include "Classes/Contracts/Gameplay/Unit.h"
#include "Classes/Contracts/Gameplay/EconomySystem.h"

// ❌ Error: Direct include of private implementation (Strictly forbidden to reference cpp across modules; causes linker errors)
#include "../../Gameplay/Entities/Unit.cpp"
```

## **2. Behavior Contracts**

When calling Gameplay module interfaces, you **must** adhere to the following contracts. This is the prerequisite for ensuring game logic correctness and memory safety.

### **2.1 Coordinate System Contracts**

The game logic and rendering layers mix two types of coordinate systems. Please distinguish between them carefully:

1.  **Logical Grid (Grid Coordinate):**
    *   **Type:** int
    *   **Range:** 0 <= x < 40, 0 <= y < 40
    *   **Origin:** Map logical **Bottom-Left** (0,0)
    *   **Usage:** Building placement positions, A* pathfinding, GameStructs data storage.
2.  **World Pixel (Pixel Coordinate):**
    *   **Type:** float (Cocos2d Standard)
    *   **Origin:** Screen/Layer Bottom-Left (0,0)
    *   **Conversion Formula:**
        *   Pixel X = Grid X * 64 + 32 (Center aligned)
        *   Pixel Y = Grid Y * 64 + 32
    *   **Usage:** Unit movement, `cocos2d::Node::setPosition`, Projectile flight.

**⚠️ Constraint:** The AnchorPoint for all Units and Buildings is locked at (0.5, 0.5).

### **2.2 Lifecycle & Ownership Contracts**

*   **Creation (Factory Pattern):**
    *   Forbidden to use `new`. Must use the static factory `create()`.
    *   **Return Value:** `autorelease` object.
    *   **Obligation:** The caller (Scene/Layer) must call `addChild()` or `retain()` **immediately** after creation; otherwise, the object will be automatically destroyed at the end of the current frame.
*   **Destruction:**
    *   **Forbidden `delete`:** Strictly forbidden to manually `delete` entity pointers.
    *   **External Active Destruction:** If the UI needs to remove an entity (e.g., cancelling construction), please call `entity->MarkForDestruction()` or `entity->removeFromParent()`.
    *   **Internal Passive Destruction:** When HP <= 0, the Gameplay module will internally play the death animation and remove the node.
    *   **Dangling Pointer Warning:** The UI layer must not hold `Unit*` pointers for long periods. It is recommended to check validity via `IsMarkedForDestruction()` every frame.

### **2.3 Timing & Sequencing Contracts**

*   **Initialization Timing:**
    *   `CombatResolver::Initialize(Layer*)` must be called during the scene `init()` phase, and before any units are spawned.
    *   `EconomySystem::RecalculateLimits(...)` must be called after **every** building construction/upgrade is completed.
*   **Frame Update:**
    *   The UI layer **does not need** to manually drive entity updates. They inherit from `Node` and will be automatically driven by the Cocos scheduler after `addChild`.
    *   `CombatResolver` will automatically take over the `battle_layer` scheduler.

### **2.4 Events & Data Contracts**

*   **Resource Data:** Is **Read-Only**.
    *   ❌ Forbidden: `economy->current_gold_ += 100;`
    *   ✅ Allowed: `economy->AddGold(100);` or `economy->SpendGold(50);`
*   **UI Refresh Mechanism:**
    *   Currently uses **Polling** mode. The UI layer should query the status of `EconomySystem` and `HealthComp` inside `update(dt)` to refresh the interface.

## **3. Substantial API Reference**

### **3.1 EconomySystem (Economy System Singleton)**

**Header:** `Classes/Contracts/Gameplay/EconomySystem.h`

| Interface Method | Parameters | Description | Contract Constraint |
| :---- | :---- | :---- | :---- |
| GetInstance() | None | Get singleton instance | Global unique entry point |
| Reset() | None | Reset all resources | Must be called when a new game starts |
| GetCurrentGold() | None | Get current gold | Return value >= 0 |
| GetMaxGold() | None | Get max gold capacity | Determined by gold storage buildings |
| GetCurrentElixir() | None | Get current elixir | Return value >= 0 |
| GetMaxElixir() | None | Get max elixir capacity | Determined by elixir storage buildings |
| GetCurrentPopulation() | None | Get current population | Increases with troop training, decreases with death/donation |
| GetMaxPopulation() | None | Get max population capacity | Determined by army camps |
| AddGold() | int amount | Add gold to storage | Automatically clamps to max limit |
| AddElixir() | int amount | Add elixir to storage | Automatically clamps to max limit |
| SpendGold() | int amount | Spend Gold | Returns `false` and does not deduct if balance is insufficient |
| SpendElixir() | int amount | Spend Elixir | Returns `false` and does not deduct if balance is insufficient |
| TryCollectResource() | Building* | Collect Resources | Automatically calculates remaining storage space, **will not overflow**, returns actual collected amount |
| RecalculateLimits() | Vector<Building*> | Recalculate Limits | **Must pass all buildings in the scene**. Must be called upon construction completion to update population/resource caps. |
| AddTroopPopulation() | int space | Occupy Population | Must check return value before creating troops; `true` indicates sufficient population |

### **3.2 CombatResolver (Combat Arbitration Singleton)**

**Header:** `Classes/Contracts/Gameplay/CombatResolver.h`

| Interface Method | Parameters | Description | Contract Constraint |
| :---- | :---- | :---- | :---- |
| Initialize() | Node* layer | Initialize | **Must be called in Scene::init**. The passed `layer` is used to host projectiles. |
| ResolveMeleeAttack() | attacker, target, dmg | Melee Settlement | Deals damage instantly. Special: Wall Breakers deal 40x damage to walls and commit suicide. |
| SpawnProjectile() | attacker, target, dmg, type | Ranged Attack | Spawns a projectile. Damage is settled **when** the projectile hits. |

### **3.3 BaseEntity (Common Entity Base)**

**Header:** `Classes/Contracts/Gameplay/BaseEntity.h`

| Interface Method | Parameters | Description |
| :---- | :---- | :---- |
| init() | None | Initialize the entity. |
| onEnter() | None | Called when the entity enters the scene. |
| onExit() | None | Called when the entity exits the scene. |
| update() | float dt | Per-frame update callback. |
| set_instance_id() | int id | Set unique instance ID. |
| get_instance_id() | None | Get unique instance ID. |
| set_owner_id() | int id | Set owner/faction ID. |
| get_owner_id() | None | Get owner/faction ID. |
| IsAlly() | BaseEntity* other | Returns true if owner_id matches. |
| MarkForDestruction() | None | Marks entity for removal. |
| IsMarkedForDestruction() | None | Returns true if entity is marked for destruction. |
| GetCenterPosition() | None | Returns world-space center position. |
| GetAllEntities() | None | Returns global list of active entities. |

### **3.4 Unit (Troop Entity)**

**Header:** `Classes/Contracts/Gameplay/Unit.h`

**Inheritance:** `BaseEntity`

| Interface Method | Parameters | Description |
| :---- | :---- | :---- |
| create(...) | TroopType, level, owner_id | **Static Factory**. Returns `Unit*`. owner_id: 0=Player, 1=Enemy. |
| GetTroopType() | None | Get troop enumeration type. |
| GetGeneralType() | None | Get general troop type (Ground/Air). |
| GetState() | None | Get the current animation state. |
| GetLevel() | None | Get troop level. |
| GetDamage() | None | Get single-hit damage value. |
| GetOccupiedRect() | None | Get world-space hit-test bounds (pixels) for UI selection. |
| CanAttack(...) | GeneralType target_type | Query if target can be attacked (e.g., Barbarians cannot hit kAir). |
| SetFacing(...) | Facing | Set facing direction (affects Sprite flipping). |

### **3.5 Building (Building Entity)**

**Header:** `Classes/Contracts/Gameplay/Building.h`

**Inheritance:** `BaseEntity`

| Interface Method | Parameters | Description |
| :---- | :---- | :---- |
| create(...) | BuildingType, level, owner_id | **Static Factory**. Returns `Building*`. |
| StartConstruction(...) | float duration | Start construction. During this, `IsConstructing()` is true, functions are paused. |
| CollectResource(...) | int max | **Low-level Interface**. UI is recommended to use `EconomySystem::TryCollectResource`. |
| GetOccupiedRect() | None | Get the occupied rectangle in world coordinates (Pixels). Used for collision/click detection. |
| GetLevel() | None | Get building level. |
| GetBuildingType() | None | Get building enumeration type. |
| GetStoredResource() | None | Get stored resource amount in collectors/mines. |
| IsConstructing() | None | Returns true if building is under construction/upgrade. |

### **3.6 HealthComp (Health Component)**

**Header:** `Classes/Contracts/Gameplay/HealthComp.h`

| Interface Method | Parameters | Description |
| :---- | :---- | :---- |
| GetHealthPercentage() | None | Get current health percentage in range [0.0, 1.0]. |
| GetCurrentHP() | None | Get current HP (absolute). |
| GetMaxHP() | None | Get max HP (absolute). |
| IsDead() | None | Returns true if HP <= 0. |
| SetHealthBarOffset() | cocos2d::Vec2 offset | Set the health bar offset relative to the parent node. |
| TakeDamage() | int amount | Apply damage; returns true if this damage caused death. |
| InitStats() | int max_hp | Initialize max HP; must be called after creation. |

## **4. Standard Integration Example**

The following code demonstrates how to correctly integrate the Gameplay module into a blank scene. This code complies with all contracts and is directly compilable.

**Scene Functionality:**

1.  Initialize combat and economy systems.
2.  Place Town Hall (Establish logical grid).
3.  Place Cannon (Demonstrate construction flow).
4.  Spawn Barbarian (Demonstrate unit deployment and combat).
5.  UI Polling for resource refresh.

### **4.1 Header File (GameplayIntegrationScene.h)**

```cpp
#ifndef INTEGRATION_TEST_SCENE_H_
#define INTEGRATION_TEST_SCENE_H_

#include "cocos2d.h"

// [Contract 1] Only include Contracts headers
#include "Classes/Contracts/Gameplay/Unit.h"
#include "Classes/Contracts/Gameplay/Building.h"
#include "Classes/Contracts/Gameplay/EconomySystem.h"
#include "Classes/Contracts/Gameplay/CombatResolver.h"

class GameplayIntegrationScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;
    
    // Simulate user actions
    void OnPlaceTownHall();
    void OnBuildCannon();
    void OnSpawnBarbarian();

    CREATE_FUNC(GameplayIntegrationScene);

private:
    cocos2d::Node* game_layer_ = nullptr; // Game Layer (ZOrder 0)
    cocos2d::Label* resource_label_ = nullptr; // UI Layer (ZOrder 100)
    
    // Simple helper: Grid to Pixel
    cocos2d::Vec2 GridToPixel(int x, int y) {
        return cocos2d::Vec2(x * 64 + 32, y * 64 + 32);
    }
};

#endif // INTEGRATION_TEST_SCENE_H_
```

### **4.2 Implementation File (GameplayIntegrationScene.cpp)**

```cpp
#include "GameplayIntegrationScene.h"

USING_NS_CC;

Scene* GameplayIntegrationScene::createScene() {
    return GameplayIntegrationScene::create();
}

bool GameplayIntegrationScene::init() {
    if (!Scene::init()) return false;

    // 1. Layered Architecture: Separation of Logic and UI layers
    game_layer_ = Node::create();
    this->addChild(game_layer_, 0);

    auto ui_layer = Node::create();
    this->addChild(ui_layer, 100);

    // 2. [Contract 2.3] Initialize core systems
    // Must pass game_layer_ to host projectiles
    CombatResolver::GetInstance()->Initialize(game_layer_);
    // Reset Economy
    EconomySystem::GetInstance()->Reset(); 

    // 3. Build simple UI
    resource_label_ = Label::createWithSystemFont("Gold: 0/0 | Pop: 0/0", "Arial", 24);
    resource_label_->setPosition(Vec2(400, 500));
    ui_layer->addChild(resource_label_);

    // 4. Execute simulation flow
    OnPlaceTownHall();    // Place Town Hall
    OnBuildCannon();      // Build Cannon
    OnSpawnBarbarian();   // Spawn Barbarian

    this->scheduleUpdate();
    return true;
}

void GameplayIntegrationScene::OnPlaceTownHall() {
    // [Contract 2.2] Create building (Town Hall, Lv1, Player owned)
    auto townhall = Building::create(Core::BuildingType::kTownHall, 1, 0);

    // [Contract 2.1] Set coordinates (Logical Grid 10,10)
    townhall->setPosition(GridToPixel(10, 10));

    // [Contract 2.2] Immediately add to scene
    game_layer_->addChild(townhall);
    
    // [Contract 2.3] Notify Economy System to recalculate limits (Town Hall provides resource capacity)
    cocos2d::Vector<Building*> buildings;
    buildings.pushBack(townhall);
    EconomySystem::GetInstance()->RecalculateLimits(buildings);
}

void GameplayIntegrationScene::OnBuildCannon() {
    // Assume cost is 200 Gold (Force add gold for testing)
    EconomySystem::GetInstance()->AddGold(1000); 

    int cost = 200;
    if (EconomySystem::GetInstance()->SpendGold(cost)) {
        auto cannon = Building::create(Core::BuildingType::kCannon, 1, 0);
        cannon->setPosition(GridToPixel(15, 10)); // Placed to the right of Town Hall
        
        // Start construction: Cannot attack for 5 seconds
        cannon->StartConstruction(5.0f);
        
        game_layer_->addChild(cannon);
        cocos2d::log("Test: Cannon construction started.");
    }
}

void GameplayIntegrationScene::OnSpawnBarbarian() {
    // Assume Barbarian takes 1 population
    if (EconomySystem::GetInstance()->AddTroopPopulation(1)) {
        // Create Barbarian (Lv1, Player owned)
        auto barb = Unit::create(Core::TroopType::kBarbarian, 1, 0);
        
        // Place slightly further away (Pixel coordinates)
        barb->setPosition(Vec2(200, 200));
        
        game_layer_->addChild(barb);
        cocos2d::log("Test: Barbarian spawned.");
    } else {
        cocos2d::log("Test: Not enough population!");
    }
}

void GameplayIntegrationScene::update(float dt) {
    // 5. [Contract 2.4] UI Data Polling
    auto eco = EconomySystem::GetInstance();
    resource_label_->setString(StringUtils::format(
        "Gold: %d/%d | Pop: %d/%d", 
        eco->GetCurrentGold(), eco->GetMaxGold(),
        eco->GetCurrentPopulation(), eco->GetMaxPopulation()
    ));
}
```
