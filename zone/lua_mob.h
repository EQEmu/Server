#ifndef EQEMU_LUA_MOB_H
#define EQEMU_LUA_MOB_H
#ifdef LUA_EQEMU

#include "lua_entity.h"

class Mob;

class Lua_Mob : public Lua_Entity
{
public:
	Lua_Mob() { d_ = nullptr; }
	Lua_Mob(Mob *d) { d_ = d; }
	virtual ~Lua_Mob() { }

	const char *GetName();

	void Depop();
	void Depop(bool start_spawn_timer);
	void RogueAssassinate(Lua_Mob other);
	bool BehindMob();
	bool BehindMob(Lua_Mob other);
	bool BehindMob(Lua_Mob other, float x);
	bool BehindMob(Lua_Mob other, float x, float y);
	void SetLevel(int level);
	void SetLevel(int level, bool command);
	void SendWearChange(int material_slot);
	uint32 GetEquipment(int material_slot);
	int32 GetEquipmentMaterial(int material_slot);
	uint32 GetEquipmentColor(int material_slot);
	uint32 GetArmorTint(int i);
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

	//uint32 GetLevelCon(int other);
	//uint32 GetLevelCon(int my, int other);
	//void SetHP(int hp);
	//void DoAnim(int anim_num);
	//void DoAnim(int anim_num, int type);
	//void DoAnim(int anim_num, int type, bool ackreq);
	//void DoAnim(int anim_num, int type, bool ackreq, eqFilterType filter);
	//void ChangeSize(float in_size);
	//void ChangeSize(float in_size, bool no_restriction);
	//void GMMove(float x, float y, float z);
	//void GMMove(float x, float y, float z, float heading);
	//void GMMove(float x, float y, float z, float heading, bool SendUpdate);
	//void SendPosUpdate();
	//void SendPosUpdate(bool send_to_self);
	//void SendPosition();
	//bool HasProcs();
	//bool IsInvisible();
	//bool IsInvisible(Lua_Mob other);
    //void SetInvisible(int state);
	//bool FindBuff(uint16 spell_id);
    //bool FindType(uint16 type);
	//bool FindType(uint16 type, bool bOffensive);
	//bool FindType(uint16 type, bool bOffensive, uint16 threshold);
	//int GetBuffSlotFromType(int slot);
	//void MakePet(int spell_id, const char* pet_type);
	//void MakePet(int spell_id, const char* pet_type, const char *pet_name);
	//void MakePoweredPet(int spell_id, const char* pet_type);
	//void MakePoweredPet(int spell_id, const char* pet_type, int pet_power);
	//void MakePoweredPet(int spell_id, const char* pet_type, int pet_power, const char *pet_name);
	//int GetBaseRace();
	//int GetBaseGender();
	//int GetDeity();
	//int GetRace();
	//int GetGender();
	//int GetTexture();
	//int GetHelmTexture();
	//int GetHairColor();
	//int GetBeardColor();
	//int GetEyeColor1();
	//int GetEyeColor2();
	//int GetHairStyle();
	//int GetLuclinFace();
	//int GetBeard();
	//int GetDrakkinHeritage();
	//int GetDrakkinTattoo();
	//int GetDrakkinDetails();
	//int GetClass();
	//int GetLevel();
	//const char *GetCleanName();
	//Lua_Mob GetTarget();
	//void SetTarget(Lua_Mob t);
/*

"GetHPRatio"), XS_Mob_GetHPRatio, file, "$");
"IsWarriorClass"), XS_Mob_IsWarriorClass, file, "$");
"GetHP"), XS_Mob_GetHP, file, "$");
"GetMaxHP"), XS_Mob_GetMaxHP, file, "$");
"GetItemHPBonuses"), XS_Mob_GetItemHPBonuses, file, "$");
"GetSpellHPBonuses"), XS_Mob_GetSpellHPBonuses, file, "$");
"GetWalkspeed"), XS_Mob_GetWalkspeed, file, "$");
"GetRunspeed"), XS_Mob_GetRunspeed, file, "$");
"GetCasterLevel"), XS_Mob_GetCasterLevel, file, "$$");
"GetMaxMana"), XS_Mob_GetMaxMana, file, "$");
"GetMana"), XS_Mob_GetMana, file, "$");
"SetMana"), XS_Mob_SetMana, file, "$$");
"GetManaRatio"), XS_Mob_GetManaRatio, file, "$");
"GetAC"), XS_Mob_GetAC, file, "$");
"GetATK"), XS_Mob_GetATK, file, "$");
"GetSTR"), XS_Mob_GetSTR, file, "$");
"GetSTA"), XS_Mob_GetSTA, file, "$");
"GetDEX"), XS_Mob_GetDEX, file, "$");
"GetAGI"), XS_Mob_GetAGI, file, "$");
"GetINT"), XS_Mob_GetINT, file, "$");
"GetWIS"), XS_Mob_GetWIS, file, "$");
"GetCHA"), XS_Mob_GetCHA, file, "$");
"GetMR"), XS_Mob_GetMR, file, "$");
"GetFR"), XS_Mob_GetFR, file, "$");
"GetDR"), XS_Mob_GetDR, file, "$");
"GetPR"), XS_Mob_GetPR, file, "$");
"GetCR"), XS_Mob_GetCR, file, "$");
"GetCorruption"), XS_Mob_GetCR, file, "$");
"GetMaxSTR"), XS_Mob_GetMaxSTR, file, "$");
"GetMaxSTA"), XS_Mob_GetMaxSTA, file, "$");
"GetMaxDEX"), XS_Mob_GetMaxDEX, file, "$");
"GetMaxAGI"), XS_Mob_GetMaxAGI, file, "$");
"GetMaxINT"), XS_Mob_GetMaxINT, file, "$");
"GetMaxWIS"), XS_Mob_GetMaxWIS, file, "$");
"GetMaxCHA"), XS_Mob_GetMaxCHA, file, "$");
*/
};

#endif
#endif