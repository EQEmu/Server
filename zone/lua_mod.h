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
	bool AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault);
	bool CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &ignoreDefault);
	void TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);

private:
	LuaParser *parser_;
	lua_State *L;
	std::string package_name_;

	bool m_has_melee_mitigation;
	bool m_has_apply_damage_table;
	bool m_has_avoid_damage;
	bool m_has_check_hit_chance;
	bool m_has_try_critical_hit;
};
