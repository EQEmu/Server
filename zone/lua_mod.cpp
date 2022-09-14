#ifdef LUA_EQEMU

#include <sol/sol.hpp>

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

void PutDamageHitInfo(sol::state_view &sv, sol::table &e, DamageHitInfo &hit) {
	auto lua_hit = sv.create_table();
	lua_hit["base_damage"] = hit.base_damage;
	lua_hit["min_damage"] = hit.min_damage;
	lua_hit["damage_done"] = hit.damage_done;
	lua_hit["offense"] = hit.offense;
	lua_hit["tohit"] = hit.tohit;
	lua_hit["hand"] = hit.hand;
	lua_hit["skill"] = (int)hit.skill;
	e["hit"] = lua_hit;
}

void GetDamageHitInfo(sol::table &ret, DamageHitInfo &hit) {
	auto luaHitTable = ret.get<sol::table>("hit");
	hit.base_damage = luaHitTable.get<int>("base_damage");
	hit.min_damage = luaHitTable.get<int>("min_damage");
	hit.damage_done = luaHitTable.get<int>("damage_done");
	hit.offense = luaHitTable.get<int>("offense");
	hit.tohit = luaHitTable.get<int>("tohit");
	hit.hand = luaHitTable.get<int>("hand");
	hit.skill = static_cast<EQ::skills::SkillType>(luaHitTable.get<int>("skill"));
}

void PutExtraAttackOptions(sol::state_view &sv, sol::table &e, ExtraAttackOptions *opts) {
	if (opts) {
		auto lua_opts = sv.create_table();
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

void GetExtraAttackOptions(sol::table &ret, ExtraAttackOptions *opts) {
	if (opts) {
		auto opts_optional = ret.get<sol::optional<sol::table>>("opts");
		if (opts_optional) {
			auto table = opts_optional.value();
			opts->damage_percent = table.get_or("damage_percent", 1.0f);
			opts->damage_flat = table.get_or("damage_flat", 0);
			opts->armor_pen_percent = table.get_or("armor_pen_percent", 0.0f);
			opts->armor_pen_flat = table.get_or("armor_pen_flat", 0);
			opts->crit_percent = table.get_or("crit_percent", 1.0f);
			opts->crit_flat = table.get_or("crit_flat", 0.0f);
			opts->hate_percent = table.get_or("hate_percent", 1.0f);
			opts->hate_flat = table.get_or("hate_flat", 0);
			opts->hit_chance = table.get_or("hit_chance", 0);
			opts->melee_damage_bonus_flat = table.get_or("melee_damage_bonus_flat", 0);
			opts->skilldmgtaken_bonus_flat = table.get_or("skilldmgtaken_bonus_flat", 0);
		}
	}
}

void LuaMod::MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault) {
	try {
		if (!m_has_melee_mitigation) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("MeleeMitigation");

		Lua_Mob l_self(self);
		Lua_Mob l_other(attacker);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(sv, e, hit);
		PutExtraAttackOptions(sv, e, opts);

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}
		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault) {
	try {
		if (!m_has_apply_damage_table) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("ApplyDamageTable");

		Lua_Mob l_self(self);
		auto e = sv.create_table();
		e["self"] = l_self;

		PutDamageHitInfo(sv, e, hit);

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}
			GetDamageHitInfo(ret, hit);
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault) {
	try {
		if (!m_has_avoid_damage) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("AvoidDamage");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(sv, e, hit);

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}

			auto return_value = ret.get<sol::optional<bool>>("ReturnValue");
			if (return_value) {
				returnValue = return_value.value();
			}
			GetDamageHitInfo(ret, hit);
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault) {
	try {
		if (!m_has_check_hit_chance) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("CheckHitChance");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(sv, e, hit);

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}

			auto return_value = ret.get<sol::optional<bool>>("ReturnValue");
			if (return_value) {
				returnValue = return_value.value();
			}

			GetDamageHitInfo(ret, hit);
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::CommonOutgoingHitSuccess(Mob *self, Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	try {
		if (!m_has_common_outgoing_hit_success) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("CommonOutgoingHitSuccess");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(sv, e, hit);
		PutExtraAttackOptions(sv, e, opts);

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault) {
	try {
		if (!m_has_try_critical_hit) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("TryCriticalHit");

		Lua_Mob l_self(self);
		Lua_Mob l_other(defender);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(sv, e, hit);
		PutExtraAttackOptions(sv, e, opts);

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::GetRequiredAAExperience(Client *self, uint32 &returnValue, bool &ignoreDefault)
{
	try {
		if (!m_has_get_required_aa_experience) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("GetRequiredAAExperience");

		Lua_Client l_self(self);
		auto e = sv.create_table();
		e["self"] = l_self;

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}
			auto return_value = ret.get<sol::optional<uint32_t>>("ReturnValue");
			if (return_value) {
				returnValue = return_value.value();
			}
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::GetEXPForLevel(Client *self, uint16 level, uint32 &returnValue, bool &ignoreDefault) {
	try {
		if (!m_has_get_exp_for_level) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("GetEXPForLevel");

		Lua_Client l_self(self);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["level"] = level;

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}
			auto return_value = ret.get<sol::optional<uint32_t>>("ReturnValue");
			if (return_value) {
				returnValue = return_value.value();
			}
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

void LuaMod::GetExperienceForKill(Client *self, Mob *against, uint32 &returnValue, bool &ignoreDefault)
{
	try {
		if (!m_has_get_experience_for_kill) {
			return;
		}

		sol::state_view sv = env->lua_state();
		auto f = env->get<sol::protected_function>("GetExperienceForKill");

		Lua_Client l_self(self);
		Lua_Mob l_other(against);
		auto e = sv.create_table();
		e["self"] = l_self;
		e["other"] = l_other;

		auto result = f(e);
		if (!result.valid()) {
			sol::error error = result;
			parser_->AddError(error.what());
			return;
		}

		if (result.get_type() == sol::type::table) {
			auto ret = result.get<sol::table>();
			auto ignore_default = ret.get<sol::optional<bool>>("IgnoreDefault");
			if (ignore_default) {
				ignoreDefault = ignoreDefault || ignore_default.value();
			}
			auto return_value = ret.get<sol::optional<uint32_t>>("ReturnValue");
			if (return_value) {
				returnValue = return_value.value();
			}
		}
	} catch (sol::error &error) {
		parser_->AddError(error.what());
	}
}

#endif
