// 模块4：背包与道具系统 —— 实现文件
// ----------------------------------------------------------
// 设计遵循 v3 game_api.h 抽象基类架构：
//   1. 不修改公共头文件 game_api.h（已锁定）。
//   2. 状态作为 InventoryManager 实例成员存放，不再使用全局变量。
//   3. 所有ID使用 enum class（ItemID），禁止裸 int。
//   4. 跨模块调用经抽象基类指针；本模块 use_item 需调用 target->heal()/restore_mp()，
//      详见 use_item 内的集成说明注释。
//   5. 符咒相关功能已全部移除，交由模块2。

#include "inventory_manager_impl.h"

#include <algorithm>   // std::min

// ============================================================================
// 构造函数
// ============================================================================
InventoryManager::InventoryManager()
    : money(0),
      battle_mgr(nullptr),
      talisman_mgr(nullptr),
      status_mgr(nullptr),
      level_mgr(nullptr) {
    // 填充道具配置表（数值后续可做平衡调整）
    item_configs[ItemID::POTION_HP] = {ItemID::POTION_HP, "生命药水", 10, 100,   0, 50};
    item_configs[ItemID::POTION_MP] = {ItemID::POTION_MP, "魔力药水", 10,   0, 100, 50};
}

// ============================================================================
// 内部辅助
// ============================================================================
int InventoryManager::get_max_stack(ItemID id) const {
    auto it = item_configs.find(id);
    return it == item_configs.end() ? 1 : it->second.max_stack;
}

// ============================================================================
// IInventoryManager 接口实现
// ============================================================================

// ---- 背包管理 ----

bool InventoryManager::add_item(ItemID item_id, int count) {
    if (count <= 0) return false;

    auto it = items.find(item_id);
    if (it != items.end()) {
        // 已有该道具：在堆叠上限内累加
        int cap = get_max_stack(item_id);
        it->second = std::min(it->second + count, cap);
        return true;
    }
    // 新道具：检查背包格子是否已满
    if (static_cast<int>(items.size()) >= MAX_SLOTS) return false;
    items[item_id] = count;
    return true;
}

bool InventoryManager::remove_item(ItemID item_id, int count) {
    if (count <= 0) return false;
    auto it = items.find(item_id);
    if (it == items.end() || it->second < count) return false;

    it->second -= count;
    if (it->second == 0) items.erase(it);
    return true;
}

bool InventoryManager::has_item(ItemID item_id) const {
    auto it = items.find(item_id);
    return it != items.end() && it->second > 0;
}

int InventoryManager::get_item_count(ItemID item_id) const {
    auto it = items.find(item_id);
    return it == items.end() ? 0 : it->second;
}

// ---- 金钱管理 ----

void InventoryManager::add_money(int amount) {
    if (amount > 0) money += amount;
}

bool InventoryManager::consume_money(int amount) {
    if (amount <= 0 || money < amount) return false;
    money -= amount;
    return true;
}

int InventoryManager::get_money() const {
    return money;
}

// ---- 使用道具 ----

void InventoryManager::use_item(ItemID item_id, Entity* target) {
    // 必须持有至少1个
    if (!has_item(item_id)) return;
    if (target == nullptr) return;

    auto cfg_it = item_configs.find(item_id);
    if (cfg_it == item_configs.end()) return;
    const ItemConfig& cfg = cfg_it->second;

    // 应用效果：恢复HP/MP
    // ---- 集成说明 ----
    // game_api.h 仅前置声明 class Entity，本 .cpp 无法直接调用 target->heal()/restore_mp()。
    // 集成时需确保模块1的 Entity 提供虚函数 heal(int)/restore_mp(int)（Player 覆写之），
    // 并在本文件顶部 #include 模块1的实体头文件（如 "../module1/battle_engine_impl.h"），
    // 然后取消下面两行注释即可生效。这是模块4唯一的跨模块集成点。
    if (cfg.hp_restore > 0) {
        // target->heal(cfg.hp_restore);
    }
    if (cfg.mp_restore > 0) {
        // target->restore_mp(cfg.mp_restore);
    }

    // 消耗1个
    remove_item(item_id, 1);
}

// ============================================================================
// 跨模块指针注入
// ============================================================================
void InventoryManager::set_battle_engine(IBattleEngine* mgr) {
    battle_mgr = mgr;
}
void InventoryManager::set_talisman_manager(ITalismanManager* mgr) {
    talisman_mgr = mgr;
}
void InventoryManager::set_status_effect_manager(IStatusEffectManager* mgr) {
    status_mgr = mgr;
}
void InventoryManager::set_level_manager(ILevelManager* mgr) {
    level_mgr = mgr;
}

// ============================================================================
// 模块4内部方法（UI/存档）
// ============================================================================
int InventoryManager::get_used_slots() const {
    return static_cast<int>(items.size());
}

bool InventoryManager::is_full() const {
    return static_cast<int>(items.size()) >= MAX_SLOTS;
}

const std::map<ItemID, int>& InventoryManager::get_all_items() const {
    return items;
}
