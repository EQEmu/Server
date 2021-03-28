#ifndef __EQEMU_ZONE_COMMON_H
#define __EQEMU_ZONE_COMMON_H

#include "../common/types.h"
#include "../common/spdat.h"

#define	HIGHEST_RESIST 9 //Max resist type value
#define MAX_SPELL_PROJECTILE 10 //Max amount of spell projectiles that can be active by a single mob.

/* macros for IsAttackAllowed, IsBeneficialAllowed */
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
#define CON_WHITE		10
#define CON_WHITE_TITANIUM		20
#define CON_YELLOW		15
#define CON_RED			13
#define CON_GRAY		6

#define DMG_BLOCKED		-1
#define DMG_PARRIED		-2
#define DMG_RIPOSTED		-3
#define DMG_DODGED		-4
#define DMG_INVULNERABLE	-5
#define DMG_RUNE		-6

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

// Defines based on the RoF2 Client
#define PET_HEALTHREPORT	0	// 0x00 - /pet health or Pet Window
#define PET_LEADER			1	// 0x01 - /pet leader or Pet Window
#define PET_ATTACK			2	// 0x02 - /pet attack or Pet Window
#define PET_QATTACK			3	// 0x03 - /pet qattack or Pet Window
#define PET_FOLLOWME		4	// 0x04 - /pet follow or Pet Window
#define PET_GUARDHERE		5	// 0x05 - /pet guard or Pet Window
#define PET_SIT				6	// 0x06 - /pet sit or Pet Window
#define PET_SITDOWN			7	// 0x07 - /pet sit on
#define PET_STANDUP			8	// 0x08 - /pet sit off
#define PET_STOP			9	// 0x09 - /pet stop or Pet Window - Not implemented
#define PET_STOP_ON			10	// 0x0a - /pet stop on - Not implemented
#define PET_STOP_OFF		11	// 0x0b - /pet stop off - Not implemented
#define PET_TAUNT			12	// 0x0c - /pet taunt or Pet Window
#define PET_TAUNT_ON		13	// 0x0d - /pet taunt on
#define PET_TAUNT_OFF		14	// 0x0e - /pet taunt off
#define PET_HOLD			15	// 0x0f - /pet hold or Pet Window, won't add to hate list unless attacking
#define PET_HOLD_ON			16	// 0x10 - /pet hold on
#define PET_HOLD_OFF		17	// 0x11 - /pet hold off
#define PET_GHOLD			18	// 0x12 - /pet ghold, will never add to hate list unless told to
#define PET_GHOLD_ON		19	// 0x13 - /pet ghold on
#define PET_GHOLD_OFF		20	// 0x14 - /pet ghold off
#define PET_SPELLHOLD		21	// 0x15 - /pet no cast or /pet spellhold or Pet Window
#define PET_SPELLHOLD_ON	22	// 0x16 - /pet spellhold on
#define PET_SPELLHOLD_OFF	23	// 0x17 - /pet spellhold off
#define PET_FOCUS			24	// 0x18 - /pet focus or Pet Window
#define PET_FOCUS_ON		25	// 0x19 - /pet focus on
#define PET_FOCUS_OFF		26	// 0x1a - /pet focus off
#define PET_FEIGN			27	// 0x1b - /pet feign
#define PET_BACKOFF			28	// 0x1c - /pet back off
#define PET_GETLOST			29	// 0x1d - /pet get lost
#define PET_GUARDME			30	// 0x1e - Same as /pet follow, but different message in older clients - define not from client /pet target in modern clients but doesn't send packet
#define PET_REGROUP			31	// 0x1f - /pet regroup, acts like classic hold. Stops attack and moves back to guard/you but doesn't clear hate list
#define PET_REGROUP_ON		32	// 0x20 - /pet regroup on, turns on regroup
#define PET_REGROUP_OFF		33	// 0x21 - /pet regroup off, turns off regroup
#define PET_MAXCOMMANDS		PET_REGROUP_OFF + 1

// can change the state of these buttons with a packet
#define PET_BUTTON_SIT			0
#define PET_BUTTON_STOP			1
#define PET_BUTTON_REGROUP		2
#define PET_BUTTON_FOLLOW		3
#define PET_BUTTON_GUARD		4
#define PET_BUTTON_TAUNT		5
#define PET_BUTTON_HOLD			6
#define PET_BUTTON_GHOLD		7
#define PET_BUTTON_FOCUS		8
#define PET_BUTTON_SPELLHOLD	9

