#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "../common/spdat.h"
#include "masterentity.h"
#include "questmgr.h"
#include "zone.h"
#include "zone_config.h"

#include "lua_parser.h"
#include "lua_mod.h"
#include "lua_bit.h"
#include "lua_entity.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_hate_list.h"
#include "lua_client.h"
#include "lua_inventory.h"
#include "lua_npc.h"
#include "lua_spell.h"
#include "lua_entity_list.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_corpse.h"
#include "lua_object.h"
#include "lua_door.h"
#include "lua_spawn.h"
#include "lua_packet.h"
#include "lua_general.h"
#include "lua_encounter.h"
#include "lua_stat_bonuses.h"

void LuaMod::Init()
{
	m_has_melee_mitigation = parser_->HasFunction("MeleeMitigation", package_name_);
	m_has_apply_damage_table = parser_->HasFunction("ApplyDamageTable", package_name_);
	m_has_avoid_damage = parser_->HasFunction("AvoidDamage", package_name_);
	m_has_check_hit_chance = parser_->HasFunction("CheckHitChance", package_name_);
	m_has_try_critical_hit = parser_->HasFunction("TryCriticalHit", package_name_);
	m_has_get_required_aa_experience = parser_->HasFunction("GetRequiredAAExperience", package_name_);
	m_has_get_exp_for_level = parser_->HasFunction("GetEXPForLevel", package_name_);
	m_has_get_experience_for_kill = parser_->HasFunction("GetExperienceForKill", package_name_);
	m_has_common_outgoing_hit_success = parser_->HasFunction("CommonOutgoingHitSuccess", package_name_);
}

void PutDamageHitInfo(lua_State *L, luabind::adl::object &e, DamageHitInfo &hit) {
	luabind::adl::object lua_hit = luabind::newtable(L);
	lua_hit["base_damage"] = hit.base_damage;
	lua_hit["min_damage"] = hit.min_damage;
	lua_hit["damage_done"] = hit.damage_done;
	lua_hit["offense"] = hit.offense;
	lua_hit["tohit"] = hit.tohit;
	lua_hit["hand"] = hit.hand;
	lua_hit["skill"] = (int)hit.skill;
	e["hit"] = lua_hit;
}

void GetDamageHitInfo(luabind::adl::object &ret, DamageHitInfo &hit) {
	auto luaHitTable = ret["hit"];
	if (luabind::type(luaHitTable) == LUA_TTABLE) {
		auto base_damage = luaHitTable["base_damage"];
		auto min_damage = luaHitTable["min_damage"];
		auto damage_done = luaHitTable["damage_done"];
		auto offense = luaHitTable["offense"];
		auto tohit = luaHitTable["tohit"];
		auto hand = luaHitTable["hand"];
		auto skill = luaHitTable["skill"];

		if (luabind::type(base_damage) == LUA_TNUMBER) {
			hit.base_damage = luabind::object_cast<int>(base_damage);
		}

		if (luabind::type(min_damage) == LUA_TNUMBER) {
			hit.min_damage = luabind::object_cast<int>(min_damage);
		}

		if (luabind::type(damage_done) == LUA_TNUMBER) {
			hit.damage_done = luabind::object_cast<int>(damage_done);
		}

		if (luabind::type(offense) == LUA_TNUMBER) {
			hit.offense = luabind::object_cast<int>(offense);
		}

		if (luabind::type(tohit) == LUA_TNUMBER) {
			hit.tohit = luabind::object_cast<int>(tohit);
		}

		if (luabind::type(hand) == LUA_TNUMBER) {
			hit.hand = luabind::object_cast<int>(hand);
		}

		if (luabind::type(skill) == LUA_TNUMBER) {
			hit.skill = (EQ::skills::SkillType)luabind::object_cast<int>(skill);
		}
	}
}

void PutExtraAttackOptions(lua_State *L, luabind::adl::object &e, ExtraAttackOptions *opts) {
	if (opts) {
		luabind::adl::object lua_opts = luabind::newtable(L);
		lua_opts["damage_percent"] = opts->damage_percent;
		lua_opts["damage_flat"] = opts->damage_flat;
		lua_opts["armor_pen_percent"] = opts->armor_pen_percent;
		lua_opts["armor_pen_flat"] = opts->armor_pen_flat;
		lua_opts["crit_percent"] = opts->crit_percent;
		lua_opts["crit_flat"] = opts->crit_flat;
		lua_opts["hate_percent"] = opts->hate_percent;
		lua_opts["hate_flat"] = opts->hate_flat;
		lua_opts["hit_chance"] = opts->hit_chance;
		lua_opts["melee_damage_bonus_flat"] = opts->melee_damage_bonus_flat;
		lua_opts["skilldmgtaken_bonus_flat"] = opts->skilldmgtaken_bonus_flat;
		e["opts"] = lua_opts;
	}
}

