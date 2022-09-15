#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "masterentity.h"
#include "lua_iteminst.h"
#include "lua_item.h"

void lua_register_iteminst(sol::state_view &sv)
{
	auto iteminst = sv.new_usertype<Lua_ItemInst>(
	    "ItemInst", sol::constructors<Lua_ItemInst(), Lua_ItemInst(int), Lua_ItemInst(int, int)>());
	iteminst["null"] = sol::readonly_property(&Lua_ItemInst::Null);
	iteminst["valid"] = sol::readonly_property(&Lua_ItemInst::Valid);
	iteminst["AddExp"] = (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::AddExp;
	iteminst["ClearTimers"] = (void(Lua_ItemInst::*)(void))&Lua_ItemInst::ClearTimers;
	iteminst["Clone"] = (Lua_ItemInst(Lua_ItemInst::*)(void))&Lua_ItemInst::Clone;
	iteminst["ContainsAugmentByID"] = (bool(Lua_ItemInst::*)(uint32))&Lua_ItemInst::ContainsAugmentByID;
	iteminst["CountAugmentByID"] = (int(Lua_ItemInst::*)(uint32))&Lua_ItemInst::CountAugmentByID;
	iteminst["DeleteCustomData"] = (void(Lua_ItemInst::*)(std::string))&Lua_ItemInst::DeleteCustomData;
	iteminst["GetAugment"] = (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetAugment;
	iteminst["GetAugmentItemID"] = (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetAugmentItemID;
	iteminst["GetAugmentType"] = (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetAugmentType;
	iteminst["GetCharges"] = (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetCharges;
	iteminst["GetColor"] = (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetColor;
	iteminst["GetCustomData"] = (std::string(Lua_ItemInst::*)(std::string))&Lua_ItemInst::GetCustomData;
	iteminst["GetCustomDataString"] = (std::string(Lua_ItemInst::*)(void))&Lua_ItemInst::GetCustomDataString;
	iteminst["GetExp"] = (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetExp;
	iteminst["GetID"] = (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetID;
	iteminst["GetItem"] = sol::overload((Lua_Item(Lua_ItemInst::*)(void)) & Lua_ItemInst::GetItem,
					    (Lua_ItemInst(Lua_ItemInst::*)(int)) & Lua_ItemInst::GetItem);
	iteminst["GetItemID"] = (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetItemID;
	iteminst["GetItemScriptID"] = (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetItemScriptID;
	iteminst["GetKillsNeeded"] = (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetKillsNeeded;
	iteminst["GetMaxEvolveLvl"] = (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetMaxEvolveLvl;
	iteminst["GetPrice"] = (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetPrice;
	iteminst["GetTotalItemCount"] = (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetTotalItemCount;
	iteminst["GetUnscaledItem"] = (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetUnscaledItem;
	iteminst["IsAmmo"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAmmo;
	iteminst["IsAugmentable"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAugmentable;
	iteminst["IsAugmented"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAugmented;
	iteminst["IsEquipable"] = sol::overload((bool(Lua_ItemInst::*)(int)) & Lua_ItemInst::IsEquipable,
						(bool(Lua_ItemInst::*)(int, int)) & Lua_ItemInst::IsEquipable);
	iteminst["IsExpendable"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsExpendable;
	iteminst["IsInstNoDrop"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsInstNoDrop;
	iteminst["IsStackable"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsStackable;
	iteminst["IsType"] = (bool(Lua_ItemInst::*)(int))&Lua_ItemInst::IsType;
	iteminst["IsWeapon"] = (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsWeapon;
	iteminst["SetCharges"] = (void(Lua_ItemInst::*)(int))&Lua_ItemInst::SetCharges;
	iteminst["SetColor"] = (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetColor;
	iteminst["SetCustomData"] =
	    sol::overload((void(Lua_ItemInst::*)(std::string, bool)) & Lua_ItemInst::SetCustomData,
			  (void(Lua_ItemInst::*)(std::string, float)) & Lua_ItemInst::SetCustomData,
			  (void(Lua_ItemInst::*)(std::string, int)) & Lua_ItemInst::SetCustomData,
			  (void(Lua_ItemInst::*)(std::string, std::string)) & Lua_ItemInst::SetCustomData);
	iteminst["SetExp"] = (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetExp;
	iteminst["SetInstNoDrop"] = (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetInstNoDrop;
	iteminst["SetPrice"] = (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetPrice;
	iteminst["SetScale"] = (void(Lua_ItemInst::*)(double))&Lua_ItemInst::SetScale;
	iteminst["SetScaling"] = (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetScaling;
	iteminst["SetTimer"] = (void(Lua_ItemInst::*)(std::string,uint32))&Lua_ItemInst::SetTimer;
	iteminst["StopTimer"] = (void(Lua_ItemInst::*)(std::string))&Lua_ItemInst::StopTimer;
}

#endif