#define AURA_HARDCAP		2

typedef enum {	//focus types
	focusSpellHaste = 1,
	focusSpellDuration,
	focusRange,
	focusReagentCost,
	focusManaCost,
	focusImprovedHeal,
	focusImprovedDamage,
	focusImprovedDamage2,
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
	focusFcDamageAmt2,
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
	ALLOW_BENEFICIAL = 38,
	DISABLE_MELEE = 39,
	NPC_CHASE_DISTANCE = 40,
	ALLOW_TO_TANK = 41,
	IGNORE_ROOT_AGGRO_RULES = 42,
	CASTING_RESIST_DIFF = 43,
	COUNTER_AVOID_DAMAGE = 44,
	PROX_AGGRO = 45,
	IMMUNE_RANGED_ATTACKS = 46,
	IMMUNE_DAMAGE_CLIENT = 47,
	IMMUNE_DAMAGE_NPC = 48,
	IMMUNE_AGGRO_CLIENT = 49,
	IMMUNE_AGGRO_NPC = 50,
	MAX_SPECIAL_ATTACK = 51
};

typedef enum {	//fear states
	fearStateNotFeared = 0,
	fearStateRunning,		//I am running, hoping to find a grid at my WP
	fearStateRunningForever,	//can run straight until spell ends
	fearStateGrid,			//I am allready on a fear grid
	fearStateStuck			//I cannot move somehow...
} FearState;

// This is actually FlyMode, from MQ2
enum GravityBehavior {
	Ground,
	Flying,
	Levitating,
	Water,
	Floating, // boat
	LevitateWhileRunning
};

struct TradeEntity;
class Trade;
enum TradeState {
	TradeNone,
	Trading,
	TradeAccepted,
	TradeCompleting
};

enum class NumHit {		  // Numhits type
	IncomingHitAttempts = 1,  // Attempted incoming melee attacks (hit or miss) on YOU.
	OutgoingHitAttempts = 2,  // Attempted outgoing melee attacks (hit or miss) on YOUR TARGET.
	IncomingSpells = 3,       // Incoming detrimental spells
	OutgoingSpells = 4,       // Outgoing detrimental spells
	OutgoingHitSuccess = 5,   // Successful outgoing melee attack HIT on YOUR TARGET.
	IncomingHitSuccess = 6,   // Successful incoming melee attack HIT on YOU.
	MatchingSpells = 7,       // Any casted spell matching/triggering a focus effect.
	IncomingDamage = 8,       // Successful incoming spell or melee dmg attack on YOU
	ReflectSpell = 9,	 // Incoming Reflected spells.
	DefensiveSpellProcs = 10, // Defensive buff procs
	OffensiveSpellProcs = 11  // Offensive buff procs
};

enum class PlayerState : uint32 {
	None = 0,
	Open = 1,
	WeaponSheathed = 2,
	Aggressive = 4,
	ForcedAggressive = 8,
	InstrumentEquipped = 16,
	Stunned = 32,
	PrimaryWeaponEquipped = 64,
	SecondaryWeaponEquipped = 128
};

enum class LootResponse : uint8 {
	SomeoneElse = 0,
	Normal = 1,
	NotAtThisTime = 2,
	Normal2 = 3, // acts exactly the same as Normal, maybe group vs ungroup? No idea
	Hostiles = 4,
	TooFar = 5,
	LootAll = 6 // SoD+
};

enum class LootRequestType : uint8 {
	Forbidden = 0,
	GMPeek,
	GMAllowed,
	Self,
	AllowedPVE,
	AllowedPVPAll,
	AllowedPVPSingle, // can make this 'AllowedPVPVariable' and allow values between 1 and EQ::invtype::POSSESSIONS_SIZE
	AllowedPVPDefined,
};

namespace Journal {
	enum class SpeakMode : uint8 {
		Raw = 0,	// this just uses the raw message
		Say = 1,	// prints with "%1 says,%2 '%3'" if in another language else "%1 says '%2'"
		Shout = 2,	// prints with "%1 shouts,%2 '%3'" if in another language else "%1 shouts '%2'"
		EmoteAlt = 3,	// prints "%2", this should just be the same as raw ...
		Emote = 4,	// prints "%1 %2" if message doesn't start with "\" or "@", else "%1%2"
		Group = 5	// prints "%1 tells the group,%2 '%3'"
	};

