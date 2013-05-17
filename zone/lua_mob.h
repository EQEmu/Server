#ifndef EQEMU_LUA_MOB_H
#define EQEMU_LUA_MOB_H
#ifdef LUA_EQEMU

#include "lua_entity.h"

class Mob;

class Lua_Mob : public Lua_Entity
{
	typedef Mob NativeType;
public:
	Lua_Mob() { }
	Lua_Mob(Mob *d) { SetLuaPtrData(d); }
	virtual ~Lua_Mob() { }

	operator Mob*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Mob*>(d);
		}

		return nullptr;
	}

	const char *GetName();
	void Depop();
	void Depop(bool start_spawn_timer);
	bool BehindMob();
	bool BehindMob(Lua_Mob other);
	bool BehindMob(Lua_Mob other, float x);
	bool BehindMob(Lua_Mob other, float x, float y);
	void SetLevel(int level);
	void SetLevel(int level, bool command);
	void SendWearChange(int material_slot);
	bool IsMoving();
	void GotoBind();
	void Gate();
	bool Attack(Lua_Mob other);
	bool Attack(Lua_Mob other, int hand);
	bool Attack(Lua_Mob other, int hand, bool from_riposte);
	bool Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough);
	bool Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell);
	void Damage(Lua_Mob from, int damage, int spell_id, int attack_skill);
	void Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable);
	void Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot);
	void Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot, bool buff_tic);
	void RangedAttack(Lua_Mob other);
	void ThrowingAttack(Lua_Mob other);
	void Heal();
	void HealDamage(uint32 amount);
	void HealDamage(uint32 amount, Lua_Mob other);
	uint32 GetLevelCon(int other);
	uint32 GetLevelCon(int my, int other);
	void SetHP(int hp);
	void DoAnim(int anim_num);
	void DoAnim(int anim_num, int type);
	void DoAnim(int anim_num, int type, bool ackreq);
	void DoAnim(int anim_num, int type, bool ackreq, int filter);
	void ChangeSize(double in_size);
	void ChangeSize(double in_size, bool no_restriction);
	void GMMove(double x, double y, double z);
	void GMMove(double x, double y, double z, double heading);
	void GMMove(double x, double y, double z, double heading, bool send_update);
	bool HasProcs();
	bool IsInvisible();
	bool IsInvisible(Lua_Mob other);
    void SetInvisible(int state);
	bool FindBuff(int spell_id);
    bool FindType(int type);
	bool FindType(int type, bool offensive);
	bool FindType(int type, bool offensive, int threshold);
	int GetBuffSlotFromType(int slot);
	int GetBaseRace();
	int GetBaseGender();
	int GetDeity();
	int GetRace();
	int GetGender();
	int GetTexture();
	int GetHelmTexture();
	int GetHairColor();
	int GetBeardColor();
	int GetEyeColor1();
	int GetEyeColor2();
	int GetHairStyle();
	int GetLuclinFace();
	int GetBeard();
	int GetDrakkinHeritage();
	int GetDrakkinTattoo();
	int GetDrakkinDetails();
	int GetClass();
	int GetLevel();
	const char *GetCleanName();
	Lua_Mob GetTarget();
	void SetTarget(Lua_Mob t);
	double GetHPRatio();
	bool IsWarriorClass();
	int GetHP();
	int GetMaxHP();
	int GetItemHPBonuses();
	int GetSpellHPBonuses();
	double GetWalkspeed();
	double GetRunspeed();
	int GetCasterLevel(int spell_id);
	int GetMaxMana();
	int GetMana();
	int SetMana(int mana);
	double GetManaRatio();
	int GetAC();
	int GetATK();
	int GetSTR();
	int GetSTA();
	int GetDEX();
	int GetAGI();
	int GetINT();
	int GetWIS();
	int GetCHA();
	int GetMR();
	int GetFR();
	int GetDR();
	int GetPR();
	int GetCR();
	int GetCorruption();
	int GetMaxSTR();
	int GetMaxSTA();
	int GetMaxDEX();
	int GetMaxAGI();
	int GetMaxINT();
	int GetMaxWIS();
	int GetMaxCHA();
	double ResistSpell(int resist_type, int spell_id, Lua_Mob caster);
	double ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override);
	double ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override);
	double ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override, bool charisma_check);
	int GetSpecializeSkillValue(int spell_id);
	int GetNPCTypeID();
	bool IsTargeted();
	double GetX();
	double GetY();
	double GetZ();
	double GetHeading();
	double GetWaypointX();
	double GetWaypointY();
	double GetWaypointZ();
	double GetWaypointH();
	double GetWaypointPause();
	int GetWaypointID();
	void SetCurrentWP(int wp);
	double GetSize();
	void Message(int type, const char *message);
	void Message_StringID(int type, int string_id, uint32 distance);
	void Say(const char *message);
	void QuestSay(Lua_Client client, const char *message);
	void Shout(const char *message);
	void Emote(const char *message);
	void InterruptSpell();
	void InterruptSpell(int spell_id);
	bool CastSpell(int spell_id, int target_id);
	bool CastSpell(int spell_id, int target_id, int slot);
	bool CastSpell(int spell_id, int target_id, int slot, int cast_time);
	bool CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost);
	bool CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot);
	bool CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer, int timer_duration);
	bool CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer, int timer_duration, 
		int resist_adjust);
	bool SpellFinished(int spell_id, Lua_Mob target);
	bool SpellFinished(int spell_id, Lua_Mob target, int slot);
	bool SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used);
	bool SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot);
	bool SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust);
	bool SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust, bool proc);
	void SpellEffect(Lua_Mob caster, int spell_id, double partial);
	Lua_Mob GetPet();
	Lua_Mob GetOwner();
};

#endif
#endif