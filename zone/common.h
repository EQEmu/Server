#ifndef __EQEMU_ZONE_COMMON_H
#define __EQEMU_ZONE_COMMON_H

#include "../common/types.h"
#include "../common/spdat.h"

#define	HIGHEST_RESIST 9
#define MAX_SPELL_PROJECTILE 10
#define _CLIENT(x) (x && x->IsClient() && !x->CastToClient()->IsBecomeNPC())
#define _NPC(x) (x && x->IsNPC() && !x->CastToMob()->GetOwnerID())
#define _BECOMENPC(x) (x && x->IsClient() && x->CastToClient()->IsBecomeNPC())
#define _CLIENTCORPSE(x) (x && x->IsCorpse() && x->CastToCorpse()->IsPlayerCorpse())
#define _NPCCORPSE(x) (x && x->IsCorpse() && (x->CastToCorpse()->IsNPCCorpse()))
#define _CLIENTPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient())
#define _NPCPET(x) (x && x->IsNPC() && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsNPC())
#define _BECOMENPCPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient() && x->CastToMob()->GetOwner()->CastToClient()->IsBecomeNPC())
#define HEAD_POSITION 0.9f
#define SEE_POSITION 0.5f
#define CHECK_LOS_STEP 1.0f
#define MAX_SHIELDERS 2
#define ARCHETYPE_HYBRID 1
#define ARCHETYPE_CASTER 2
#define ARCHETYPE_MELEE	3
#define CON_GREEN 2
#define CON_LIGHTBLUE 18
#define CON_BLUE 4
#define CON_WHITE 20
#define CON_YELLOW 15
#define CON_RED	13
#define SPECIALIZE_FIZZLE 11
#define SPECIALIZE_MANA_REDUCE 12
#define ZONEPOINT_NOZONE_RANGE 40000.0f
#define ZONEPOINT_ZONE_RANGE 40000.0f

typedef enum {
	focusSpellHaste = 1,
	focusSpellDuration,
	focusRange,
	focusReagentCost,
	focusManaCost,
	focusImprovedHeal,
	focusImprovedDamage,
	focusImprovedDOT,
	focusFcDamagePctCrit,
	focusImprovedUndeadDamage,
	focusPetPower,
	focusResistRate,
	focusSpellHateMod,
	focusTriggerOnCast,
	focusSpellVulnerability,
	focusTwincast,
	focusSympatheticProc,
	focusFcDamageAmt,
	focusFcDamageAmtCrit,
	focusSpellDurByTic,
	focusSwarmPetDuration,
	focusReduceRecastTime,
	focusBlockNextSpell,
	focusFcHealPctIncoming,
	focusFcDamageAmtIncoming,
	focusFcHealAmtIncoming,
	focusFcBaseEffects,
	focusIncreaseNumHits,
	focusFcLimitUse,
	focusFcMute,
	focusFcTimerRefresh,
	focusFcStunTimeMod,
	focusFcHealPctCritIncoming,
	focusFcHealAmt,
	focusFcHealAmtCrit,
} focusType;
#define HIGHEST_FOCUS	focusFcHealAmtCrit

enum {
	SPECATK_SUMMON = 1,
	SPECATK_ENRAGE = 2,
	SPECATK_RAMPAGE = 3,
	SPECATK_AREA_RAMPAGE = 4,
	SPECATK_FLURRY = 5,
	SPECATK_TRIPLE = 6,
	SPECATK_QUAD = 7,
	SPECATK_INNATE_DW = 8,
	SPECATK_BANE = 9,
	SPECATK_MAGICAL = 10,
	SPECATK_RANGED_ATK = 11,
	UNSLOWABLE = 12,
	UNMEZABLE = 13,
	UNCHARMABLE = 14,
	UNSTUNABLE = 15,
	UNSNAREABLE = 16,
	UNFEARABLE = 17,
	UNDISPELLABLE = 18,
	IMMUNE_MELEE = 19,
	IMMUNE_MAGIC = 20,
	IMMUNE_FLEEING = 21,
	IMMUNE_MELEE_EXCEPT_BANE = 22,
	IMMUNE_MELEE_NONMAGICAL = 23,
	IMMUNE_AGGRO = 24,
	IMMUNE_AGGRO_ON = 25,
	IMMUNE_CASTING_FROM_RANGE = 26,
	IMMUNE_FEIGN_DEATH = 27,
	IMMUNE_TAUNT = 28,
	NPC_TUNNELVISION = 29,
	NPC_NO_BUFFHEAL_FRIENDS = 30,
	IMMUNE_PACIFY = 31,
	LEASH = 32,
	TETHER = 33,
	DESTRUCTIBLE_OBJECT = 34,
	NO_HARM_FROM_CLIENT = 35,
	ALWAYS_FLEE = 36,
	FLEE_PERCENT = 37,
	ALLOW_BENEFICIAL = 38,
	DISABLE_MELEE = 39,
	MAX_SPECIAL_ATTACK = 40
	
};