	enum class Mode : uint8 {
		None = 0,
		Log1 = 1, // 1 and 2 log to journal
		Log2 = 2, // our current code uses 2
	};

	struct Options {
		SpeakMode speak_mode;
		Mode journal_mode;
		int8 language;
		uint32 message_type;
		uint32 target_spawn_id; // who the message is talking to (limits journaling)
	};
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
	uint32	instrument_mod;
	bool	persistant_buff;
	bool	client; //True if the caster is a client
	bool	UpdateClient;
};

struct StatBonuses {
	int32	AC;
	int32	HP;
	int32	HPRegen;
	int32	MaxHP;
	int32	ManaRegen;
	int32	EnduranceRegen;
	int32	Mana;
	int32	Endurance;
	int32	ATK;
	//would it be worth it to create a Stat_Struct?
	int32	STR;
	int32	STRCapMod;
	int32	HeroicSTR;
	int32	STA;
	int32	STACapMod;
	int32	HeroicSTA;
	int32	DEX;
	int32	DEXCapMod;
	int32	HeroicDEX;
	int32	AGI;
	int32	AGICapMod;
	int32	HeroicAGI;
	int32	INT;
	int32	INTCapMod;
	int32	HeroicINT;
	int32	WIS;
	int32	WISCapMod;
	int32	HeroicWIS;
	int32	CHA;
	int32	CHACapMod;
	int32	HeroicCHA;
	int32	MR;
	int32	MRCapMod;
	int32	HeroicMR;
	int32	FR;
	int32	FRCapMod;
	int32	HeroicFR;
	int32	CR;
	int32	CRCapMod;
	int32	HeroicCR;
	int32	PR;
	int32	PRCapMod;
	int32	HeroicPR;
	int32	DR;
	int32	DRCapMod;
	int32	HeroicDR;
	int32	Corrup;
	int32	CorrupCapMod;
	int32	HeroicCorrup;
	uint16	DamageShieldSpellID;
	int		DamageShield;						// this is damage done to mobs that attack this
	DmgShieldType	DamageShieldType;
	int		SpellDamageShield;
	int		SpellShield;
	int		ReverseDamageShield;				// this is damage done to the mob when it attacks
	uint16	ReverseDamageShieldSpellID;
	DmgShieldType	ReverseDamageShieldType;
	int		movementspeed;
	int32	haste;
	int32	hastetype2;
	int32	hastetype3;
	int32	inhibitmelee;
	float	AggroRange;							// when calculate just replace original value with this
	float	AssistRange;
	int32	skillmod[EQ::skills::HIGHEST_SKILL + 1];
	int32	skillmodmax[EQ::skills::HIGHEST_SKILL + 1];
	int		effective_casting_level;
	int		adjusted_casting_skill;				// SPA 112 for fizzles
	int		reflect_chance;						// chance to reflect incoming spell
	uint32	singingMod;
	uint32	Amplification;						// stacks with singingMod
	uint32	brassMod;
	uint32	percussionMod;
	uint32	windMod;
	uint32	stringedMod;
	uint32	songModCap;
	int8	hatemod;
	int32	EnduranceReduction;

