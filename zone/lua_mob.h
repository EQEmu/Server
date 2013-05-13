#ifndef EQEMU_LUA_MOB_H
#define EQEMU_LUA_MOB_H
#ifdef LUA_EQEMU

#include "lua_entity.h"

class Mob;

class Lua_Mob : public Lua_Entity
{
	typedef Mob NativeType;
public:
	Lua_Mob() { d_ = nullptr; }
	Lua_Mob(NativeType *d) { d_ = d; }
	virtual ~Lua_Mob() { }

	operator NativeType* () {
		if(d_) {
			return reinterpret_cast<NativeType*>(d_);
		}

		return nullptr;
	}

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

	/*bool CastSpell(int spell_id, int target_id);
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
	bool IsImmuneToSpell(int spell_id, Lua_Mob caster);
	void BuffFadeBySpellID(int spell_id);
    void BuffFadeByEffect(int effect_id, int skip_slot);
    void BuffFadeAll();
    void BuffFadeDetrimental();
    void BuffFadeBySlot(int slot, bool recalc_bonuses);
    void BuffFadeDetrimentalByCaster(Lua_Mob caster);
    void BuffFadeBySitModifier();
    void BuffModifyDurationBySpellID(int spell_id, int new_duration);
	int CanBuffStack(int spell_id, int caster_level, bool fail_if_overwrite);
	bool IsCasting();
	int CastingSpellID();
	void SetAppearance(int app);
	void SetAppearance(int app, bool ignore_self);
	int GetAppearance();
	int GetPetID();
	int GetOwnerID();
	void SetPetType(int type);
	int GetPetType();
	int GetBodyType();
	void SetBodyType(int type);
	void Stun(int duration);
	void UnStun();
	void Spin();
	void Kill();
	void SetInvul(bool invul);
	bool GetInvul();
	void SetExtraHaste(int haste);
	int GetHaste();
	int GetMonkHandToHandDamage();
	int GetMonkHandToHandDelay();
	bool CanThisClassDoubleAttack();
	bool CanThisClassDualWield();
	bool CanThisClassRiposte();
	bool CanThisClassDodge();
	bool CanThisClassParry();
	bool CanThisClassBlock();
	int GetClassLevelFactor();
	void Mesmerize();
	bool IsMezzed();
	bool IsStunned();
	void StartEnrage();
	bool IsEnraged();
	int GetReverseFactionCon(Lua_Mob other);
	bool IsAIControlled();
	double GetAggroRange();
	double GetAssistRange();
	void SetPetOrder(int order);
	int GetPetOrder();
	bool IsRoamer();
	bool IsRooted();
	void AddToHateList(Lua_Mob other);
	void AddToHateList(Lua_Mob other, int hate);
	void AddToHateList(Lua_Mob other, int hate, int damage);
	void AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help);
	void AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help, bool frenzy);
	void AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help, bool frenzy, bool buff_tic);
	bool RemoveFromHateList(Lua_Mob mob);
	void SetHate(Lua_Mob other);
	void SetHate(Lua_Mob other, int hate);
	void SetHate(Lua_Mob other, int hate, int damage);
	uint32 GetHateAmount(Lua_Mob tmob);
	uint32 GetHateAmount(Lua_Mob tmob, bool is_damage);
	uint32 GetDamageAmount(Lua_Mob tmob);
	Lua_Mob GetHateTop();
	Lua_Mob GetHateDamageTop(Lua_Mob other);
	Lua_Mob GetHateRandom();
	Lua_Mob GetHateMost();
	bool IsEngaged();
	bool HateSummon();
	void FaceTarget();
	void FaceTarget(Lua_Mob mob_to_face);
	void SetHeading(double new_h);
	void WipeHateList();
	double CalculateHeadingToTarget(double x, double y);
	double CalculateDistance(double x, double y, double z);
	void SendTo(double new_x, double new_y, double new_z);
    void SendToFixZ(double new_x, double new_y, double new_z);
	void NPCSpecialAttacks(const char* parse, int permtag);
	void NPCSpecialAttacks(const char* parse, int permtag, bool reset);
	void NPCSpecialAttacks(const char* parse, int permtag, bool reset, bool remove);
	int GetResist(int type);
	bool Charmed();
	uint32 GetLevelHP(int level);
	uint32 GetAA(int level);
	int CheckAggroAmount(int spell_id);
	int CheckAggroAmount(int spell_id, bool is_proc);
	int CheckHealAggroAmount(int spell_id);
	int CheckHealAggroAmount(int spell_id, uint32 heal_possible);
	bool DivineAura();
	void SetOOCRegen(int new_regen);
	const char* GetEntityVariable(const char *id);
	void SetEntityVariable(const char *id, const char *m_var);
	bool EntityVariableExists(const char *id);
	bool CombatRange(Lua_Mob other);
	void DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage);
	void DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage, int hate_override);
	void DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage, int hate_override, int reuse_time);
	void DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage, int hate_override, int reuse_time, bool hit_chance);
	void DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse);
	void DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse, int chance_mod);
	void DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse, int chance_mod, int focus);
	void DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse, int chance_mod, int focus, bool can_riposte);
	bool CheckLoS(Lua_Mob other); // make sure this is CheckLoSFN
	bool CheckLoSToLoc(double x, double y, double z);
	bool CheckLoSToLoc(double x, double y, double z, double mob_size);
	double FindGroundZ(double new_x, double new_y);
	double FindGroundZ(double new_x, double new_y, double z_offset);
	void ProjectileAnimation(Lua_Mob to, int item_id);
	void ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow);
	void ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed);
	void ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle);
	void ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt);
	void ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt, double arc);
	bool HasNPCSpecialAtk(const char* parse);
	void SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5);
	void SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Lua_Client client);
	void SetFlyMode(int value);
	void SetTexture(int value);
	void SetRace(int value);
	void SetGender(int value);
	void SendIllusionPacket(int in_race, int in_gender, int in_texture, int in_helmtexture, int in_haircolor, int in_beardcolor, 
		int in_eyecolor1, int in_eyecolor2, int in_hairstyle, int in_luclinface, int in_beard, int in_aa_title, 
		uint32 in_drakkin_heritage, uint32 in_drakkin_tattoo, uint32 in_drakkin_details, double in_size);
	void QuestReward(Lua_Client c);
	void QuestReward(Lua_Client c, uint32 silver);
	void QuestReward(Lua_Client c, uint32 silver, uint32 gold);
	void QuestReward(Lua_Client c, uint32 silver, uint32 gold, uint32 platinum);
	void CameraEffect(uint32 duration, uint32 intensity);
	void SendSpellEffect(uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20, bool perm_effect, 
		Lua_Client client);
	void TempName(const char *new_name);
	void SetGlobal(const char *var_name, const char *new_value, int options, const char *duration);
	void SetGlobal(const char *var_name, const char *new_value, int options, const char *duration, Lua_Mob other);
    void TarGlobal(const char *var_name, const char *value, const char *duration, int npc_id, int char_id, int zone_id);
    void DelGlobal(const char *var_name);
	void SetSlotTint(int slot, int red, int green, int blue);
	void WearChange(int material_slot, int texture, uint32 color);
	void DoKnockback(Lua_Mob caster, uint32 pushback, uint32 pushup);
	void RemoveNimbusEffect(int effect_id);
	bool IsRunning();
	void SetRunning(bool v);
	void SetTargetable(bool v);
	void ModSkillDmgTaken(int skill_num, int value);
	int GetModSkillDmgTaken(int skill_num);
	int GetSkillDmgTaken(int skill_num);
	void SetAllowBeneficial(bool v);
	bool GetAllowBeneficial();
	bool IsBeneficialAllowed(Lua_Mob target);
	void ModVulnerability(int resist, int value);
	int GetModVulnerability(int resist);
	void SetDisableMelee(bool v);
	bool IsMeleeDisabled();
	void SetFlurryChance(int v);
	int GetFlurryChance();
	*/

	//GetHateList - Requires classes not yet exported
	//SignalClient - Gonna do this differently
	//DoArcheryAttackDmg - Requires classes not yet exported
	//DoThrowingAttackDmg - Requires classes not yet exported
};

#endif
#endif