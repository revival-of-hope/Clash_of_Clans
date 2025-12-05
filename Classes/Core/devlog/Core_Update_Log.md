Core 模块更新日志1 (Core Module Update Log)

日期: 2025-12-05
作者: Developer B (Gameplay)
模块: Classes/Core/
涉及文件: GameConstants.h, GameStructs.h, GameConfig.cpp

🚀 更新概览 (Overview)

本次更新主要完善了建筑物的核心数值与属性。

Developer B 特别说明:

"我已完善了 GameConfig.cpp 中原本缺失的部分，补全了核心建筑（如资源收集器、兵营）的数值配置，并添加了必要的对地/对空逻辑支持。"

具体实现了以下三个关键功能：

攻击目标区分 (Target Logic): 区分防御塔是只打地面、只打空中，还是陆空兼顾。

资源系统支持 (Economy): 补充了金矿、圣水收集器、储金罐、圣水瓶的数值。

人口系统支持 (Capacity): 为兵营 (Army Camp) 增加了“人口容量”属性。

📂 1. GameConstants.h (常量定义)

新增枚举: TargetType

用于定义攻击者的目标偏好。
```text
enum class TargetType {
    kNone = 0,          // 不攻击 (如资源建筑)
    kGround = 1,        // 仅地面 (如加农炮)
    kAir = 2,           // 仅空中 (如防空火箭)
    kGroundAndAir = 3   // 地面和空中 (如箭塔)
};
```

补充枚举: BuildingType

为了防止遗漏，明确添加了兵营。
```text
enum class BuildingType {
    // ... 原有内容 ...
    kArmyCamp = 10 // [NEW] 补充定义 ArmyCamp
};
```

📂 2. GameStructs.h (数据结构)

修改结构体: BuildingStats

增加了两个新字段以支持新逻辑。
```text
struct BuildingStats {
    // ... 原有字段 (hp, damage, etc.) ...
    
    // [NEW] 攻击逻辑
    TargetType target_type_;  // 攻击目标类型 (对地/对空)
    
    // [NEW] 兵营逻辑
    int troop_capacity_;      // 兵营提供的总人口 (Housing Space)
};
```

📂 3. GameConfig.cpp (数值配置)

3.1 安全性优化

在 GetBuildingStats 函数开头增加了默认初始化，防止未定义的字段读取到内存垃圾值。

3.2 缺失内容补全 (Work by Dev B)

完善了以下之前缺失的建筑逻辑：

资源建筑: 采集器 (Mines) 和 仓库 (Storages)。

防御塔: 新增箭塔 (Archer Tower) 和防空火箭 (Air Defense)。

兵营: 新增人口容量逻辑 (troop_capacity_)。
```text
// 初始化默认值
stats.damage_ = 0;
stats.range_ = 0.0f;
stats.target_type_ = TargetType::kNone; // [NEW]
```
