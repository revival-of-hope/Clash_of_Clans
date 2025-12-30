# **Gameplay Event System Interface Documentation**

Version: 1.1 (Added Event List Quick Reference)  
Author: Developer B (Gameplay)  
Module: Classes/Contract/Gameplay/GameEvents.h

## **1. Overview**

This system adopts the **Observer Pattern** and is designed to decouple the Gameplay layer (Logic) from the UI/Audio layers (Presentation).  
The UI layer does not need to poll logic data changes every frame; instead, it registers listeners to passively receive state change notifications from the logic layer.

*   **Push Mechanism**: Logic layer changes -> `GameEventManager` broadcasts -> All registered `IGameEventListener` instances receive notification.
*   **Thread Safety**: Currently designed as single-threaded synchronous calls (Main Thread). Please do not perform time-consuming operations inside callbacks.

## **2. Stable Event List Quick Reference**

This is the complete event contract that the Gameplay layer promises to send to the UI/Audio layers. Dev C should implement the corresponding presentation logic based on this table.

| Event Interface Name | Trigger Timing | Core Payload | Expected Behavior (UI/Dev C) |
| :---- | :---- | :---- | :---- |
| **OnResourceChanged** | Resource collection, troop training cost, upgrade cost | Type (Gold/Elixir/Pop/Gems/Trophies), Current, Max, Delta (Change Amount) | Refresh top resource bar numbers; if Delta > 0, play resource fly-in animation. |
| **OnTroopCountUpdated** | Troop counts change from training/consumption | OwnerID, TroopType, RemainingCount | Update deployment bar counts for the matching troop type. |
| **OnDeploymentSelectionChanged** | Player selects or clears a troop slot for deployment | OwnerID, HasSelection, TroopType | Highlight selected troop in deployment bar or clear highlight when HasSelection is false. |
| **OnEntitySpawned** | Player places unit, map initializes buildings | InstanceID, Type, Level, Pos, OwnerID | Create Sprite Node; Create HealthBar component; **Must** maintain an ID -> Node mapping table. |
| **OnEntityDestroyed** | Unit/Building HP reaches zero | InstanceID, Type | Play death sound; Play death animation; Remove node from scene; Clean up mapping. |
| **OnBuildingStateChanged** | Construction start, completion, destruction | InstanceID, State, TimeRemaining | Constructing: Show countdown/progress bar. Destroyed: Switch to ruin sprite. Idle: Hide progress bar, restore normal state. |
| **OnEntityDamaged** | Upon receiving any attack damage | TargetID, CurrentHP, MaxHP, DamageAmount | Find HealthBar via TargetID, update progress (Current/Max); Pop up floating damage numbers. |
| **OnProjectileFired** | When ranged unit attack pre-cast ends | SourceID, TargetPos, ProjectileType | **Audio**: Play firing sound (e.g., bowstring). **VFX**: (Optional) If Logic layer doesn't create the Sprite, UI layer creates it here. |
| **OnProjectileHit** | Projectile arrives at target or hits/vanishes | Pos, ProjectileType | **VFX**: Play explosion/hit effect at Pos. **Audio**: Play hit sound. |
| **OnBattleStarted** | Placing the first troop or entering combat scene | TimeLimit (Seconds) | Start top-screen countdown; Play battle BGM. |
| **OnBattleEnded** | Win/Loss condition met or time runs out | Result (Win/Loss), Stars, Percentage, Loot | Pause countdown; Stop BGM; Pop up settlement panel showing stars and resources gained. |
| **OnLootAvailabilityUpdated** | Enemy base scouted or match preview available | GoldAvail, ElixirAvail, TrophiesAvail | Show available loot/trophies preview (left HUD). |

## **3. Payload Definitions**

All events carry specific structure data to convey context information.

### **3.1 Resource Update (ResourceUpdateEvent)**

Triggered when Gold, Elixir, Population, Gems, or Trophies changes.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| resource_type | string | Resource type string ("Gold", "Elixir", "Population", "Gems", "Trophies") |
| current_amount | int | Current value after change |
| max_capacity | int | Current maximum capacity |
| change_amount | int | Difference in this change (e.g., -200 or +50) |

### **3.2 Loot Availability (LootAvailabilityEvent)**

Triggered when the game has a loot preview for the current target (e.g., match preview or scouting).

| Field Name | Type | Description |
| :---- | :---- | :---- |
| gold_available | int | Available gold to steal |
| elixir_available | int | Available elixir to steal |
| trophies_available | int | Available trophies to win/lose |

### **3.3 Troop Count Update (TroopCountUpdateEvent)**

Triggered when a troop's remaining count changes (training completed or troop deployed). Counts are scoped by owner.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| owner_id | int | Faction ID (0: Player, 1: Enemy/AI) |
| troop_type | enum | Troop type enumeration |
| remaining_count | int | Remaining troop count after the change |

### **3.4 Deployment Selection (DeploymentSelectionEvent)**

Triggered when a player selects or clears a troop slot for deployment.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| owner_id | int | Faction ID (0: Player, 1: Enemy/AI) |
| has_selection | bool | `true` when a troop slot is selected; `false` to clear selection |
| troop_type | enum | Selected troop type (valid only when `has_selection` is true) |

### **3.5 Entity Spawn (EntitySpawnEvent)**

Triggered when a unit is deployed or map buildings are loaded.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| instance_id | int | Global unique ID of the entity (for future indexing) |
| owner_id | int | Faction ID (0: Player, 1: Enemy/AI) |
| x, y | float | Initial world coordinates (Pixels) |
| level | int | Level (Determines appearance texture) |
| current_hp | int | Current HP at spawn time |
| max_hp | int | Max HP at spawn time |
| is_building | bool | `true` for building, `false` for troop |
| troop_type | enum | Troop type enumeration (if it is a troop) |
| building_type | enum | Building type enumeration (if it is a building) |

