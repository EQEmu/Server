#ifndef EQEMU_LUA_ITEMINST_H
#define EQEMU_LUA_ITEMINST_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class ItemInst;
class Lua_Item;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_iteminst();

class Lua_ItemInst : public Lua_Ptr<void>
{
	typedef ItemInst NativeType;
public:
	Lua_ItemInst() { }
	Lua_ItemInst(ItemInst *d) : Lua_Ptr(d) { }
	virtual ~Lua_ItemInst() { }

	operator ItemInst*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<ItemInst*>(d);
		}

		return nullptr;
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
	void SetScale(double scale_factor);
	uint32 GetExp();
	void SetExp(uint32 exp);
	void AddExp(uint32 exp);
	int GetMaxEvolveLvl();
	uint32 GetKillsNeeded(int current_level);
};

#endif
#endif
