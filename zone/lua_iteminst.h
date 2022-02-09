#ifndef EQEMU_LUA_ITEMINST_H
#define EQEMU_LUA_ITEMINST_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class Lua_Item;

namespace EQ
{
	class ItemInstance;
}

namespace luabind {
	struct scope;
}

luabind::scope lua_register_iteminst();

class Lua_ItemInst : public Lua_Ptr<EQ::ItemInstance>
{
	typedef EQ::ItemInstance NativeType;
public:
	Lua_ItemInst(int item_id);
	Lua_ItemInst(int item_id, int charges);
	Lua_ItemInst() : Lua_Ptr(nullptr), cloned_(false) { }
	Lua_ItemInst(EQ::ItemInstance *d) : Lua_Ptr(d), cloned_(false) { }
	Lua_ItemInst(EQ::ItemInstance *d, bool cloned) : Lua_Ptr(d), cloned_(cloned) { }
	Lua_ItemInst& operator=(const Lua_ItemInst& o);
	Lua_ItemInst(const Lua_ItemInst& o);
	virtual ~Lua_ItemInst() { if(cloned_) { EQ::ItemInstance *ptr = GetLuaPtrData(); if(ptr) { delete ptr; } } }

	operator EQ::ItemInstance*() {
		return reinterpret_cast<EQ::ItemInstance*>(GetLuaPtrData());
	}

	bool IsType(int item_class);
	bool IsStackable();
	bool IsEquipable(int race, int class_);
	bool IsEquipable(int slot_id);
	bool IsAugmentable();
	int GetAugmentType();
	bool IsExpendable();
	Lua_ItemInst GetItem(int slot);
	Lua_Item GetItem();
	void SetItem(Lua_Item item);
	Lua_Item GetUnscaledItem(int slot);
	uint32 GetItemID(int slot);
	int GetTotalItemCount();
	Lua_ItemInst GetAugment(int slot);
	uint32 GetAugmentItemID(int slot);
	bool IsAugmented();
	bool IsWeapon();
	bool IsAmmo();
	uint32 GetID();
	uint32 GetItemScriptID();
	int GetCharges();
	void SetCharges(int charges);
	uint32 GetPrice();
	void SetPrice(uint32 price);
	void SetColor(uint32 color);
	uint32 GetColor();
	bool IsInstNoDrop();
	void SetInstNoDrop(bool flag);
	std::string GetCustomDataString();
	void SetCustomData(std::string identifier, std::string value);
	void SetCustomData(std::string identifier, int value);
	void SetCustomData(std::string identifier, float value);
	void SetCustomData(std::string identifier, bool value);
	std::string GetCustomData(std::string identifier);
	void DeleteCustomData(std::string identifier);
	void SetScaling(bool v);
	void SetScale(double scale_factor);
	uint32 GetExp();
	void SetExp(uint32 exp);
	void AddExp(uint32 exp);
	int GetMaxEvolveLvl();
	uint32 GetKillsNeeded(int current_level);
	Lua_ItemInst Clone();
	void SetTimer(std::string name, uint32 time);
	void StopTimer(std::string name);
	void ClearTimers();
	bool ContainsAugmentByID(uint32 item_id);
	int CountAugmentByID(uint32 item_id);

private:
	bool cloned_;
};

#endif
#endif
