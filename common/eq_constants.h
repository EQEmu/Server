/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_EQ_CONSTANTS_H
#define COMMON_EQ_CONSTANTS_H

#include "skills.h"
#include "types.h"


//SpawnAppearance types: (compared two clients for server-originating types: SoF & RoF2)
#define AT_Die 0			// this causes the client to keel over and zone to bind point (default action)
#define AT_WhoLevel 1		// the level that shows up on /who
#define AT_HPMax 2			// idk
#define AT_Invis 3			// 0 = visible, 1 = invisible
#define AT_PVP 4			// 0 = blue, 1 = pvp (red)
#define AT_Light 5			// light type emitted by player (lightstone, shiny shield)
#define AT_Anim 14			// 100=standing, 110=sitting, 111=ducking, 115=feigned, 105=looting
#define AT_Sneak 15			// 0 = normal, 1 = sneaking
#define AT_SpawnID 16		// server to client, sets player spawn id
#define AT_HP 17			// Client->Server, my HP has changed (like regen tic)
#define AT_Linkdead 18		// 0 = normal, 1 = linkdead
#define AT_Levitate 19		// 0=off, 1=flymode, 2=levitate max 5, see GravityBehavior enum
#define AT_GM 20			// 0 = normal, 1 = GM - all odd numbers seem to make it GM
#define AT_Anon 21			// 0 = normal, 1 = anon, 2 = roleplay
#define AT_GuildID 22
#define AT_GuildRank 23		// 0=member, 1=officer, 2=leader
#define AT_AFK 24			// 0 = normal, 1 = afk
#define AT_Pet 25			// Param is EntityID of owner, or 0 for when charm breaks
#define AT_Summoned 27		// Unsure
#define AT_Split 28			// 0 = normal, 1 = autosplit on (not showing in SoF+) (client-to-server only)
#define AT_Size 29			// spawn's size (present: SoF, absent: RoF2)
#define AT_SetType 30		// 0 = PC, 1 = NPC, 2 <= = corpse
#define AT_NPCName 31		// change PC's name's color to NPC color 0 = normal, 1 = npc name, Trader on RoF2?
#define AT_AARank 32		// AA Rank Title ID thingy, does is this the title in /who?
#define AT_CancelSneakHide 33	// Turns off Hide and Sneak
//#define AT_34 34			// unknown (present: SoF, absent: RoF2)
#define AT_AreaHPRegen 35	// guild hall regen pool sets to value * 0.001
#define AT_AreaManaRegen 36	// guild hall regen pool sets to value * 0.001
#define AT_AreaEndRegen 37	// guild hall regen pool sets to value * 0.001
#define AT_FreezeBuffs 38	// Freezes beneficial buff timers
#define AT_NpcTintIndex 39	// not 100% sure
#define AT_GroupConsent 40	// auto consent group
#define AT_RaidConsent 41	// auto consent raid
#define AT_GuildConsent 42	// auto consent guild
#define AT_ShowHelm 43		// 0 = hide graphic, 1 = show graphic
#define AT_DamageState 44	// The damage state of a destructible object (0 through 10) plays soundids most only have 2 or 4 states though
#define AT_EQPlayers 45		// /eqplayersupdate
#define AT_FindBits 46		// set FindBits, whatever those are!
#define AT_TextureType 48	// TextureType
#define AT_FacePick 49		// Turns off face pick window? maybe ...
#define AT_GuildShow 52		// this is what MQ2 call sit, not sure
#define AT_Offline 53		// Offline mode

//#define AT_Trader 300		// Bazaar Trader Mode (not present in SoF or RoF2)

// animations for AT_Anim
#define ANIM_FREEZE	    102
#define	ANIM_STAND		0x64
#define	ANIM_SIT		0x6e
#define	ANIM_CROUCH		0x6f
#define	ANIM_DEATH		0x73
#define ANIM_LOOT		0x69

typedef enum {
	eaStanding = 0,
	eaSitting,		//1
	eaCrouching,	//2
	eaDead,			//3
	eaLooting,		//4
	_eaMaxAppearance
} EmuAppearance;

