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

	bool ClientAttack(Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	bool NPCAttack(Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	bool BotAttack(Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	bool CommonAttack(const std::string &fn, Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	void MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault);
	bool AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault);
	bool CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &ignoreDefault);
	void DoSpecialAttackDamage(Mob *self, Mob *who, EQEmu::skills::SkillType skill, int32 base_damage, int32 min_damage, int32 hate_override, int ReuseTime, bool &ignoreDefault);
	void DoRangedAttackDmg(Mob *self, Mob* other, bool Launch, int16 damage_mod, int16 chance_mod, EQEmu::skills::SkillType skill, float speed, const char *IDFile, bool &ignoreDefault);
	void DoArcheryAttackDmg(Mob *self, Mob *other, const EQEmu::ItemInstance *RangeWeapon, const EQEmu::ItemInstance *Ammo, uint16 weapon_damage, int16 chance_mod, int16 focus,
		int ReuseTime, uint32 range_id, uint32 ammo_id, const EQEmu::ItemData *AmmoItem, int AmmoSlot, float speed, bool &ignoreDefault);
	void DoThrowingAttackDmg(Mob *self, Mob *other, const EQEmu::ItemInstance *RangeWeapon, const EQEmu::ItemData *AmmoItem, uint16 weapon_damage, int16 chance_mod, int16 focus,
		int ReuseTime, uint32 range_id, int AmmoSlot, float speed, bool &ignoreDefault);
	void DoMeleeSkillAttackDmg(Mob *self, Mob *other, uint16 weapon_damage, EQEmu::skills::SkillType skillinuse, int16 chance_mod, int16 focus, bool CanRiposte, int ReuseTime,
		bool &ignoreDefault);
private:
	LuaParser *parser_;
	lua_State *L;
	std::string package_name_;

	bool m_has_client_attack;
	bool m_has_npc_attack;
	bool m_has_bot_attack;
	bool m_has_melee_mitigation;
	bool m_has_apply_damage_table;
	bool m_has_avoid_damage;
	bool m_has_check_hit_chance;
	bool m_has_do_special_attack_damage;
	bool m_has_do_ranged_attack_dmg;
	bool m_has_do_archery_attack_dmg;
	bool m_has_do_throwing_attack_dmg;
	bool m_has_do_melee_skill_attack_dmg;
};
