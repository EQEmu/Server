#pragma once

#include <string>

struct lua_State;

class LuaParser;
class LuaMod
{
public:
	LuaMod(lua_State *ls, LuaParser *lp, const std::string &package_name) {
		L = ls; 
		parser_ = lp;
		package_name_ = package_name;
		Init();
	}
	~LuaMod() { }
	void Init();

	void MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault);
	void AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault);
	void CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault);
	void CommonOutgoingHitSuccess(Mob *self, Mob* other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void GetRequiredAAExperience(Client *self, uint32 &returnValue, bool &ignoreDefault);
	void GetEXPForLevel(Client *self, uint16 level, uint32 &returnValue, bool &ignoreDefault);
	void GetExperienceForKill(Client *self, Mob *against, uint32 &returnValue, bool &ignoreDefault);
	void ClientDamage(Client *self, Mob *other, int32 &in_damage, uint16 &spell_id,  int &attack_skill, bool &avoidable, int8 &buffslot, bool &iBuffTic, int &special, int32 &out_damage, bool &ignoreDefault);
	void PVPResistSpell(Client *self, uint8 &resist_type, uint16 &spell_id, Client *caster, bool &use_resist_override, int &resist_override, bool &CharismaCheck, bool &CharmTick, bool &IsRoot, int &level_override, float &returnValue, bool &ignoreDefault);
private:
	LuaParser *parser_;
	lua_State *L;
	std::string package_name_;

	bool m_has_melee_mitigation;
	bool m_has_apply_damage_table;
	bool m_has_avoid_damage;
	bool m_has_check_hit_chance;
	bool m_has_common_outgoing_hit_success;
	bool m_has_try_critical_hit;
	bool m_has_get_required_aa_experience;
	bool m_has_get_exp_for_level;
	bool m_has_get_experience_for_kill;
	bool m_has_client_damage;
	bool m_has_pvp_resist_spell;
};