namespace Chat {
	const uint16 White       = 0;
	const uint16 DimGray     = 1;
	const uint16 Default     = 1;
	const uint16 Green       = 2;
	const uint16 BrightBlue  = 3;
	const uint16 LightBlue   = 4;
	const uint16 Magenta     = 5;
	const uint16 Gray        = 6;
	const uint16 LightGray   = 7;
	const uint16 NPCQuestSay = 10;
	const uint16 DarkGray    = 12;
	const uint16 Red         = 13;
	const uint16 Lime        = 14;
	const uint16 Yellow      = 15;
	const uint16 Blue        = 16;
	const uint16 LightNavy   = 17;
	const uint16 Cyan        = 18;
	const uint16 Black       = 20;

	/**
	 * User colors
	 */
	const uint16 Say              = 256;
	const uint16 Tell             = 257;
	const uint16 Group            = 258;
	const uint16 Guild            = 259;
	const uint16 OOC              = 260;
	const uint16 Auction          = 261;
	const uint16 Shout            = 262;
	const uint16 Emote            = 263;
	const uint16 Spells           = 264;
	const uint16 YouHitOther      = 265;
	const uint16 OtherHitYou      = 266;
	const uint16 YouMissOther     = 267;
	const uint16 OtherMissYou     = 268;
	const uint16 Broadcasts       = 269;
	const uint16 Skills           = 270;
	const uint16 Disciplines      = 271;
	const uint16 Unused1          = 272;
	const uint16 DefaultText      = 273;
	const uint16 Unused2          = 274;
	const uint16 MerchantOffer    = 275;
	const uint16 MerchantExchange = 276;
	const uint16 YourDeath        = 277;
	const uint16 OtherDeath       = 278;
	const uint16 OtherHitOther    = 279;
	const uint16 OtherMissOther   = 280;
	const uint16 Who              = 281;
	const uint16 YellForHelp      = 282;
	const uint16 NonMelee         = 283;
	const uint16 SpellWornOff     = 284;
	const uint16 MoneySplit       = 285;
	const uint16 Loot             = 286;
	const uint16 DiceRoll         = 287;
	const uint16 OtherSpells      = 288;
	const uint16 SpellFailure     = 289;
	const uint16 ChatChannel      = 290;
	const uint16 Chat1            = 291;
	const uint16 Chat2            = 292;
	const uint16 Chat3            = 293;
	const uint16 Chat4            = 294;
	const uint16 Chat5            = 295;
	const uint16 Chat6            = 296;
	const uint16 Chat7            = 297;
	const uint16 Chat8            = 298;
	const uint16 Chat9            = 299;
	const uint16 Chat10           = 300;
	const uint16 MeleeCrit        = 301;
	const uint16 SpellCrit        = 302;
	const uint16 TooFarAway       = 303;
	const uint16 NPCRampage       = 304;
	const uint16 NPCFlurry        = 305;
	const uint16 NPCEnrage        = 306;
	const uint16 EchoSay          = 307;
	const uint16 EchoTell         = 308;
	const uint16 EchoGroup        = 309;
	const uint16 EchoGuild        = 310;
	const uint16 EchoOOC          = 311;
	const uint16 EchoAuction      = 312;
	const uint16 EchoShout        = 313;
	const uint16 EchoEmote        = 314;
	const uint16 EchoChat1        = 315;
	const uint16 EchoChat2        = 316;
	const uint16 EchoChat3        = 317;
	const uint16 EchoChat4        = 318;
	const uint16 EchoChat5        = 319;
	const uint16 EchoChat6        = 320;
	const uint16 EchoChat7        = 321;
	const uint16 EchoChat8        = 322;
	const uint16 EchoChat9        = 323;
	const uint16 EchoChat10       = 324;
	const uint16 DotDamage        = 325;
	const uint16 ItemLink         = 326;
	const uint16 RaidSay          = 327;
	const uint16 MyPet            = 328;
	const uint16 DamageShield     = 329;
	const uint16 LeaderShip       = 330;
	const uint16 PetFlurry        = 331;
	const uint16 PetCritical      = 332;
	const uint16 FocusEffect      = 333;
	const uint16 Experience       = 334;
	const uint16 System           = 335;
	const uint16 PetSpell         = 336;
	const uint16 PetResponse      = 337;
	const uint16 ItemSpeech       = 338;
	const uint16 StrikeThrough    = 339;
	const uint16 Stun             = 340;
};

