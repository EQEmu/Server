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
	void SendPosUpdate();
	void SendPosUpdate(bool send_to_self);
	void SendPosition();
	bool HasProcs();
	bool IsInvisible();
	bool IsInvisible(Lua_Mob other);
    void SetInvisible(int state);
	bool FindBuff(int spell_id);
    bool FindType(int type);
	bool FindType(int type, bool offensive);
	bool FindType(int type, bool offensive, int threshold);
	int GetBuffSlotFromType(int slot);
	void MakePet(int spell_id, const char* pet_type);
	void MakePet(int spell_id, const char* pet_type, const char *pet_name);
	void MakePoweredPet(int spell_id, const char* pet_type, int pet_power);
	void MakePoweredPet(int spell_id, const char* pet_type, int pet_power, const char *pet_name);
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
	double GetActSpellRange(int spell_id, double range);
	double GetActSpellRange(int spell_id, double range, bool is_bard);
    int GetActSpellDamage(int spell_id, int value);
    int GetActSpellHealing(int spell_id, int value);
    int GetActSpellCost(int spell_id, int cost);
    int GetActSpellDuration(int spell_id, int duration);
    int GetActSpellCasttime(int spell_id, int cast_time);
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
	void SetFollowID(int id);
	int GetFollowID();
	void Message(int type, const char *message);
	void Message_StringID(int type, int string_id, uint32 distance);
	void Say(const char *message);
	void Shout(const char *message);
	void Emote(const char *message);
	void InterruptSpell();
	void InterruptSpell(int spell_id);
	//CastSpell
	//SpellFinished
	//IsImmuneToSpell
	//BuffFadeBySpellID
	//BuffFadeByEffect
	//BuffFadeAll
	//BuffFadeBySlot
	//CanBuffStack
	//IsCasting
	//CastingSpellID
	//SetAppearance
	//GetAppearance
	//GetRunAnimSpeed
	//SetRunAnimSpeed
	//SetPetID
	//GetPetID
	//SetOwnerID
	//GetOwnerID
	//GetPetType
	//GetBodyType
	//Stun
	//Spin
	//Kill
	//SetInvul
	//GetInvul
	//SetExtraHaste
	//GetHaste
	//GetMonkHandToHandDamage
	//CanThisClassDoubleAttack
	//CanThisClassDualWield
	//CanThisClassRiposte
	//CanThisClassDodge
	//CanThisClassParry
	//GetMonkHandToHandDelay
	//GetClassLevelFactor
	//Mesmerize
	//IsMezzed
	//IsStunned
	//StartEnrage
	//IsEnraged
	//GetReverseFactionCon
	//IsAIControlled
	//GetAggroRange
	//GetAssistRange
	//SetPetOrder
	//GetPetOrder
	//IsRoamer
	//IsRooted
	//AddToHateList
	//SetHate
	//GetHateAmount
	//GetDamageAmount
	//GetHateTop
	//GetHateDamageTop
	//GetHateRandom
	//IsEngaged
	//HateSummon
	//FaceTarget
	//SetHeading
	//WipeHateList
	//CheckAggro
	//CalculateHeadingToTarget
	//CalculateNewPosition
	//CalculateNewPosition2
	//CalculateDistance
	//SendTo
	//SendToFixZ
	//NPCSpecialAttacks
	//DontHealMeBefore
	//DontBuffMeBefore
	//DontDotMeBefore
	//DontRootMeBefore
	//DontSnareMeBefore
	//GetResist
	//GetShieldTarget
	//SetShieldTarget
	//Charmed
	//GetLevelHP
	//GetZoneID
	//CheckAggroAmount
	//CheckHealAggroAmount
	//GetAA
	//DivineAura
	//AddFeignMemory
	//RemoveFromFeignMemory
	//ClearFeignMemory
	//SetOOCRegen
	//GetEntityVariable
	//SetEntityVariable
	//EntityVariableExists
	//GetHateList
	//SignalClient
	//CombatRange
	//DoSpecialAttackDamage
	//CheckLoS
	//CheckLoSToLoc
	//FindGroundZ
	//ProjectileAnim
	//HasNPCSpecialAtk
	//SendAppearanceEffect
	//SetFlyMode
	//SetTexture
	//SetRace
	//SetGender
	//SendIllusion
	//MakeTempPet
	//QuestReward
	//CameraEffect
	//SpellEffect
	//TempName
	//GetItemStat
	//SetGlobal
	//TarGlobal
	//DelGlobal
	//SetSlotTint
	//WearChange
	//DoKnockback
	//RemoveNimbusEffect
	//IsRunning
	//SetRunning
	//SetBodyType
	//SetDeltas
	//SetLD
	//SetTargetDestSteps
	//SetTargetable
	//MakeTempPet
	//ModSkillDmgTaken
	//GetModSkillDmgTaken
	//GetSkillDmgTaken
	//SetAllowBeneficial
	//GetAllowBeneficial
	//IsBeneficialAllowed
	//ModVulnerability
	//GetModVulnerability
	//DoMeleeSkillAttackDmg
	//DoArcheryAttackDmg
	//DoThrowingAttackDmg
	//SetDisableMelee
	//IsMeleeDisabled
	//SetFlurryChance
	//GetFlurryChance

};

#endif
#endif