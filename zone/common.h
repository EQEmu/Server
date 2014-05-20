#ifndef __EQEMU_ZONE_COMMON_H
#define __EQEMU_ZONE_COMMON_H

#include "../common/types.h"
#include "../common/spdat.h"

#define	HIGHEST_RESIST 9 //Max resist type value
#define MAX_SPELL_PROJECTILE 10 //Max amount of spell projectiles that can be active by a single mob.

/* solar: macros for IsAttackAllowed, IsBeneficialAllowed */
#define _CLIENT(x) (x && x->IsClient() && !x->CastToClient()->IsBecomeNPC())
#define _NPC(x) (x && x->IsNPC() && !x->CastToMob()->GetOwnerID())
#define _BECOMENPC(x) (x && x->IsClient() && x->CastToClient()->IsBecomeNPC())
#define _CLIENTCORPSE(x) (x && x->IsCorpse() && x->CastToCorpse()->IsPlayerCorpse() && !x->CastToCorpse()->IsBecomeNPCCorpse())
#define _NPCCORPSE(x) (x && x->IsCorpse() && (x->CastToCorpse()->IsNPCCorpse() || x->CastToCorpse()->IsBecomeNPCCorpse()))
#define _CLIENTPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient())
#define _NPCPET(x) (x && x->IsNPC() && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsNPC())
#define _BECOMENPCPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient() && x->CastToMob()->GetOwner()->CastToClient()->IsBecomeNPC())


//LOS Parameters:
#define HEAD_POSITION 0.9f	//ratio of GetSize() where NPCs see from
#define SEE_POSITION 0.5f	//ratio of GetSize() where NPCs try to see for LOS
#define CHECK_LOS_STEP 1.0f

#define MAX_SHIELDERS 2		//I dont know if this is based on a client limit

#define ARCHETYPE_HYBRID	1
#define ARCHETYPE_CASTER	2
#define ARCHETYPE_MELEE		3

#define CON_GREEN		2
#define CON_LIGHTBLUE	18
#define CON_BLUE		4
#define CON_WHITE		20
#define CON_YELLOW		15
#define CON_RED			13

//Spell specialization parameters, not sure of a better place for them
#define SPECIALIZE_FIZZLE 11		//% fizzle chance reduce at 200 specialized
#define SPECIALIZE_MANA_REDUCE 12	//% mana cost reduction at 200 specialized

//these are large right now because the x,y,z coords of the zone
//lines do not make a lot of sense
//Maximum distance from a zone point given that the request didnt
//know what zone that the line was for
#define ZONEPOINT_NOZONE_RANGE 40000.0f
//Maximum distance from a zone point if zone was specified
#define ZONEPOINT_ZONE_RANGE 40000.0f

typedef enum {	//focus types
	focusSpellHaste = 1,
	focusSpellDuration,
	focusRange,
	focusReagentCost,
	focusManaCost,
	focusImprovedHeal,
	focusImprovedDamage,
	focusImprovedDOT,		//i dont know about this...
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
} focusType; //Any new FocusType needs to be added to the Mob::IsFocus function
#define HIGHEST_FOCUS	focusFcHealAmtCrit //Should always be last focusType in enum

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
	MAX_SPECIAL_ATTACK = 38
	
};