//ZoneChange_Struct->success values
#define ZONE_ERROR_NOMSG 0
#define ZONE_ERROR_NOTREADY -1
#define ZONE_ERROR_VALIDPC -2
#define ZONE_ERROR_STORYZONE -3
#define ZONE_ERROR_NOEXPANSION -6
#define ZONE_ERROR_NOEXPERIENCE -7


typedef enum {
	FilterNone = 0,
	FilterGuildChat = 1,		//0=hide, 1=show
	FilterSocials = 2,			//0=hide, 1=show
	FilterGroupChat = 3,		//0=hide, 1=show
	FilterShouts = 4,			//0=hide, 1=show
	FilterAuctions = 5,			//0=hide, 1=show
	FilterOOC = 6,				//0=hide, 1=show
	FilterBadWords = 7,			//0=hide, 1=show
	FilterPCSpells = 8,			//0=show, 1=hide, 2=group only
	FilterNPCSpells = 9,		//0=show, 1=hide
	FilterBardSongs = 10,		//0=show, 1=mine only, 2=group only, 3=hide
	FilterSpellCrits = 11,		//0=show, 1=mine only, 2=hide
	FilterMeleeCrits = 12,		//0=show, 1=hide
	FilterSpellDamage = 13,		//0=show, 1=mine only, 2=hide
	FilterMyMisses = 14,		//0=hide, 1=show
	FilterOthersMiss = 15,		//0=hide, 1=show
	FilterOthersHit = 16,		//0=hide, 1=show
	FilterMissedMe = 17,		//0=hide, 1=show
	FilterDamageShields = 18,	//0=show, 1=hide
	FilterDOT = 19,				//0=show, 1=hide
	FilterPetHits = 20,			//0=show, 1=hide
	FilterPetMisses = 21,		//0=show, 1=hide
	FilterFocusEffects = 22,	//0=show, 1=hide
	FilterPetSpells = 23,		//0=show, 1=hide
	FilterHealOverTime = 24,	//0=show, 1=hide
	FilterUnknown25 = 25,
	FilterUnknown26 = 26,
	FilterUnknown27 = 27,
	FilterUnknown28 = 28,
	_FilterCount
} eqFilterType;

typedef enum {
	FilterHide,
	FilterShow,
	FilterShowGroupOnly,
	FilterShowSelfOnly
} eqFilterMode;

#define	STAT_STR		0
#define	STAT_STA		1
#define	STAT_AGI		2
#define	STAT_DEX		3
#define	STAT_INT		4
#define	STAT_WIS		5
#define	STAT_CHA		6
#define	STAT_MAGIC		7
#define	STAT_COLD		8
#define	STAT_FIRE		9
#define	STAT_POISON		10
#define	STAT_DISEASE		11
#define	STAT_MANA		12
#define	STAT_HP			13
#define	STAT_AC			14
#define STAT_ENDURANCE		15
#define STAT_ATTACK		16
#define STAT_HP_REGEN		17
#define STAT_MANA_REGEN		18
#define STAT_HASTE		19
#define STAT_DAMAGE_SHIELD	20

