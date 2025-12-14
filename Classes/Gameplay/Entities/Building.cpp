// Copyright 2025 Clash of Clans Project. All rights reserved.
//
// Author: Developer B
//
// Implementation of Building logic.

#include "Building.h"
#include "Unit.h" 
#include "Gameplay/Components/HealthComp.h" 
#include "Gameplay/Logic/EconomySystem.h" // 引入经济系统
#include "Gameplay/Logic/CombatResolver.h"
#include "Gameplay/Components/AttackComp.h"

//静态创建函数
Building* Building::create(Core::BuildingType type, int level, int owner_id) {
    Building* pRet = new(std::nothrow) Building();
    // 调用 init 初始化
    if (pRet && pRet->init(type, level, owner_id)) {
        pRet->autorelease();
        return pRet;
    }
    else {
        // 初始化失败，手动删除防止内存泄漏
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}
//初始化函数
bool Building::init(Core::BuildingType type, int level, int owner_id) {
    //初始化父类 BaseEntity
    if (!BaseEntity::init()) {
        return false;
    }

    // 建筑永远在第 10 层。
    // 这比 TileMap (-1) 高，但比 Unit (20) 低。
    this->setLocalZOrder(10);
    //保存类型和等级
    this->type_ = type;
    this->level_ = level;
    //从GameConfig读取数值
    // 血量、攻击力、攻击范围、产出效率
    this->stats_ = Core::GameConfig::GetInstance()->GetBuildingStats(type, level);

    //设置阵营
    this->set_owner_id(owner_id);
    if (owner_id == 0) {
        this->set_camp(Core::CampType::kPlayer);
    }
    else {
        this->set_camp(Core::CampType::kEnemy);
    }

    // 默认不在建造中 (除非加载存档时另行设置)
    this->is_constructing_ = false;
    this->construction_timer_ = 0.0f;

    //加载外观
    std::string filename = GetSpriteFilename(type);
    visual_sprite_ = cocos2d::Sprite::create(filename);

    if (visual_sprite_) {
        // 放在节点中心
        visual_sprite_->setPosition(cocos2d::Vec2::ZERO);

        // 稍微调整一下层级，保证建筑盖在地板上，但在单位下面
        // (虽然 BaseEntity 外部也会设置 ZOrder，这里是内部相对层级)
        this->addChild(visual_sprite_);
    }
    else {
        cocos2d::log("Error: Failed to load building sprite: %s", filename.c_str());
        // 如果图出现问题，画个红方块代表建筑
        auto debugRect = cocos2d::DrawNode::create();
        debugRect->drawSolidRect(cocos2d::Vec2(-30, -30), cocos2d::Vec2(30, 30), cocos2d::Color4F::RED);
        this->addChild(debugRect);
    }
    //挂载HealthComp
    auto health_comp = HealthComp::create();
    health_comp->InitStats(stats_.max_hp_);
    health_comp->setName("HealthComp");

    // 建筑通常比较高大，血条挂高
    health_comp->SetHealthBarOffset(cocos2d::Vec2(0, 50));
    this->addChild(health_comp);

    // 只有具有攻击力的建筑才挂载攻击组件，节省内存
    if (stats_.damage_ > 0) {
        auto attack_comp = AttackComp::create();

        // 1. 获取对应的投射物类型
        Core::ProjectileType projType = GetProjectileTypeFromBuilding(type);

        // 2. 初始化组件 (伤害, 射程像素, 攻速, 投射物类型)
        attack_comp->InitStats(stats_.damage_, GetRangeInPixels(), stats_.attack_speed_, projType);

        attack_comp->setName("AttackComp");
        this->addChild(attack_comp);
    }

    stored_resource_ = 0.0f;

    return true;
}

// 每一帧的逻辑 (Update)
void Building::update(float dt) {
    auto hp_comp = dynamic_cast<HealthComp*>(this->getChildByName("HealthComp"));
    // 如要更新动画请在此处
    if (hp_comp && hp_comp->IsDead() && !is_collapsing_) {

        // 1. 标记状态，防止重复进入
        is_collapsing_ = true;

        // 2. 关键：撤销 BaseEntity 的销毁标记
        this->is_marked_for_destruction_ = false;

        // 3. 播放倒塌动画
        if (visual_sprite_) {
            visual_sprite_->setColor(cocos2d::Color3B::GRAY); // 变灰（废墟感）

            // 动画序列：
            // 1. 稍微震动一下
            // 2. 压扁 (ScaleY 变小) 同时 淡出 (FadeOut)
            auto collapse = cocos2d::Spawn::create(
                cocos2d::ScaleTo::create(0.5f, 1.2f, 0.1f), // 变宽变扁
                cocos2d::FadeOut::create(0.5f),             // 慢慢消失
                nullptr
            );

            auto seq = cocos2d::Sequence::create(
                collapse,
                cocos2d::CallFunc::create([this]() {
                    // 4. 戏演完了，再次标记销毁
                    this->MarkForDestruction();
                    }),
                nullptr
            );
            visual_sprite_->runAction(seq);

            // 隐藏血条，因为它已经毁了
            hp_comp->setVisible(false);
        }
        else {
            // 如果没有图片，直接销毁
            this->MarkForDestruction();
        }
        return; // 正在倒塌中，跳过后续逻辑
    }
    // 调用父类逻辑
    BaseEntity::update(dt);
    if (IsMarkedForDestruction()) return;
    if (is_collapsing_) return; // 倒塌中不工作

    if (is_constructing_) {
        construction_timer_ -= dt;

        // 可以在这里添加建造进度条更新逻辑...

        if (construction_timer_ <= 0.0f) {
            // 建造完成!
            is_constructing_ = false;
            construction_timer_ = 0.0f;

            cocos2d::log("Building[%d] construction finished!", this->get_instance_id());

            // 视觉反馈: 恢复正常颜色 (假设建造时是暗色)
            if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::WHITE);
            // 通知 EconomySystem 重新计算上限 (Capacity/Population)
            auto parent_node = this->getParent();
            if (parent_node) {
                cocos2d::Vector<Building*> all_buildings;
                auto children = parent_node->getChildren();

                for (auto node : children) {
                    auto b = dynamic_cast<Building*>(node);
                    if (b) {
                        all_buildings.pushBack(b);
                    }
                }

                // 调用单例重新计算
                // 因为新建筑(this)的 is_constructing_ 刚刚变为 false，
                // 所以它现在会被 EconomySystem 纳入计算，从而增加上限。
                EconomySystem::GetInstance()->RecalculateLimits(all_buildings);
            }
        }
        else {
            // 还在建造中，跳过后续逻辑
            // 视觉反馈: 变暗表示正在施工
            if (visual_sprite_) visual_sprite_->setColor(cocos2d::Color3B::GRAY);
            return;
        }
    }
    // 资源生产
    // 只有产出率 > 0 的建筑（如金矿）才会执行此逻辑
    // 如加农炮的 production_rate_ 为 0，条件不满足，自动跳过
    if (stats_.production_rate_ > 0) {
        ProduceResource(dt);
    }

    // 防御塔索敌
    // 只有伤害 > 0 的建筑（如加农炮、箭塔）才会执行
    // 金矿的 damage_ 为 0，条件不满足，自动跳过
    if (stats_.damage_ > 0) {
        UpdateCombatLogic(dt);
    }
}