### **3.6 Entity Destroy (EntityDestroyEvent)**

Triggered when a unit dies or a building completely collapses.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| instance_id | int | ID of the corresponding entity |
| is_building | bool | Whether it is a building |

### **3.7 Building State Change (BuildingStateEvent)**

Used to handle construction progress bars and ruin state switching.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| instance_id | int | Building ID |
| type | enum | Building Type |
| new_state | enum | kConstructing, kIdle (Normal), kDestroyed (Ruin) |
| time_remaining | float | Remaining construction time (seconds) |
| total_build_time | float | Total construction time (used to calculate progress bar percentage) |

### **3.8 Damage & Health (DamageEvent)**

Triggered when an entity is attacked.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| target_instance_id | int | Victim ID |
| damage_amount | int | Damage received this time |
| current_hp | int | Remaining HP |
| max_hp | int | Max HP (UI can use this to calculate health bar percentage) |
| is_critical | bool | Is critical hit (Reserved) |

### **3.9 Projectile Events**

*   **Fired (ProjectileEvent)**: Triggered when a ranged unit finishes attack pre-cast and fires a bullet.
*   **Hit (ProjectileHitEvent)**: Triggered when a bullet hits a target or the ground.

### **3.10 Battle End (BattleEndEvent)**

Triggered when the battle ends. This payload is the authoritative settlement dataset for the results scene.

| Field Name | Type | Description |
| :---- | :---- | :---- |
| result | enum | kVictory, kDefeat, kTimeOut |
| stars_earned | int | Number of stars earned (0-3) |
| destruction_percent | int | Destruction rate (0-100) |
| gold_stolen | int | Total gold looted |
| elixir_stolen | int | Total elixir looted |
| trophies_earned | int | Trophy delta earned (can be negative) |
| trophies_total | int | Player's total trophies after settlement |
| battle_duration_seconds | int | Total elapsed battle time (seconds) |
| troops_deployed | int | Total count of troops deployed |
| troops_remaining | int | Troops still alive when the battle ended |
| spells_used | int | Spells cast during the battle |

**Access pattern:** The latest `BattleEndEvent` is cached in `GameEventManager` (Contract path `Classes/Contract/Gameplay/GameEvents.h`) and can be retrieved via `GetLastBattleEnded()` even if no listener was registered in time.

## **4. Usage Guide**

### **4.1 For Dev C (UI / Audio) - How to Listen for Events**

You need to make your class (e.g., `HUDLayer` or `AudioManager`) inherit from `Gameplay::IGameEventListener` and implement the callback functions of interest.

**Example Code (HUDLayer.h):**

```cpp
#include "Classes/Contract/Gameplay/GameEvents.h"

class HUDLayer : public cocos2d::Layer, public Gameplay::IGameEventListener {
public:
    virtual bool init() override {
        // ... Initialization code ...
        
        // [Key Step 1] Register Listener
        Gameplay::GameEventManager::GetInstance()->AddListener(this);
        return true;
    }

    virtual void onExit() override {
        // [Key Step 2] Remove Listener (Must do, otherwise it will crash)
        Gameplay::GameEventManager::GetInstance()->RemoveListener(this);
        cocos2d::Layer::onExit();
    }

    // [Key Step 3] Implement Callback Interfaces
    
    // Resource Change -> Update UI Numbers
    virtual void OnResourceChanged(const Gameplay::ResourceUpdateEvent& evt) override {
        if (evt.resource_type == "Gold") {
            this->UpdateGoldLabel(evt.current_amount);
            if (evt.change_amount > 0) {
                this->PlayResourceGainEffect("Gold");
            }
        }
    }

    // Battle Ended -> Popup
    virtual void OnBattleEnded(const Gameplay::BattleEndEvent& evt) override {
        this->ShowResultPopup(evt.stars_earned, evt.destruction_percent);
    }
    
    // Other uninterested interfaces can be left unimplemented (Base class has default empty implementations)
};
```

### **4.2 For Dev B (Gameplay) - How to Broadcast Events**

In the logic code, send notifications via `Broadcast...` methods.

**Example Code (EconomySystem.cpp):**

```cpp
void EconomySystem::AddGold(int amount) {
    int old_val = current_gold_;
    current_gold_ += amount;
    // ... Cap handling ...
    
    // [Broadcast Event]
    Gameplay::ResourceUpdateEvent evt;
    evt.resource_type = "Gold";
    evt.current_amount = current_gold_;
    evt.max_capacity = max_gold_;
    evt.change_amount = amount; // Here it is +50
    
    Gameplay::GameEventManager::GetInstance()->BroadcastResourceChange(evt);
}
```

## **5. Best Practices**

1.  **Lifecycle Management**:
    *   Be sure to call `RemoveListener(this)` in `onExit()` or the destructor. If an object is destroyed but remains in the listener list, the next broadcast will cause a dangling pointer crash.
2.  **UI Map Maintenance**:
    *   Upon receiving `OnEntitySpawned`, the UI layer should establish a `std::map<int, Node*> entity_map_`.
    *   Upon receiving `OnEntityDestroyed` or `OnEntityDamaged`, use `evt.instance_id` to look up the corresponding node in the map for operations.
3.  **Do Not Modify Logic in Callbacks**:
    *   `IGameEventListener` callbacks are primarily for **Read-Only** display. Avoid calling logic state modification functions (like `EconomySystem::AddGold`) inside a callback, as this may lead to infinite loops or state inconsistencies.