void GetExtraAttackOptions(luabind::adl::object &ret, ExtraAttackOptions *opts) {
	if (opts) {
		auto luaOptsTable = ret["opts"];
		if (luabind::type(luaOptsTable) == LUA_TTABLE) {
			auto damage_percent = luaOptsTable["damage_percent"];
			auto damage_flat = luaOptsTable["damage_flat"];
			auto armor_pen_percent = luaOptsTable["armor_pen_percent"];
			auto armor_pen_flat = luaOptsTable["armor_pen_flat"];
			auto crit_percent = luaOptsTable["crit_percent"];
			auto crit_flat = luaOptsTable["crit_flat"];
			auto hate_percent = luaOptsTable["hate_percent"];
			auto hate_flat = luaOptsTable["hate_flat"];
			auto hit_chance = luaOptsTable["hit_chance"];
			auto melee_damage_bonus_flat = luaOptsTable["melee_damage_bonus_flat"];
			auto skilldmgtaken_bonus_flat = luaOptsTable["skilldmgtaken_bonus_flat"];

			if (luabind::type(damage_percent) == LUA_TNUMBER) {
				opts->damage_percent = luabind::object_cast<float>(damage_percent);
			}

			if (luabind::type(damage_flat) == LUA_TNUMBER) {
				opts->damage_flat = luabind::object_cast<int>(damage_flat);
			}

			if (luabind::type(armor_pen_percent) == LUA_TNUMBER) {
				opts->armor_pen_percent = luabind::object_cast<float>(armor_pen_percent);
			}

			if (luabind::type(armor_pen_flat) == LUA_TNUMBER) {
				opts->armor_pen_flat = luabind::object_cast<int>(armor_pen_flat);
			}

			if (luabind::type(crit_percent) == LUA_TNUMBER) {
				opts->crit_percent = luabind::object_cast<float>(crit_percent);
			}

			if (luabind::type(crit_flat) == LUA_TNUMBER) {
				opts->crit_flat = luabind::object_cast<float>(crit_flat);
			}

			if (luabind::type(hate_percent) == LUA_TNUMBER) {
				opts->hate_percent = luabind::object_cast<float>(hate_percent);
			}

			if (luabind::type(hate_flat) == LUA_TNUMBER) {
				opts->hate_flat = luabind::object_cast<int>(hate_flat);
			}

			if (luabind::type(hit_chance) == LUA_TNUMBER) {
				opts->hit_chance = luabind::object_cast<int>(hit_chance);
			}

			if (luabind::type(melee_damage_bonus_flat) == LUA_TNUMBER) {
				opts->melee_damage_bonus_flat = luabind::object_cast<int>(melee_damage_bonus_flat);
			}

			if (luabind::type(skilldmgtaken_bonus_flat) == LUA_TNUMBER) {
				opts->skilldmgtaken_bonus_flat = luabind::object_cast<int>(skilldmgtaken_bonus_flat);
			}
		}
	}
}

void LuaMod::MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_melee_mitigation) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "MeleeMitigation");

		Lua_Mob l_self(self);
		Lua_Mob l_other(attacker);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		PutExtraAttackOptions(L, e, opts);

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_apply_damage_table) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "ApplyDamageTable");

		Lua_Mob l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;

		PutDamageHitInfo(L, e, hit);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_avoid_damage) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "AvoidDamage");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TBOOLEAN) {
				returnValue = luabind::object_cast<bool>(returnValueObj);
			}

			GetDamageHitInfo(ret, hit);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_check_hit_chance) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "CheckHitChance");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TBOOLEAN) {
				returnValue = luabind::object_cast<bool>(returnValueObj);
			}

			GetDamageHitInfo(ret, hit);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::CommonOutgoingHitSuccess(Mob *self, Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_common_outgoing_hit_success) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "CommonOutgoingHitSuccess");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		PutExtraAttackOptions(L, e, opts);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_try_critical_hit) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "TryCriticalHit");

		Lua_Mob l_self(self);
		Lua_Mob l_other(defender);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		PutExtraAttackOptions(L, e, opts);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::GetRequiredAAExperience(Client *self, uint32 &returnValue, bool &ignoreDefault)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_get_required_aa_experience) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "GetRequiredAAExperience");

		Lua_Client l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<uint32>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::GetEXPForLevel(Client *self, uint16 level, uint32 &returnValue, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_get_exp_for_level) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "GetEXPForLevel");

		Lua_Client l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["level"] = level;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<uint32>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::GetExperienceForKill(Client *self, Mob *against, uint32 &returnValue, bool &ignoreDefault)
{
	int start = lua_gettop(L);
	uint32 retval = 0;

	try {
		if (!m_has_get_experience_for_kill) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "GetExperienceForKill");

		Lua_Client l_self(self);
		Lua_Mob l_other(against);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<uint32>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

#endif