void Building::StartConstruction(float duration) {
    if (duration <= 0) return;
    is_constructing_ = true;
    construction_timer_ = duration;
    cocos2d::log("Building[%d] started construction. Time: %.1f", this->get_instance_id(), duration);
}

// 经济系统实现 (Economy Logic)
// [内部] 每帧累加资源
void Building::ProduceResource(float dt) {
    // 如果满了，停止生产
    if (stored_resource_ >= stats_.resource_capacity_) {
        return;
    }

    // 计算公式：时产 / 3600 = 秒产
    // dt 是这一帧经过的秒数
    float amount_per_sec = stats_.production_rate_ / 3600.0f;
    stored_resource_ += amount_per_sec * dt;

    // 封顶检查
    if (stored_resource_ > stats_.resource_capacity_) {
        stored_resource_ = static_cast<float>(stats_.resource_capacity_);
    }
}

// [外部接口] 玩家点击收集时调用
int Building::CollectResource(int max_amount) {
    int available = static_cast<int>(stored_resource_);
    int to_collect = available;

    // 如果指定了上限，且库存充足，则只取上限值
    if (max_amount >= 0 && available > max_amount) {
        to_collect = max_amount;
    }

    // 扣除实际取走的量
    stored_resource_ -= static_cast<float>(to_collect);

    // 浮点数误差修正（防止剩余 0.0001 这种）
    if (stored_resource_ < 0.0f) stored_resource_ = 0.0f;

    return to_collect;
}