/*
**	Recast timer types. Used as an off set to charProfileStruct timers.
**
**	(Another orphaned enumeration...)
*/
enum RecastTimerTypes
{
	RecTimer_0 = 0,
	RecTimer_1,
	RecTimer_WeaponHealClick,		// 2
	RecTimer_MuramiteBaneNukeClick,	// 3
	RecTimer_4,
	RecTimer_DispellClick,			// 5 (also click heal orbs?)
	RecTimer_Epic,					// 6
	RecTimer_OoWBPClick,			// 7
	RecTimer_VishQuestClassItem,	// 8
	RecTimer_HealPotion,			// 9
	RecTimer_10,
	RecTimer_11,
	RecTimer_12,
	RecTimer_13,
	RecTimer_14,
	RecTimer_15,
	RecTimer_16,
	RecTimer_17,
	RecTimer_18,
	RecTimer_ModRod,				// 19
	_RecTimerCount
};

enum GroupUpdateAction
{
	GUA_Joined = 0,
	GUA_Left = 1,
	GUA_LastLeft = 6,
	GUA_FullGroupInfo = 7,
	GUA_MakeLeader = 8,
	GUA_Started = 9
};

static const uint8 DamageTypeSomething	= 0x1C;	//0x1c is something...
static const uint8 DamageTypeFalling	= 0xFC;
static const uint8 DamageTypeSpell		= 0xE7;
static const uint8 DamageTypeUnknown	= 0xFF;

/*
**	Skill damage types
**
**	(indexed by 'Skill' of SkillUseTypes)
*/
static const uint8 SkillDamageTypes[EQ::skills::HIGHEST_SKILL + 1] = // change to _SkillServerArraySize once activated
{
/*1HBlunt*/					0,
/*1HSlashing*/				1,
/*2HBlunt*/					0,
/*2HSlashing*/				1,
/*Abjuration*/				DamageTypeSpell,
/*Alteration*/				DamageTypeSpell,
/*ApplyPoison*/				DamageTypeUnknown,
/*Archery*/					7,
/*Backstab*/				8,
/*BindWound*/				DamageTypeUnknown,
/*Bash*/					10,
/*Block*/					DamageTypeUnknown,
/*BrassInstruments*/		DamageTypeSpell,
/*Channeling*/				DamageTypeUnknown,
/*Conjuration*/				DamageTypeSpell,
/*Defense*/					DamageTypeUnknown,
/*Disarm*/					DamageTypeUnknown,
/*DisarmTraps*/				DamageTypeUnknown,
/*Divination*/				DamageTypeSpell,
/*Dodge*/					DamageTypeUnknown,
/*DoubleAttack*/			DamageTypeUnknown,
/*DragonPunch*/				21,
/*DualWield*/				DamageTypeUnknown,
/*EagleStrike*/				23,
/*Evocation*/				DamageTypeSpell,
/*FeignDeath*/				4,
/*FlyingKick*/				30,
/*Forage*/					DamageTypeUnknown,
/*HandtoHand*/				4,
/*Hide*/					DamageTypeUnknown,
/*Kick*/					30,
/*Meditate*/				DamageTypeUnknown,
/*Mend*/					DamageTypeUnknown,
/*Offense*/					DamageTypeUnknown,
/*Parry*/					DamageTypeUnknown,
/*PickLock*/				DamageTypeUnknown,
/*1HPiercing*/				36,
/*Riposte*/					DamageTypeUnknown,
/*RoundKick*/				30,
/*SafeFall*/				DamageTypeUnknown,
/*SsenseHeading*/			DamageTypeUnknown,
/*Singing*/					DamageTypeSpell,
/*Sneak*/					DamageTypeUnknown,
/*SpecializeAbjure*/		DamageTypeUnknown,
/*SpecializeAlteration*/	DamageTypeUnknown,
/*SpecializeConjuration*/	DamageTypeUnknown,
/*SpecializeDivination*/	DamageTypeUnknown,
/*SpecializeEvocation*/		DamageTypeUnknown,
/*PickPockets*/				DamageTypeUnknown,
/*StringedInstruments*/		DamageTypeSpell,
/*Swimming*/				DamageTypeUnknown,
/*Throwing*/				51,
/*TigerClaw*/				23,
/*Tracking*/				DamageTypeUnknown,
/*WindInstruments*/			DamageTypeSpell,
/*Fishing*/					DamageTypeUnknown,
/*MakePoison*/				DamageTypeUnknown,
/*Tinkering*/				DamageTypeUnknown,
/*Research*/				DamageTypeUnknown,
/*Alchemy*/					DamageTypeUnknown,
/*Baking*/					DamageTypeUnknown,
/*Tailoring*/				DamageTypeUnknown,
/*SenseTraps*/				DamageTypeUnknown,
/*Blacksmithing*/			DamageTypeUnknown,
/*Fletching*/				DamageTypeUnknown,
/*Brewing*/					DamageTypeUnknown,
/*AlcoholTolerance*/		DamageTypeUnknown,
/*Begging*/					DamageTypeUnknown,
/*JewelryMaking*/			DamageTypeUnknown,
/*Pottery*/					DamageTypeUnknown,
/*PercussionInstruments*/	DamageTypeSpell,
/*Intimidation*/			DamageTypeUnknown,
/*Berserking*/				DamageTypeUnknown,
/*Taunt*/					DamageTypeUnknown,
/*Frenzy*/					74,
/*RemoveTrap*/				DamageTypeUnknown,	// Needs research (set for SenseTrap value)
/*TripleAttack*/			DamageTypeUnknown,	// Needs research (set for DoubleAttack value)
/*2HPiercing*/				36					// Needs research (set for 1HPiercing value - similar to slash/blunt)
};