typedef enum {
	fearStateNotFeared = 0,
	fearStateRunning,
	fearStateRunningForever,
	fearStateGrid,
	fearStateStuck
} FearState;

enum { FlyMode0 = 0, FlyMode1 = 1, Flymode2 = 2, FlyMode3 = 3 };

struct TradeEntity;
class Trade;
enum TradeState {
	TradeNone,
	Trading,
	TradeAccepted,
	TradeCompleting
};


struct Buffs_Struct {
	uint16	spellid;
	uint8	casterlevel;
	uint16	casterid;
	char	caster_name[64];
	int32	ticsremaining;
	uint32	counters;
	uint32	numhits;
	uint32	melee_rune;
	uint32	magic_rune;
	uint32	dot_rune;
	int32	caston_x;
	int32	caston_y;
	int32	caston_z;
	int32	ExtraDIChance;
	int16	RootBreakChance;
	bool	persistant_buff;
	bool	client;
	bool	UpdateClient;
};

struct StatBonuses {
	int16	AC;
	int32	HP;
	int32	HPRegen;
	int32	MaxHP;
	int32	ManaRegen;
	int32	EnduranceRegen;
	int32	Mana;
	int32	Endurance;
	int16	ATK;
	int16	STR;
	int16	STRCapMod;
	int16	HeroicSTR;
	int16	STA;
	int16	STACapMod;
	int16	HeroicSTA;
	int16	DEX;
	int16	DEXCapMod;
	int16	HeroicDEX;
	int16	AGI;
	int16	AGICapMod;
	int16	HeroicAGI;
	int16	INT;
	int16	INTCapMod;
	int16	HeroicINT;
	int16	WIS;
	int16	WISCapMod;
	int16	HeroicWIS;
	int16	CHA;
	int16	CHACapMod;
	int16	HeroicCHA;
	int16	MR;
	int16	MRCapMod;
	int16	HeroicMR;
	int16	FR;
	int16	FRCapMod;
	int16	HeroicFR;
	int16	CR;
	int16	CRCapMod;
	int16	HeroicCR;
	int16	PR;
	int16	PRCapMod;
	int16	HeroicPR;
	int16	DR;
	int16	DRCapMod;
	int16	HeroicDR;
	int16	Corrup;
	int16	CorrupCapMod;
	int16	HeroicCorrup;
	uint16	DamageShieldSpellID;
	int DamageShield;
	DmgShieldType	DamageShieldType;
	int SpellDamageShield;
	int SpellShield;
	int ReverseDamageShield;
	uint16	ReverseDamageShieldSpellID;
	DmgShieldType	ReverseDamageShieldType;
	int movementspeed;
	int16	haste;
	int16	hastetype2;
	int16	hastetype3;
	int16	inhibitmelee;
	float	AggroRange;
	float	AssistRange;
	int16	skillmod[HIGHEST_SKILL+1];
	int effective_casting_level;
	int reflect_chance;
	uint16	singingMod;
	uint16	Amplification;
	uint16	brassMod;
	uint16	percussionMod;
	uint16	windMod;
	uint16	stringedMod;
	uint16	songModCap;
	int8	hatemod;
	int32	EnduranceReduction;
	int16	StrikeThrough;
	int16	MeleeMitigation;
	int16	CriticalHitChance[HIGHEST_SKILL+2];
	int16	CriticalSpellChance;
	int16	SpellCritDmgIncrease;
	int16	SpellCritDmgIncNoStack;
	int16	DotCritDmgIncrease;
	int16	CriticalHealChance;
	int16	CriticalHealOverTime;
	int16	CriticalDoTChance;
	int16	CrippBlowChance;
	int16	AvoidMeleeChance;
	int16	RiposteChance;
	int16	DodgeChance;
	int16	ParryChance;
	int16	DualWieldChance;
	int16	DoubleAttackChance;
	int16	TripleAttackChance;
	int16   DoubleRangedAttack;
	int16	ResistSpellChance;
	int16	ResistFearChance;
	bool	Fearless;	
	bool	IsFeared;	
	int16	StunResist;	
	int16	MeleeSkillCheck;
	uint8	MeleeSkillCheckSkill;
	int16	HitChance;	
	int16	HitChanceEffect[HIGHEST_SKILL+2];	
	int16	DamageModifier[HIGHEST_SKILL+2];	
	int16	MinDamageModifier[HIGHEST_SKILL+2]; 
	int16	ProcChance;	
	int16	ProcChanceSPA;
	int16	ExtraAttackChance;
	int16	DoTShielding;
	int16	DivineSaveChance[2];
	uint16	DeathSave[4];
	int16	FlurryChance;
	int16	Accuracy[HIGHEST_SKILL+2];
	int16	HundredHands;
	int8	MeleeLifetap;
	int16	HealRate;	
	int32	MaxHPChange;
	int16	SkillDmgTaken[HIGHEST_SKILL+2];
	int32	HealAmt;	
	int32	SpellDmg;	
	int32	Clairvoyance;
	int16	DSMitigation;
	int16	DSMitigationOffHand;
	uint32	SpellTriggers[MAX_SPELL_TRIGGER];	
	uint32	SpellOnKill[MAX_SPELL_TRIGGER*3];	
	uint32	SpellOnDeath[MAX_SPELL_TRIGGER*2];	
	int16	CritDmgMob[HIGHEST_SKILL+2];
	int16	SkillReuseTime[HIGHEST_SKILL+1];	
	int16	SkillDamageAmount[HIGHEST_SKILL+2];	
	int16	TwoHandBluntBlock;
	uint16	ItemManaRegenCap;
	int16	GravityEffect;
	bool	AntiGate;	
	bool	MagicWeapon;
	int16	IncreaseBlockChance;
	uint16	PersistantCasting;
	int		XPRateMod;
	int		HPPercCap[2];
	int 	ManaPercCap[2];
	int 	EndPercCap[2];
	bool	BlockNextSpell;
	bool	ImmuneToFlee;
	uint16	VoiceGraft;	
	int16	SpellProcChance;
	int16	CharmBreakChance;
	int16	SongRange;	
	uint16	HPToManaConvert;
	uint16	FocusEffects[HIGHEST_FOCUS+1];
	bool	NegateEffects;
	int16	SkillDamageAmount2[HIGHEST_SKILL+2];
	uint16	NegateAttacks[2];
	uint16	MitigateMeleeRune[2];
	uint16	MeleeThresholdGuard[3];
	uint16	SpellThresholdGuard[3];
	uint16	MitigateSpellRune[2];
	uint16	MitigateDotRune[2];
	uint32	TriggerMeleeThreshold[3];
	uint32	TriggerSpellThreshold[3];
	uint16	ManaAbsorbPercentDamage[2];
	int16	ShieldBlock;
	int16	BlockBehind;
	bool	CriticalRegenDecay;
	bool	CriticalHealDecay;
	bool	CriticalDotDecay;
	bool	DivineAura;	
	bool	DistanceRemoval;
	int16	ImprovedTaunt[3];
	int8	Root[2];	
	int16	FrenziedDevastation;
	uint16	AbsorbMagicAtt[2];
	uint16	MeleeRune[2];
	bool	NegateIfCombat;
	int8	Screech;	
	int16	AlterNPCLevel;
	int8	Packrat;	
	uint8	BuffSlotIncrease;
	uint16	DelayDeath;	
	int8	BaseMovementSpeed;
	uint8	IncreaseRunSpeedCap;
	int16	DoubleSpecialAttack;
	int16	SpecialAttackKBProc[2];
	uint8	FrontalStunResist;
	int16	BindWound;	
	int16	MaxBindWound;
	int16	ChannelChanceSpells;
	int16	ChannelChanceItems;
	uint8	SeeInvis;	
	uint8	TripleBackstab;
	bool	FrontalBackstabMinDmg;
	uint8	FrontalBackstabChance;
	uint8	ConsumeProjectile;
	uint8	ForageAdditionalItems;
	uint8	SalvageChance;
	uint16	ArcheryDamageModifier;
	bool	SecondaryDmgInc;
	uint16	GiveDoubleAttack;
	int16	SlayUndead[2];
	int16	PetCriticalHit;
	int16	PetAvoidance;
	int16	CombatStability;
	int16	GiveDoubleRiposte[3];
	uint16	RaiseSkillCap[2];
	int16	Ambidexterity;
	int16	PetMaxHP;	
	int16	PetFlurry;	
	uint8	MasteryofPast;
	bool	GivePetGroupTarget;
	int16	RootBreakChance;
	int16	UnfailingDivinity;
	int16	ItemHPRegenCap;
	int16	SEResist[MAX_RESISTABLE_EFFECTS*2];	
	int16	OffhandRiposteFail;
	int16	ItemATKCap;	
	int32	FinishingBlow[2];
	uint16	FinishingBlowLvl[2];
	int16	ShieldEquipHateMod;
	int16	ShieldEquipDmgMod[2];
	bool	TriggerOnValueAmount;
	int8	StunBashChance;
	int8	IncreaseChanceMemwipe;
	int8	CriticalMend;
};

