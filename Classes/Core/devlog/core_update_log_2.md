### **Update Log2: Combat Logic Refactor**


Date: 2025-12-06  
Author: dev C (full stack)
Module: Core
Affected Module: GamePlay

**Core Changes:**

* **Bitmask Implementation:** Converted GeneralType from sequential enum (0,1,2,3) to bitwise flags (1\<\<0, 1\<\<1).  
* **Dual-Purpose Typing:**  
  * **Units:** GeneralType now defines what a unit *is* (e.g., kAir).  
  * **Structures:** GeneralType now defines what a structure *targets* (e.g., kGround | kAir).  
* **Stats Standardization:** Removed inconsistent TargetType usage; all entities now strictly use GeneralType for classification.
* **Readme Update:** Updated core/readme. Removed coding convention details that were already clarified in the main file.

**Entity Updates:**

* **Archer Tower:** Targeting logic updated to kGround | kAir (Hybrid targeting).  
* **Baby Dragon:** Classification confirmed as kAir (Air unit).  
* **Cannon:** Targeting restricted to kGround.  
* **Safety:** Added default initialization for GetBuildingStats to prevent garbage data on passive buildings.

**Technical Impact:**

* Enables O(1) targeting checks using bitwise AND (&) rather than complex switch/if-else chains.