/*
// Used for worn NPC inventory tracking. NPCs don't use
// augments, so only the basic slots need to be kept track of.
#define MAX_WORN_INVENTORY	22
*/

/*
**	Inventory Slot Equipment Enum
**	Mostly used for third-party tools to reference inventory slots
**
**	[pre-HoT]
**	NOTE: Numbering for personal inventory goes top to bottom, then left to right
**	It's the opposite for inside bags: left to right, then top to bottom
**	Example:
**	Inventory:	Containers:
**	1	5		1	2
**	2	6		3	4
**	3	7		5	6
**	4	8		7	8
**	-	-		9	10
**
**	[HoT and Higher]
**	Note: Numbering for inventory and bags goes left to right, then top to bottom
**	Example:
**	Inventory:	Containers:
**	1	2		1	2
**	3	4		3	4
**	5	6		5	6
**	7	8		7	8
**	9	10		9	10
**	-	-		11	12	[Note: Additional slots are only available in RoF and higher]
**
*/

#define INVALID_INDEX	-1

static const uint32 MAX_SPELL_DB_ID_VAL = 65535;

static const uint32 DB_FACTION_GEM_CHOPPERS = 255;
static const uint32 DB_FACTION_HERETICS = 265;
static const uint32 DB_FACTION_KING_AKANON = 333;

enum ChatChannelNames : uint16
{
	ChatChannel_Guild = 0,
	ChatChannel_Group = 2,
	ChatChannel_Shout = 3,
	ChatChannel_Auction = 4,
	ChatChannel_OOC = 5,
	ChatChannel_Broadcast = 6,
	ChatChannel_Tell = 7,
	ChatChannel_Say = 8,
	ChatChannel_Petition = 10,
	ChatChannel_GMSAY = 11,
	ChatChannel_TellEcho = 14,
	ChatChannel_Raid = 15,

	ChatChannel_UNKNOWN_Guild = 17,
	ChatChannel_UNKNOWN_GMSAY = 18,
	ChatChannel_UCSRelay = 20,
	ChatChannel_Emotes = 22
};

namespace ZoneBlockedSpellTypes {
	const uint8 ZoneWide = 1;
	const uint8 Region   = 2;
};

enum class DynamicZoneType
{
	None = 0,
	Expedition,
	Tutorial,
	Task,
	Mission, // Shared Task
	Quest
};

enum class DynamicZoneMemberStatus : uint8_t
{
	Unknown = 0,
	Online,
	Offline,
	InDynamicZone,
	LinkDead
};

#endif /*COMMON_EQ_CONSTANTS_H*/