typedef struct
{
	uint16 spellID;
	uint16 chance;
	uint16 base_spellID;
} tProc;

struct Shielders_Struct {
	uint32 shielder_id;
	uint16 shielder_bonus;
};

enum {
	animKick = 1,
	animPiercing = 2,
	anim2HSlashing = 3,
	anim2HWeapon = 4,
	anim1HWeapon = 5,
	animDualWield = 6,
	animTailRake = 7,
	animHand2Hand = 8,
	animShootBow = 9,
	animRoundKick = 11,
	animSwarmAttack = 20,
	animFlyingKick = 45,
	animTigerClaw = 46,
	animEagleStrike = 47,

};


typedef enum {
	petFamiliar,
	petAnimation,
	petOther,
	petCharmed,
	petNPCFollow,
	petHatelist
} PetType;

typedef enum {
	SingleTarget,
	AETarget,
	AECaster,
	GroupSpell,
	CAHateList,
	DirectionalAE,
	CastActUnknown
} CastAction_type;


struct MercType {
	uint32	Type;
	uint32	ClientVersion;
};

struct MercData {
	uint32	MercTemplateID;
	uint32	MercType;
	uint32	MercSubType;
	uint32	CostFormula;
	uint32	ClientVersion;
	uint32	NPCID;
};

class ItemInst;
class Mob;
class Trade
{
public:
	Trade(Mob* in_owner);
	virtual ~Trade();

	void Reset();
	void SetTradeCash(uint32 in_pp, uint32 in_gp, uint32 in_sp, uint32 in_cp);
	void Start(uint32 mob_id, bool initiate_with=true);

	Mob* With();

	void AddEntity(uint16 from_slot_id, uint16 trade_slot_id);
	void LogTrade();

	#if (EQDEBUG >= 9)
void DumpTrade();
	#endif

public:
	TradeState state;
	int32 pp;
	int32 gp;
	int32 sp;
	int32 cp;

private:
	void SendItemData(const ItemInst* inst, int16 dest_slot_id);

	uint32 with_id;
	Mob* owner;
};

struct ExtraAttackOptions {
	ExtraAttackOptions() : damage_percent(1.0f), damage_flat(0), armor_pen_percent(0.0f), armor_pen_flat(0), crit_percent(1.0f), crit_flat(0.0f), hate_percent(1.0f), hate_flat(0) { }

	float damage_percent;
	int damage_flat;
	float armor_pen_percent;
	int armor_pen_flat;
	float crit_percent;
	float crit_flat;
	float hate_percent;
	int hate_flat;
};

#endif