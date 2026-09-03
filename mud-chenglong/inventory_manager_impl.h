#ifndef INVENTORY_MANAGER_IMPL_H
#define INVENTORY_MANAGER_IMPL_H

// 模块4：背包与道具系统
// 架构约束：本文件只依赖 game_api.h（公共契约），不引用其他模块的实现头文件。
// 跨模块调用一律通过抽象基类指针（IBattleEngine* / ITalismanManager* 等）。
#include "game_api.h"   // IInventoryManager / ItemID / Entity 前置声明 / 各 IXxx 抽象基类

#include <string>
#include <map>

// ==================== 道具配置结构体（模块4内部使用） ====================
struct ItemConfig {
    ItemID       id;           // 道具ID
    std::string  name;         // 中文名
    int          max_stack;    // 最大堆叠数量
    int          hp_restore;   // 恢复HP量（0=无）
    int          mp_restore;   // 恢复MP量（0=无）
    int          money_value;  // 卖出价格（0=不可卖）

    ItemConfig() : id(ItemID::POTION_HP), max_stack(1), hp_restore(0),
                   mp_restore(0), money_value(0) {}
};

// ==================== InventoryManager 类（继承 IInventoryManager） ====================
// 说明：game_api.h 中的 IInventoryManager 是纯虚接口（无成员），
//       所有状态（背包/金钱/配置）作为本实现类的 private 实例成员存放，
//       不再使用全局变量。可由主程序创建多个实例。
class InventoryManager : public IInventoryManager {
private:
    // ---- 核心数据（实例成员） ----
    std::map<ItemID, int>             items;         // 背包：道具ID → 数量
    static constexpr int              MAX_SLOTS = 10; // 背包上限10格（最多10种道具）
    int                               money;          // 持有金币
    std::map<ItemID, ItemConfig>      item_configs;   // 道具配置表

    // ---- 跨模块指针（抽象基类，由主程序注入） ----
    // 注：模块4自身方法目前主要调用 Entity*（use_item），这些指针预留给
    //     未来扩展（如可施加/驱散状态的道具调用 status_mgr）。
    IBattleEngine*         battle_mgr;
    ITalismanManager*      talisman_mgr;
    IStatusEffectManager*  status_mgr;
    ILevelManager*         level_mgr;

    // ---- 内部辅助 ----
    int get_max_stack(ItemID id) const;   // 取堆叠上限，缺省返回1

public:
    // ---- 构造函数 ----
    InventoryManager();

    // ==================== IInventoryManager 接口实现（8个 override） ====================
    bool add_item(ItemID item_id, int count) override;
    bool remove_item(ItemID item_id, int count) override;
    bool has_item(ItemID item_id) const override;
    int  get_item_count(ItemID item_id) const override;
    void add_money(int amount) override;
    bool consume_money(int amount) override;
    int  get_money() const override;
    void use_item(ItemID item_id, Entity* target) override;

    // ==================== 跨模块指针注入（内部使用，非接口） ====================
    void set_battle_engine(IBattleEngine* mgr);
    void set_talisman_manager(ITalismanManager* mgr);
    void set_status_effect_manager(IStatusEffectManager* mgr);
    void set_level_manager(ILevelManager* mgr);

    // ==================== 模块4内部方法（不放入接口，供UI/存档使用） ====================
    int  get_used_slots() const;                       // 已使用的格子数
    bool is_full() const;                              // 背包是否已满
    const std::map<ItemID, int>& get_all_items() const; // 背包只读视图（存档/UI）
};

#endif // INVENTORY_MANAGER_IMPL_H