typedef enum {	//fear states
	fearStateNotFeared = 0,
	fearStateRunning,		//I am running, hoping to find a grid at my WP
	fearStateRunningForever,	//can run straight until spell ends
	fearStateGrid,			//I am allready on a fear grid
	fearStateStuck			//I cannot move somehow...
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

//this is our internal representation of the BUFF struct, can put whatever we want in it
struct Buffs_Struct {
	uint16	spellid;
	uint8	casterlevel;
	uint16	casterid;		// Maybe change this to a pointer sometime, but gotta make sure it's 0'd when it no longer points to anything
	char	caster_name[64];
	int32	ticsremaining;
	uint32	counters;
	uint32	numhits; //the number of physical hits this buff can take before it fades away, lots of druid armor spells take advantage of this mixed with powerful effects
	uint32	melee_rune;
	uint32	magic_rune;
	uint32	dot_rune;
	int32	caston_x;
	int32	caston_y;
	int32	caston_z;
	int32	ExtraDIChance;
	int16	RootBreakChance; //Not saved to dbase
	bool	persistant_buff;
	bool	client; //True if the caster is a client
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
	//would it be worth it to create a Stat_Struct?
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
	int		DamageShield;						// this is damage done to mobs that attack this
	DmgShieldType	DamageShieldType;
	int		SpellDamageShield;
	int		SpellShield;
	int		ReverseDamageShield;				// this is damage done to the mob when it attacks
	uint16	ReverseDamageShieldSpellID;
	DmgShieldType	ReverseDamageShieldType;
	int		movementspeed;
	int16	haste;
	int16	hastetype2;
	int16	hastetype3;
	int16	inhibitmelee;
	float	AggroRange;							// when calculate just replace original value with this
	float	AssistRange;
	int16	skillmod[HIGHEST_SKILL+1];
	int		effective_casting_level;
	int		reflect_chance;						// chance to reflect incoming spell
	uint16	singingMod;
	uint16	Amplification;						// stacks with singingMod
	uint16	brassMod;
	uint16	percussionMod;
	uint16	windMod;
	uint16	stringedMod;
	uint16	songModCap;
	int8	hatemod;
	int32	EnduranceReduction;

	int16	StrikeThrough;						// PoP: Strike Through %
	int16	MeleeMitigation;					//i = Shielding
	int16	CriticalHitChance[HIGHEST_SKILL+2];	//i
	int16	CriticalSpellChance;				//i
	int16	SpellCritDmgIncrease;				//i
	int16	SpellCritDmgIncNoStack;				// increase
	int16	DotCritDmgIncrease;					//i
	int16	CriticalHealChance;					//i
	int16	CriticalHealOverTime;				//i
	int16	CriticalDoTChance;					//i
	int16	CrippBlowChance;					//
	int16	AvoidMeleeChance;					//AvoidMeleeChance/10 == % chance i = Avoidance
	int16	RiposteChance;						//i
	int16	DodgeChance;						//i
	int16	ParryChance;						//i
	int16	DualWieldChance;					//i
	int16	DoubleAttackChance;					//i
	int16	TripleAttackChance;					//i
	int16   DoubleRangedAttack;				//i
	int16	ResistSpellChance;					//i
	int16	ResistFearChance;					//i
	bool	Fearless;							//i
	bool	IsFeared;							//i
	int16	StunResist;							//i
	int16	MeleeSkillCheck;					//i
	uint8	MeleeSkillCheckSkill;
	int16	HitChance;							//HitChance/15 == % increase i = Accuracy (Item: Accuracy)
	int16	HitChanceEffect[HIGHEST_SKILL+2];	//Spell effect Chance to Hit, straight percent increase
	int16	DamageModifier[HIGHEST_SKILL+2];	//i
	int16	MinDamageModifier[HIGHEST_SKILL+2]; //i
	int16	ProcChance;							// ProcChance/10 == % increase i = CombatEffects
	int16	ProcChanceSPA;						// ProcChance from spell effects
	int16	ExtraAttackChance;
	int16	DoTShielding;
	int16	DivineSaveChance[2];				// Second Chance (base1 = chance, base2 = spell on trigger)
	uint16	DeathSave[4];						// Death Pact [0](value = 1 partial 2 = full) [1]=slot [2]=LvLimit [3]=HealAmt
	int16	FlurryChance;
	int16	Accuracy[HIGHEST_SKILL+2];			//Accuracy/15 == % increase	[Spell Effect: Accuracy)
	int16	HundredHands;						//extra haste, stacks with all other haste	i
	int8	MeleeLifetap;						//i
	int16	HealRate;							// Spell effect that influences effectiveness of heals
	int32	MaxHPChange;						// Spell Effect
	int16	SkillDmgTaken[HIGHEST_SKILL+2];		// All Skills + -1
	int32	HealAmt;							// Item Effect
	int32	SpellDmg;							// Item Effect
	int32	Clairvoyance;						// Item Effect
	int16	DSMitigation;						// Item Effect
	int16	DSMitigationOffHand;				// Lowers damage shield from off hand attacks.
	uint32	SpellTriggers[MAX_SPELL_TRIGGER];	// Innate/Spell/Item Spells that trigger when you cast
	uint32	SpellOnKill[MAX_SPELL_TRIGGER*3];	// Chance to proc after killing a mob
	uint32	SpellOnDeath[MAX_SPELL_TRIGGER*2];	// Chance to have effect cast when you die
	int16	CritDmgMob[HIGHEST_SKILL+2];		// All Skills + -1
	int16	SkillReuseTime[HIGHEST_SKILL+1];	// Reduces skill timers
	int16	SkillDamageAmount[HIGHEST_SKILL+2];	// All Skills + -1
	int16	TwoHandBluntBlock;					// chance to block when wielding two hand blunt weapon
	uint16	ItemManaRegenCap;					// Increases the amount of mana you have can over the cap(aa effect)
	int16	GravityEffect;						// Indictor of spell effect
	bool	AntiGate;							// spell effect that prevents gating
	bool	MagicWeapon;						// spell effect that makes weapon magical
	int16	IncreaseBlockChance;				// overall block chance modifier
	uint16	PersistantCasting;					// chance to continue casting through a stun
	int	XPRateMod;							//i
	int		HPPercCap[2];						//Spell effect that limits you to being healed/regening beyond a % of your max
	int		ManaPercCap[2];						// ^^ 0 = % Cap 1 = Flat Amount Cap
	int		EndPercCap[2];						// ^^
	bool	BlockNextSpell;						// Indicates whether the client can block a spell or not
	//uint16	BlockSpellEffect[EFFECT_COUNT];		// Prevents spells with certain effects from landing on you *no longer used
	bool	ImmuneToFlee;						// Bypass the fleeing flag
	uint16	VoiceGraft;							// Stores the ID of the mob with which to talk through
	int16	SpellProcChance;					// chance to proc from sympathetic spell effects
	int16	CharmBreakChance;					// chance to break charm
	int16	SongRange;							// increases range of beneficial bard songs
	uint16	HPToManaConvert;					// Uses HP to cast spells at specific conversion
	uint16	FocusEffects[HIGHEST_FOCUS+1];		// Stores the focus effectid for each focustype you have.
	bool	NegateEffects;						// Check if you contain a buff with negate effect. (only spellbonuses)
	int16	SkillDamageAmount2[HIGHEST_SKILL+2];	// Adds skill specific damage
	uint16	NegateAttacks[2];					// 0 = bool HasEffect 1 = Buff Slot
	uint16	MitigateMeleeRune[2];				// 0 = Mitigation value 1 = Buff Slot
	uint16	MeleeThresholdGuard[3];				// 0 = Mitigation value 1 = Buff Slot 2 = Min damage to trigger.
	uint16	SpellThresholdGuard[3];				// 0 = Mitigation value 1 = Buff Slot 2 = Min damage to trigger.
	uint16	MitigateSpellRune[2];				// 0 = Mitigation value 1 = Buff Slot
	uint16	MitigateDotRune[2];					// 0 = Mitigation value 1 = Buff Slot
	uint32	TriggerMeleeThreshold[3];			// 0 = Spell Effect ID 1 = Buff slot 2 = Damage Amount to Trigger
	uint32	TriggerSpellThreshold[3];			// 0 = Spell Effect ID 1 = Buff slot 2 = Damage Amount to Trigger
	uint16	ManaAbsorbPercentDamage[2];			// 0 = Mitigation value 1 = Buff Slot
	int16	ShieldBlock;						// Chance to Shield Block
	int16	BlockBehind;						// Chance to Block Behind (with our without shield)
	bool	CriticalRegenDecay;					// increase critical regen chance, decays based on spell level cast
	bool	CriticalHealDecay;					// increase critical heal chance, decays based on spell level cast
	bool	CriticalDotDecay;					// increase critical dot chance, decays based on spell level cast
	bool	DivineAura;							// invulnerability
	bool	DistanceRemoval;					// Check if Cancle if Moved effect is present
	int16	ImprovedTaunt[3];					// 0 = Max Level 1 = Aggro modifier 2 = buffid
	int8	Root[2];							// The lowest buff slot a root can be found. [0] = Bool if has root [1] = buff slot
	int16	FrenziedDevastation;				// base1= AArank(used) base2= chance increase spell criticals + all DD spells 2x mana.
	uint16	AbsorbMagicAtt[2];					// 0 = magic rune value 1 = buff slot
	uint16	MeleeRune[2];						// 0 = rune value 1 = buff slot
	bool	NegateIfCombat;						// Bool Drop buff if cast or melee
	int8	Screech;							// -1 = Will be blocked if another Screech is +(1)

	// AAs
	int8	Packrat;							//weight reduction for items, 1 point = 10%
	uint8	BuffSlotIncrease;					// Increases number of available buff slots
	uint16	DelayDeath;							// how far below 0 hp you can go
	int8	BaseMovementSpeed;					// Adjust base run speed, does not stack with other movement bonuses.
	uint8	IncreaseRunSpeedCap;				// Increase max run speed above cap.
	int16	DoubleSpecialAttack;				// Chance to to perform a double special attack (ie flying kick 2x)
	int16	SpecialAttackKBProc[2];				// Chance to to do a knockback from special attacks. (0 = chance 1 = Skill)
	uint8	FrontalStunResist;					// Chance to resist a frontal stun
	int16	BindWound;							// Increase amount of HP by percent.
	int16	MaxBindWound;						// Increase max amount of HP you can bind wound.
	int16	ChannelChanceSpells;				// Modify chance to channel a spell.
	int16	ChannelChanceItems;					// Modify chance to channel a items.
	uint8	SeeInvis;							// See Invs.
	uint8	TripleBackstab;						// Chance to triple backstab
	bool	FrontalBackstabMinDmg;				// Allow frontal backstabs for min damage
	uint8	FrontalBackstabChance;				// Chance to backstab from the front for full damage
	uint8	ConsumeProjectile;					// Chance to not consume arrow.
	uint8	ForageAdditionalItems;				// Chance to forage another item.
	uint8	SalvageChance;						// Chance to salvage a tradeskill components on fail.
	uint16	ArcheryDamageModifier;				// Increase Archery Damage by percent
	bool	SecondaryDmgInc;					// Allow off hand weapon to recieve damage bonus.
	uint16	GiveDoubleAttack;					// Allow classes to double attack with a specified chance.
	int16	SlayUndead[2];						// Allow classes to do extra damage verse undead.(base1 = rate, base2 = damage mod)
	int16	PetCriticalHit;						// Allow pets to critical hit with % value.
	int16	PetAvoidance;						// Pet avoidance chance.
	int16	CombatStability;					// Melee damage mitigation.
	int16	GiveDoubleRiposte[3];				// 0=Regular Chance, 1=Skill Attack Chance, 2=Skill
	uint16	RaiseSkillCap[2];					// Raise a specific skill cap (1 = value, 2=skill)
	int16	Ambidexterity;						// Increase chance to duel wield by adding bonus 'skill'.
	int16	PetMaxHP;							// Increase the max hp of your pet.
	int16	PetFlurry;							// Chance for pet to flurry.
	uint8	MasteryofPast;						// Can not fizzle spells below this level specified in value.
	bool	GivePetGroupTarget;					// All pets to recieve group buffs. (Pet Affinity)
	int16	RootBreakChance;					// Chance root will break;
	int16	UnfailingDivinity;					// Improves chance that DI will fire + increase partial heal.
	int16	ItemHPRegenCap;						// Increase item regen cap.
	int16	SEResist[MAX_RESISTABLE_EFFECTS*2];	// Resist chance by specific spell effects.
	int16	OffhandRiposteFail;					// chance for opponent to fail riposte with offhand attack.
	int16	ItemATKCap;							// Raise item attack cap
	int32	FinishingBlow[2];					// Chance to do a finishing blow for specified damage amount.
	uint16	FinishingBlowLvl[2];				// Sets max level an NPC can be affected by FB. (base1 = lv, base2= ???)
	int16	ShieldEquipHateMod;					// Hate mod when shield equiped.
	int16	ShieldEquipDmgMod[2];				// Damage mod when shield equiped. 0 = damage modifier 1 = Unknown
	bool	TriggerOnValueAmount;				// Triggers off various different conditions, bool to check if client has effect.
	int8	StunBashChance;						// chance to stun with bash.	
	int8	IncreaseChanceMemwipe;				// increases chance to memory wipe
	int8	CriticalMend;						// chance critical monk mend
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

//eventually turn this into a typedef and
//make DoAnim take it instead of int, to enforce its use.
enum {	//type arguments to DoAnim
	animKick				= 1,
	animPiercing			= 2,	//might be piercing?
	anim2HSlashing			= 3,
	anim2HWeapon			= 4,
	anim1HWeapon			= 5,
	animDualWield			= 6,
	animTailRake			= 7,	//slam & Dpunch too
	animHand2Hand			= 8,
	animShootBow			= 9,
	animRoundKick			= 11,
	animSwarmAttack			= 20,	//dunno about this one..
	animFlyingKick			= 45,
	animTigerClaw			= 46,
	animEagleStrike			= 47,

};


typedef enum {
	petFamiliar,		//only listens to /pet get lost
	petAnimation,		//does not listen to any commands
	petOther,
	petCharmed,
	petNPCFollow,
	petHatelist			//remain active as long something is on the hatelist. Don't listen to any commands
} PetType;

typedef enum {
	SingleTarget,	// causes effect to spell_target
	AETarget,			// causes effect in aerange of target + target
	AECaster,			// causes effect in aerange of 'this'
	GroupSpell,		// causes effect to caster + target's group
	CAHateList,		// causes effect to all people on caster's hate list within some range
	DirectionalAE,
	CastActUnknown
} CastAction_type;


struct MercType {
	uint32	Type;
	uint32	ClientVersion;
};

struct MercData {
	uint32	MercTemplateID;
	uint32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	uint32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	uint32	CostFormula;			// To determine cost to client
	uint32	ClientVersion;				// Only send valid mercs per expansion
	uint32	NPCID;
};

class ItemInst;
class Mob;
// All data associated with a single trade
class Trade
{
public:
	Trade(Mob* in_owner);
	virtual ~Trade();

	void Reset();
	void SetTradeCash(uint32 in_pp, uint32 in_gp, uint32 in_sp, uint32 in_cp);

	// Initiate a trade with another mob
	// Also puts other mob into trader mode with this mob
	void Start(uint32 mob_id, bool initiate_with=true);

	// Mob the owner is trading with
	Mob* With();

	// Add item from cursor slot to trade bucket (automatically does bag data too)
	void AddEntity(uint16 from_slot_id, uint16 trade_slot_id);

	// Audit trade
	void LogTrade();

	// Debug only method
	#if (EQDEBUG >= 9)
		void DumpTrade();
	#endif

public:
	// Object state
	TradeState state;
	int32 pp;
	int32 gp;
	int32 sp;
	int32 cp;

private:
	// Send item data for trade item to other person involved in trade
	void SendItemData(const ItemInst* inst, int16 dest_slot_id);

	uint32 with_id;
	Mob* owner;
};

struct ExtraAttackOptions {
	ExtraAttackOptions()
		: damage_percent(1.0f), damage_flat(0),
		armor_pen_percent(0.0f), armor_pen_flat(0),
		crit_percent(1.0f), crit_flat(0.0f),
		hate_percent(1.0f), hate_flat(0)
	{ }

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