// [核心重构] 建筑充当"大脑"(索敌)，AttackComp充当"手臂"(执行)
void Building::UpdateCombatLogic(float dt) {
    // 1. 获取攻击组件
    auto attack_comp = dynamic_cast<AttackComp*>(this->getChildByName("AttackComp"));
    if (!attack_comp) return;

    // 2. 性能优化：如果组件还在冷却中，不要费劲去遍历全图索敌
    if (attack_comp->GetCooldownTimer() > 0.0f) {
        return;
    }

    auto parent_node = this->getParent();
    if (!parent_node) return;

    // 3. 索敌 (这部分逻辑依然需要 Building 自己做，因为它没有 PathAgent)
    const auto& all_nodes = parent_node->getChildren();
    BaseEntity* best_target = nullptr;
    float min_dist_sq = FLT_MAX;

    // 使用 AttackComp 计算好的距离平方 (它包含了 射程^2)
    // 但这里我们需要获取射程的数值来做初步筛选
    float range_pixels = GetRangeInPixels();
    float range_sq = range_pixels * range_pixels;

    for (auto node : all_nodes) {
        if (node == this) continue;

        auto target_entity = dynamic_cast<BaseEntity*>(node);
        if (!target_entity) continue;
        if (target_entity->get_camp() == this->get_camp()) continue; // 排除友军
        if (target_entity->IsMarkedForDestruction()) continue; // 排除将死之人

        auto hp_comp = dynamic_cast<HealthComp*>(target_entity->getChildByName("HealthComp"));
        if (hp_comp && hp_comp->IsDead()) continue; // 排除死人

        // 排除打不到的目标 (陆/空 筛选)
        auto target_unit = dynamic_cast<Unit*>(target_entity);
        if (target_unit) {
            if (!this->CanAttack(target_unit->GetGeneralType())) continue;
        }

        // 距离筛选 (简单的中心点距离，精细判断交给 AttackComp)
        float dist_sq = this->getPosition().getDistanceSq(target_entity->getPosition());
        if (dist_sq <= range_sq) {
            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                best_target = target_entity;
            }
        }
    }

    // 4. 执行攻击
    if (best_target) {
        // [委托] 直接告诉组件去打
        // 组件内部会处理：冷却重置、边缘距离精细判定、调用 CombatResolver 发射投射物
        if (attack_comp->TryAttack(best_target)) {
            // 如果攻击成功触发 (返回 true)，播放一个闪烁特效作为开火反馈
            if (visual_sprite_) {
                auto tint = cocos2d::Sequence::create(
                    cocos2d::TintTo::create(0.1f, 255, 100, 100),
                    cocos2d::TintTo::create(0.1f, 255, 255, 255),
                    nullptr
                );
                visual_sprite_->runAction(tint);
            }
        }
    }
}

// 辅助函数
std::string Building::GetSpriteFilename(Core::BuildingType type) {
    switch (type) {
    case Core::BuildingType::kTownHall:       return "Buildings_Icon/TownHall.png";
    case Core::BuildingType::kGoldMine:       return "Buildings_Icon/GoldMine.png";
    case Core::BuildingType::kElixirCollector:return "Buildings_Icon/ElixirCollector.png";
    case Core::BuildingType::kCannon:         return "Buildings_Icon/Cannon.png";
    case Core::BuildingType::kArcherTower:    return "Buildings_Icon/ArcherTower.png";
    case Core::BuildingType::kWall:           return "Buildings_Icon/Wall.png";
    case Core::BuildingType::kArmyCamp:       return "Buildings_Icon/ArmyCamp.png";
    default:                                  return "Buildings_Icon/Building_Default.png";
    }
}

// 辅助：映射建筑类型到投射物类型
Core::ProjectileType Building::GetProjectileTypeFromBuilding(Core::BuildingType type) {
    switch (type) {
    case Core::BuildingType::kCannon:
        return Core::ProjectileType::kCannonBall;
    case Core::BuildingType::kArcherTower:
        return Core::ProjectileType::kArrow;
    case Core::BuildingType::kAirDefense:
        return Core::ProjectileType::kRocket;
    default:
        return Core::ProjectileType::kNone;
    }
}

// 占地矩形 (显式使用 cocos2d::Rect)
cocos2d::Rect Building::GetOccupiedRect() const {
    cocos2d::Vec2 pos = this->getPosition();

    float total_width = stats_.width_ * Core::kTileWidth;
    float total_height = stats_.height_ * Core::kTileHeight;

    return cocos2d::Rect(
        pos.x - total_width / 2,
        pos.y - total_height / 2,
        total_width,
        total_height
    );
}