	int32	StrikeThrough;						// PoP: Strike Through %
	int32	MeleeMitigation;					//i = Shielding
	int32	MeleeMitigationEffect;				//i = Spell Effect Melee Mitigation
	int32	CriticalHitChance[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	CriticalSpellChance;				//i
	int32	SpellCritDmgIncrease;				//i
	int32	SpellCritDmgIncNoStack;				// increase
	int32	DotCritDmgIncrease;					//i
	int32	CriticalHealChance;					//i
	int32	CriticalHealOverTime;				//i
	int32	CriticalDoTChance;					//i
	int32	CrippBlowChance;					//
	int32	AvoidMeleeChance;					//AvoidMeleeChance/10 == % chance i = Avoidance (item mod)
	int32	AvoidMeleeChanceEffect;				//AvoidMeleeChance Spell Effect
	int32	RiposteChance;						//i
	int32	DodgeChance;						//i
	int32	ParryChance;						//i
	int32	DualWieldChance;					//i
	int32	DoubleAttackChance;					//i
	int32	TripleAttackChance;					//i
	int32   DoubleRangedAttack;				//i
	int32	ResistSpellChance;					//i
	int32	ResistFearChance;					//i
	bool	Fearless;							//i
	bool	IsFeared;							//i
	bool	IsBlind;							//i
	int32	StunResist;							//i
	int32	MeleeSkillCheck;					//i
	uint8	MeleeSkillCheckSkill;
	int32	HitChance;							//HitChance/15 == % increase i = Accuracy (Item: Accuracy)
	int32	HitChanceEffect[EQ::skills::HIGHEST_SKILL + 2];	//Spell effect Chance to Hit, straight percent increase
	int32	DamageModifier[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	DamageModifier2[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	MinDamageModifier[EQ::skills::HIGHEST_SKILL + 2]; //i
	int32	ProcChance;							// ProcChance/10 == % increase i = CombatEffects
	int32	ProcChanceSPA;						// ProcChance from spell effects
	int32	ExtraAttackChance;
	int32	DoTShielding;
	int32	DivineSaveChance[2];				// Second Chance (base1 = chance, base2 = spell on trigger)
	uint32	DeathSave[4];						// Death Pact [0](value = 1 partial 2 = full) [1]=slot [2]=LvLimit [3]=HealAmt
	int32	FlurryChance;
	int32	Accuracy[EQ::skills::HIGHEST_SKILL + 2];			//Accuracy/15 == % increase	[Spell Effect: Accuracy)
	int32	HundredHands;						//extra haste, stacks with all other haste	i
	int32	MeleeLifetap;						//i
	int32	Vampirism;							//i
	int32	HealRate;							// Spell effect that influences effectiveness of heals
	int32	MaxHPChange;						// Spell Effect
	int16	SkillDmgTaken[EQ::skills::HIGHEST_SKILL + 2];		// All Skills + -1
	int32	HealAmt;							// Item Effect
	int32	SpellDmg;							// Item Effect
	int32	Clairvoyance;						// Item Effect
	int32	DSMitigation;						// Item Effect
	int32	DSMitigationOffHand;				// Lowers damage shield from off hand attacks.
	uint32	SpellTriggers[MAX_SPELL_TRIGGER];	// Innate/Spell/Item Spells that trigger when you cast
	uint32	SpellOnKill[MAX_SPELL_TRIGGER*3];	// Chance to proc after killing a mob
	uint32	SpellOnDeath[MAX_SPELL_TRIGGER*2];	// Chance to have effect cast when you die
	int32	CritDmgMod[EQ::skills::HIGHEST_SKILL + 2];		// All Skills + -1
	int32	SkillReuseTime[EQ::skills::HIGHEST_SKILL + 1];	// Reduces skill timers
	int32	SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 2];	// All Skills + -1
	int32	TwoHandBluntBlock;					// chance to block when wielding two hand blunt weapon
	uint32	ItemManaRegenCap;					// Increases the amount of mana you have can over the cap(aa effect)
	int32	GravityEffect;						// Indictor of spell effect
	bool	AntiGate;							// spell effect that prevents gating
	bool	MagicWeapon;						// spell effect that makes weapon magical
	int32	IncreaseBlockChance;				// overall block chance modifier
	uint32	PersistantCasting;					// chance to continue casting through a stun
	int	XPRateMod;							//i
	int		HPPercCap[2];						//Spell effect that limits you to being healed/regening beyond a % of your max
	int		ManaPercCap[2];						// ^^ 0 = % Cap 1 = Flat Amount Cap
	int		EndPercCap[2];						// ^^
	bool	BlockNextSpell;						// Indicates whether the client can block a spell or not
	//uint16	BlockSpellEffect[EFFECT_COUNT];		// Prevents spells with certain effects from landing on you *no longer used
	bool	ImmuneToFlee;						// Bypass the fleeing flag
	uint32	VoiceGraft;							// Stores the ID of the mob with which to talk through
	int32	SpellProcChance;					// chance to proc from sympathetic spell effects
	int32	CharmBreakChance;					// chance to break charm
	int32	SongRange;							// increases range of beneficial bard songs
	uint32	HPToManaConvert;					// Uses HP to cast spells at specific conversion
	uint8	FocusEffects[HIGHEST_FOCUS+1];		// Stores the focus effectid for each focustype you have.
	int16	FocusEffectsWorn[HIGHEST_FOCUS+1];	// Optional to allow focus effects to be applied additively from worn slot
	bool	NegateEffects;						// Check if you contain a buff with negate effect. (only spellbonuses)
	int32	SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 2];	// Adds skill specific damage
	uint32	NegateAttacks[3];					// 0 = bool HasEffect 1 = Buff Slot 2 = Max damage absorbed per hit
	uint32	MitigateMeleeRune[4];				// 0 = Mitigation value 1 = Buff Slot 2 = Max mitigation per hit 3 = Rune Amt
	uint32	MeleeThresholdGuard[3];				// 0 = Mitigation value 1 = Buff Slot 2 = Min damage to trigger.
	uint32	SpellThresholdGuard[3];				// 0 = Mitigation value 1 = Buff Slot 2 = Min damage to trigger.
	uint32	MitigateSpellRune[4];				// 0 = Mitigation value 1 = Buff Slot 2 = Max mitigation per spell 3 = Rune Amt
	uint32	MitigateDotRune[4];					// 0 = Mitigation value 1 = Buff Slot 2 = Max mitigation per tick 3 = Rune Amt
	bool	TriggerMeleeThreshold;				// Has Melee Threshhold
	bool	TriggerSpellThreshold;				// Has Spell Threshhold
	uint32	ManaAbsorbPercentDamage[2];			// 0 = Mitigation value 1 = Buff Slot
	int32	ShieldBlock;						// Chance to Shield Block
	int32	BlockBehind;						// Chance to Block Behind (with our without shield)
	bool	CriticalRegenDecay;					// increase critical regen chance, decays based on spell level cast
	bool	CriticalHealDecay;					// increase critical heal chance, decays based on spell level cast
	bool	CriticalDotDecay;					// increase critical dot chance, decays based on spell level cast
	bool	DivineAura;							// invulnerability
	bool	DistanceRemoval;					// Check if Cancle if Moved effect is present
	int32	ImprovedTaunt[3];					// 0 = Max Level 1 = Aggro modifier 2 = buffid
	int8	Root[2];							// The lowest buff slot a root can be found. [0] = Bool if has root [1] = buff slot
	int32	FrenziedDevastation;				// base1= AArank(used) base2= chance increase spell criticals + all DD spells 2x mana.
	uint32	AbsorbMagicAtt[2];					// 0 = magic rune value 1 = buff slot
	uint32	MeleeRune[2];						// 0 = rune value 1 = buff slot
	bool	NegateIfCombat;						// Bool Drop buff if cast or melee
	int8	Screech;							// -1 = Will be blocked if another Screech is +(1)
	int32	AlterNPCLevel;						// amount of lvls +/-
	int32	AStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	int32	BStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	int32	CStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	int32	DStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	bool	BerserkSPA;							// berserk effect
	int32	Metabolism;							// Food/drink consumption rates.
	bool	Sanctuary;							// Sanctuary effect, lowers place on hate list until cast on others.
	int32   FactionModPct;						// Modifies amount of faction gained.
	bool	LimitToSkill[EQ::skills::HIGHEST_SKILL + 2];		// Determines if we need to search for a skill proc.
	uint32  SkillProc[MAX_SKILL_PROCS];			// Max number of spells containing skill_procs.
	uint32  SkillProcSuccess[MAX_SKILL_PROCS];	// Max number of spells containing skill_procs_success.
	uint32  PC_Pet_Rampage[2];					// 0= % chance to rampage, 1=damage modifier
	uint32  PC_Pet_Flurry;						// Percent chance flurry from double attack

	// AAs
	int8	Packrat;							//weight reduction for items, 1 point = 10%
	uint8	BuffSlotIncrease;					// Increases number of available buff slots
	uint32	DelayDeath;							// how far below 0 hp you can go
	int8	BaseMovementSpeed;					// Adjust base run speed, does not stack with other movement bonuses.
	uint8	IncreaseRunSpeedCap;				// Increase max run speed above cap.
	int32	DoubleSpecialAttack;				// Chance to to perform a double special attack (ie flying kick 2x)
	int32	SkillAttackProc[3];					// [0] chance to proc [2] spell on [1] skill usage
	uint8	FrontalStunResist;					// Chance to resist a frontal stun
	int32	BindWound;							// Increase amount of HP by percent.
	int32	MaxBindWound;						// Increase max amount of HP you can bind wound.
	int32	ChannelChanceSpells;				// Modify chance to channel a spell.
	int32	ChannelChanceItems;					// Modify chance to channel a items.
	uint8	SeeInvis;							// See Invs.
	uint8	TripleBackstab;						// Chance to triple backstab
	bool	FrontalBackstabMinDmg;				// Allow frontal backstabs for min damage
	uint8	FrontalBackstabChance;				// Chance to backstab from the front for full damage
	uint8	ConsumeProjectile;					// Chance to not consume arrow.
	uint8	ForageAdditionalItems;				// Chance to forage another item.
	uint8	SalvageChance;						// Chance to salvage a tradeskill components on fail.
	uint32	ArcheryDamageModifier;				// Increase Archery Damage by percent
	bool	SecondaryDmgInc;					// Allow off hand weapon to recieve damage bonus.
	uint32	GiveDoubleAttack;					// Allow classes to double attack with a specified chance.
	int32	SlayUndead[2];						// Allow classes to do extra damage verse undead.(base1 = rate, base2 = damage mod)
	int32	PetCriticalHit;						// Allow pets to critical hit with % value.
	int32	PetAvoidance;						// Pet avoidance chance.
	int32	CombatStability;					// Melee damage mitigation.
	int32	DoubleRiposte;						// Chance to double riposte
	int32	GiveDoubleRiposte[3];				// 0=Regular Chance, 1=Skill Attack Chance, 2=Skill
	uint32	RaiseSkillCap[EQ::skills::HIGHEST_SKILL + 1];		// Raise a specific skill cap (base1= value, base2=skill)
	int32	Ambidexterity;						// Increase chance to duel wield by adding bonus 'skill'.
	int32	PetMaxHP;							// Increase the max hp of your pet.
	int32	PetFlurry;							// Chance for pet to flurry.
	uint8	MasteryofPast;						// Can not fizzle spells below this level specified in value.
	bool	GivePetGroupTarget;					// All pets to recieve group buffs. (Pet Affinity)
	int32	RootBreakChance;					// Chance root will break;
	int32	UnfailingDivinity;					// Improves chance that DI will fire + increase partial heal.
	int32	ItemHPRegenCap;						// Increase item regen cap.
	int32	SEResist[MAX_RESISTABLE_EFFECTS*2];	// Resist chance by specific spell effects.
	int32	OffhandRiposteFail;					// chance for opponent to fail riposte with offhand attack.
	int32	ItemATKCap;							// Raise item attack cap
	int32	FinishingBlow[2];					// Chance to do a finishing blow for specified damage amount.
	uint32	FinishingBlowLvl[2];				// Sets max level an NPC can be affected by FB. (base1 = lv, base2= ???)
	int32	ShieldEquipDmgMod;					// Increases weapon's base damage by base1 % when shield is equipped (indirectly increasing hate)
	bool	TriggerOnValueAmount;				// Triggers off various different conditions, bool to check if client has effect.
	int8	StunBashChance;						// chance to stun with bash.
	int8	IncreaseChanceMemwipe;				// increases chance to memory wipe
	int8	CriticalMend;						// chance critical monk mend
	int32	ImprovedReclaimEnergy;				// Modifies amount of mana returned from reclaim energy
	uint32	HeadShot[2];						// Headshot AA (Massive dmg vs humaniod w/ archery) 0= ? 1= Dmg
	uint8	HSLevel[2];							// Max Level Headshot will be effective at. and chance mod
	uint32	Assassinate[2];						// Assassinate AA (Massive dmg vs humaniod w/ assassinate) 0= ? 1= Dmg
	uint8	AssassinateLevel[2];				// Max Level Assassinate will be effective at.
	int32	PetMeleeMitigation;					// Add AC to owner's pet.
	bool	IllusionPersistence;				// Causes illusions not to fade.
	uint16	extra_xtargets;						// extra xtarget entries
	bool	ShroudofStealth;					// rogue improved invisiblity
	uint16  ReduceFallDamage;					// reduce fall damage by percent
	int32	ReduceTradeskillFail[EQ::skills::HIGHEST_SKILL + 1]; // Reduces chance for trade skills to fail by percent.
	uint8	TradeSkillMastery;					// Allow number of tradeskills to exceed 200 skill.
	int16	NoBreakAESneak;						// Percent value
	int16	FeignedCastOnChance;				// Percent Value
	bool	PetCommands[PET_MAXCOMMANDS];		// SPA 267
	int	FeignedMinionChance;				// SPA 281 base1 = chance, just like normal FD
	int	GrantForage; // affects max skill of forage as well as granting non-forage classes forage
	int aura_slots;
	int trap_slots;
	bool hunger; // Song of Sustenance -- min caps to 3500
};

typedef struct
{
	uint16 spellID;
	uint16 chance;
	uint16 base_spellID;
	int level_override;
} tProc;

struct Shielders_Struct {
	uint32 shielder_id;
	uint16 shielder_bonus;
};

typedef struct
{
	uint16 increment;
	uint16 hit_increment;
	uint16 target_id;
	int32 wpn_dmg;
	float origin_x;
	float origin_y;
	float origin_z;
	float tlast_x;
	float tlast_y;
	uint32 ranged_id;
	uint32 ammo_id;
	int ammo_slot;
	uint8 skill;
	float speed_mod;
} tProjatk;

//eventually turn this into a typedef and
//make DoAnim take it instead of int, to enforce its use.
enum {	//type arguments to DoAnim
	animKick				= 1,
	anim1HPiercing			= 2,	//might be piercing?
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

enum {
	SKILLUP_UNKNOWN = 0,
	SKILLUP_SUCCESS = 1,
	SKILLUP_FAILURE = 2
};

enum {
	GridCircular,
	GridRandom10,
	GridRandom,
	GridPatrol,
	GridOneWayRepop,
	GridRand5LoS,
	GridOneWayDepop,
	GridCenterPoint,
	GridRandomCenterPoint,
	GridRandomPath
};

typedef enum {
	petFamiliar,		//only listens to /pet get lost
	petAnimation,		//does not listen to any commands
	petOther,
	petCharmed,
	petNPCFollow,
	petTargetLock,			//remain active as long something is on the hatelist. Don't listen to any commands
	petNone = 0xFF // not a pet
} PetType;

typedef enum {
	SingleTarget,	// causes effect to spell_target
	AETarget,			// causes effect in aerange of target + target
	AECaster,			// causes effect in aerange of 'this'
	GroupSpell,		// causes effect to caster + target's group
	CAHateList,		// causes effect to all people on caster's hate list within some range
	DirectionalAE,
	Beam,
	TargetRing,
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

namespace EQ
{
	class ItemInstance;
}

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
	void AddEntity(uint16 trade_slot_id, uint32 stack_size);

	// Audit trade
	void LogTrade();

	void DumpTrade();


public:
	// Object state
	TradeState state;
	int32 pp;
	int32 gp;
	int32 sp;
	int32 cp;

private:
	// Send item data for trade item to other person involved in trade
	void SendItemData(const EQ::ItemInstance* inst, int16 dest_slot_id);

	uint32 with_id;
	Mob* owner;
};

struct ExtraAttackOptions {
	ExtraAttackOptions()
		: damage_percent(1.0f), damage_flat(0),
		armor_pen_percent(0.0f), armor_pen_flat(0),
		crit_percent(1.0f), crit_flat(0.0f),
		hate_percent(1.0f), hate_flat(0), hit_chance(0),
		melee_damage_bonus_flat(0), skilldmgtaken_bonus_flat(0)
	{ }

	float damage_percent;
	int damage_flat;
	float armor_pen_percent;
	int armor_pen_flat;
	float crit_percent;
	float crit_flat;
	float hate_percent;
	int hate_flat;
	int hit_chance;
	int melee_damage_bonus_flat;
	int skilldmgtaken_bonus_flat;

};

struct DamageTable {
	int32 max_extra; // max extra damage
	int32 chance; // chance not to apply?
	int32 minusfactor; // difficulty of rolling
};

struct DamageHitInfo {
	//uint16 attacker; // id
	//uint16 defender; // id
	int base_damage;
	int min_damage;
	int damage_done;
	int offense;
	int tohit;
	int hand;
	EQ::skills::SkillType skill;
};

struct ExpeditionInvite
{
	uint32_t    expedition_id;
	std::string inviter_name;
	std::string swap_remove_name;
};

#endif

