/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef EQ_CONSTANTS_H
#define EQ_CONSTANTS_H

#include "skills.h"

/*
**	Item attributes
**
**	(There are no grepwin hits other than these declarations... Do they have a use?)
*/
enum ItemAttributes : uint32
{
	ItemAttrNone			= 0x00000000,
	ItemAttrLore			= 0x00000001,
	ItemAttrArtifact		= 0x00000002,
	ItemAttrSummoned		= 0x00000004,
	ItemAttrMagic			= 0x00000008,
	ItemAttrAugment			= 0x00000010,
	ItemAttrPendingLore		= 0x00000020,
	ItemAttrUnknown			= 0xFFFFFFFF
};

/*
**	Item class types
**
*/
enum ItemClassTypes
{
	ItemClassCommon = 0,
	ItemClassContainer,
	ItemClassBook,
	_ItemClassCount
};

/*
**	Item use types
**
**	(ref: database and eqstr_us.txt)
**
**	(Looking at a recent database, it's possible that some of the item values may be off [10-27-2013] -U)
*/
enum ItemUseTypes : uint8
{
/*9138*/	ItemType1HSlash = 0,
/*9141*/	ItemType2HSlash,
/*9140*/	ItemType1HPiercing,
/*9139*/	ItemType1HBlunt,
/*9142*/	ItemType2HBlunt,
/*5504*/	ItemTypeBow,
/*----*/	ItemTypeUnknown1,
/*----*/	ItemTypeLargeThrowing,
/*5505*/	ItemTypeShield,
/*5506*/	ItemTypeScroll,
/*5507*/	ItemTypeArmor,
/*5508*/	ItemTypeMisc,			// a lot of random crap has this item use.
/*7564*/	ItemTypeLockPick,
/*----*/	ItemTypeUnknown2,
/*5509*/	ItemTypeFood,
/*5510*/	ItemTypeDrink,
/*5511*/	ItemTypeLight,
/*5512*/	ItemTypeCombinable,		// not all stackable items are this use...
/*5513*/	ItemTypeBandage,
/*----*/	ItemTypeSmallThrowing,
/*----*/	ItemTypeSpell,			// spells and tomes
/*5514*/	ItemTypePotion,
/*----*/	ItemTypeUnknown3,
/*0406*/	ItemTypeWindInstrument,
/*0407*/	ItemTypeStringedInstrument,
/*0408*/	ItemTypeBrassInstrument,
/*0405*/	ItemTypePercussionInstrument,
/*5515*/	ItemTypeArrow,
/*----*/	ItemTypeUnknown4,
/*5521*/	ItemTypeJewelry,
/*----*/	ItemTypeSkull,
/*5516*/	ItemTypeBook,			// skill-up tomes/books? (would probably need a pp flag if true...)
/*5517*/	ItemTypeNote,
/*5518*/	ItemTypeKey,
/*----*/	ItemTypeCoin,
/*5520*/	ItemType2HPiercing,
/*----*/	ItemTypeFishingPole,
/*----*/	ItemTypeFishingBait,
/*5519*/	ItemTypeAlcohol,
/*----*/	ItemTypeKey2,			// keys and satchels?? (questable keys?)
/*----*/	ItemTypeCompass,
/*----*/	ItemTypeUnknown5,
/*----*/	ItemTypePoison,			// might be wrong, but includes poisons
/*----*/	ItemTypeUnknown6,
/*----*/	ItemTypeUnknown7,
/*5522*/	ItemTypeMartial,
/*----*/	ItemTypeUnknown8,
/*----*/	ItemTypeUnknown9,
/*----*/	ItemTypeUnknown10,
/*----*/	ItemTypeUnknown11,
/*----*/	ItemTypeSinging,
/*5750*/	ItemTypeAllInstrumentTypes,
/*5776*/	ItemTypeCharm,
/*----*/	ItemTypeDye,
/*----*/	ItemTypeAugmentation,
/*----*/	ItemTypeAugmentationSolvent,
/*----*/	ItemTypeAugmentationDistiller,
/*----*/	ItemTypeUnknown12,
/*----*/	ItemTypeFellowshipKit,
/*----*/	ItemTypeUnknown13,
/*----*/	ItemTypeRecipe,
/*----*/	ItemTypeAdvancedRecipe,
/*----*/	ItemTypeJournal,		// only one(1) database entry
/*----*/	ItemTypeAltCurrency,	// alt-currency (as opposed to coinage)
/*5881*/	ItemTypePerfectedAugmentationDistiller,
/*----*/	_ItemTypeCount

/*
	Unknowns:

	Mounts?
	Ornamentations?
	GuildBanners?
	Collectible? 
	Placeable?
	(others?)
*/
};

/*
**	Augmentation use types (in-work)
**
**	(ref: dbstr_us.txt)
**
*/
enum AugmentationUseTypes : uint32 {
	AugTypeNone = 0,				// not 100% sure on this...
	AugTypeGeneralSingleStat,		/*1^16^1 (General: Single Stat)^0*/
	AugTypeGeneralMultipleStat,		/*2^16^2 (General: Multiple Stat)^0*/
	AugTypeGeneralSpellEffect,		/*3^16^3 (General: Spell Effect)^0*/
	AugTypeWeaponGeneral,			/*4^16^4 (Weapon: General)^0*/
	AugTypeWeaponElemDamage,		/*5^16^5 (Weapon: Elem Damage)^0*/
	AugTypeWeaponBaseDamage,		/*6^16^6 (Weapon: Base Damage)^0*/
	AugTypeGeneralGroup,			/*7^16^7 (General: Group)^0*/
	AugTypeGeneralRaid,				/*8^16^8 (General: Raid)^0*/
	AugTypeGeneralDragonsPoints,	/*9^16^9 (General: Dragons Points)^0*/
	AugTypeCraftedCommon,			/*10^16^10 (Crafted: Common)^0*/
	AugTypeCraftedGroup1,			/*11^16^11 (Crafted: Group)^0*/
	AugTypeCraftedRaid1,			/*12^16^12 (Crafted: Raid)^0*/
	AugTypeEnergeiacGroup,			/*13^16^13 (Energeiac: Group)^0*/
	AugTypeEnergeiacRaid,			/*14^16^14 (Energeiac: Raid)^0*/
	AugTypeEmblem,					/*15^16^15 (Emblem)^0*/
	AugTypeCraftedGroup2,			/*16^16^16 (Crafted: Group)^0*/
	AugTypeCraftedRaid2,			/*17^16^17 (Crafted: Raid)^0*/
	AugTypeUnknown1,				/*18^16^18^0*/
	AugTypeUnknown2,				/*19^16^19^0*/
	AugTypeOrnamentation,			/*20^16^20 (Ornamentation)^0*/
	AugTypeSpecialOrnamentation,	/*21^16^21 (Special Ornamentation)^0*/
	AugTypeUnknown3,				/*22^16^22^0*/
	AugTypeUnknown4,				/*23^16^23^0*/
	AugTypeUnknown5,				/*24^16^24^0*/
	AugTypeUnknown6,				/*25^16^25^0*/
	AugTypeUnknown7,				/*26^16^26^0*/
	AugTypeUnknown8,				/*27^16^27^0*/
	AugTypeUnknown9,				/*28^16^28^0*/
	AugTypeUnknown10,				/*29^16^29^0*/
	AugTypeEpic25,					/*30^16^30^0*/
	AugTypeTest,					/*31^16^Test^0*/ // listed as 31^16^31^0 in 5-10 client
	_AugTypeCount
};

/*
**	Augmentation restriction types (in-work)
**
**	(ref: eqstr_us.txt)
**
*/
enum AugmentationRestrictionTypes : uint8 {
/*4690*/	AugRestrAny = 0,
/*9134*/	AugRestrArmor,
/*9135*/	AugRestrWeapons,
/*9136*/	AugRestr1HWeapons,
/*9137*/	AugRestr2HWeapons,
/*9138*/	AugRestr1HSlash,
/*9139*/	AugRestr1HBlunt,
/*9140*/	AugRestrPiercing,
/*9148*/	AugRestrHandToHand,
/*9141*/	AugRestr2HSlash,
/*9142*/	AugRestr2HBlunt,
/*9143*/	AugRestr2HPierce,
/*9144*/	AugRestrBows,
/*9145*/	AugRestrShields,
/*8052*/	AugRestr1HSlash1HBluntOrHandToHand,
/*9200*/	AugRestr1HBluntOrHandToHand,		// no listed peq entries

// these three appear to be post-RoF (12-10-2012) and can not be verified until RoF (05-10-2013) is supported
/*????*/	AugRestrUnknown1,
/*????*/	AugRestrUnknown2,
/*????*/	AugRestrUnknown3,					// last value in peq entries
			_AugRestrCount

/*4687*/	//AugTypeAllItems, // ?? unknown atm
/*4688*/	//AugTypePrestige, // ?? unknown atm
/*4689*/	//AugTypeNonPrestige, // ?? unknown atm
};

/*
**	Container use types
**
**	This correlates to world 'object.type' (object.h/Object.cpp) as well as Item_Struct.BagType
**
**	(ref: database, web forums and eqstr_us.txt)
*/
enum ContainerUseTypes : uint8
{
/*3400*/	BagTypeSmallBag = 0,
/*3401*/	BagTypeLargeBag,
/*3402*/	BagTypeQuiver,
/*3403*/	BagTypeBeltPouch,
/*3404*/	BagTypeWristPouch,
/*3405*/	BagTypeBackPack,
/*3406*/	BagTypeSmallChest,
/*3407*/	BagTypeLargeChest,
/*----*/	BagTypeBandolier,				// <*Database Reference Only>
/*3408*/	BagTypeMedicineBag,
/*3409*/	BagTypeToolBox,
/*3410*/	BagTypeLexicon,
/*3411*/	BagTypeMortar,
/*3412*/	BagTypeSelfDusting,				// Quest container (Auto-clear contents?)
/*3413*/	BagTypeMixingBowl,
/*3414*/	BagTypeOven,
/*3415*/	BagTypeSewingKit,
/*3416*/	BagTypeForge,
/*3417*/	BagTypeFletchingKit,
/*3418*/	BagTypeBrewBarrel,
/*3419*/	BagTypeJewelersKit,
/*3420*/	BagTypePotteryWheel,
/*3421*/	BagTypeKiln,
/*3422*/	BagTypeKeymaker,				// (no database entries as of peq rev 69)
/*3423*/	BagTypeWizardsLexicon,
/*3424*/	BagTypeMagesLexicon,
/*3425*/	BagTypeNecromancersLexicon,
/*3426*/	BagTypeEnchantersLexicon,
/*----*/	BagTypeUnknown1,				// (a coin pouch/purse?) (no database entries as of peq rev 69)
/*----*/	BagTypeConcordanceofResearch,	// <*Database Reference Only>
/*3427*/	BagTypeAlwaysWorks,				// Quest container (Never-fail combines?)
/*3428*/	BagTypeKoadaDalForge,			// High Elf
/*3429*/	BagTypeTeirDalForge,			// Dark Elf
/*3430*/	BagTypeOggokForge,				// Ogre
/*3431*/	BagTypeStormguardForge,			// Dwarf
/*3432*/	BagTypeAkanonForge,				// Gnome
/*3433*/	BagTypeNorthmanForge,			// Barbarian
/*----*/	BagTypeUnknown2,				// (no database entries as of peq rev 69)
/*3434*/	BagTypeCabilisForge,			// Iksar
/*3435*/	BagTypeFreeportForge,			// Human 1
/*3436*/	BagTypeRoyalQeynosForge,		// Human 2
/*3439*/	BagTypeHalflingTailoringKit,
/*3438*/	BagTypeErudTailoringKit,
/*3440*/	BagTypeFierDalTailoringKit,		// Wood Elf
/*3441*/	BagTypeFierDalFletchingKit,		// Wood Elf
/*3437*/	BagTypeIksarPotteryWheel,
/*3442*/	BagTypeTackleBox,
/*3443*/	BagTypeTrollForge,
/*3445*/	BagTypeFierDalForge,			// Wood Elf
/*3444*/	BagTypeValeForge,				// Halfling
/*3446*/	BagTypeErudForge,
/*----*/	BagTypeTradersSatchel,			// <*Database Reference Only> (db: Yellow Trader's Satchel Token?)
/*5785*/	BagTypeGuktaForge,				// Froglok (no database entries as of peq rev 69)
/*3359*/	BagTypeAugmentationSealer,
/*----*/	BagTypeIceCreamChurn,			// <*Database Reference Only>
/*6325*/	BagTypeTransformationmold,		// Ornamentation
/*6340*/	BagTypeDetransformationmold,	// Ornamentation Stripper
/*5400*/	BagTypeUnattuner,
/*7684*/	BagTypeTradeskillBag,
/*7692*/	BagTypeCollectibleBag,
/*----*/	_BagTypeCount
};

/*
** Item Effect Types
**
*/
enum {
	ET_CombatProc = 0,
	ET_ClickEffect = 1,
	ET_WornEffect = 2,
	ET_Expendable = 3,
	ET_EquipClick = 4,
	ET_ClickEffect2 = 5,	//name unknown
	ET_Focus = 6,
	ET_Scroll = 7
};

//SpawnAppearance types:
#define AT_Die			0	// this causes the client to keel over and zone to bind point
#define AT_WhoLevel		1	// the level that shows up on /who
#define AT_Invis		3	// 0 = visible, 1 = invisible
#define AT_PVP			4	// 0 = blue, 1 = pvp (red)
#define AT_Light		5	// light type emitted by player (lightstone, shiny shield)
#define AT_Anim			14	// 100=standing, 110=sitting, 111=ducking, 115=feigned, 105=looting
#define AT_Sneak		15	// 0 = normal, 1 = sneaking
#define AT_SpawnID		16	// server to client, sets player spawn id
#define AT_HP			17	// Client->Server, my HP has changed (like regen tic)
#define AT_Linkdead		18	// 0 = normal, 1 = linkdead
#define AT_Levitate		19	// 0=off, 1=flymode, 2=levitate
#define AT_GM			20	// 0 = normal, 1 = GM - all odd numbers seem to make it GM
#define AT_Anon			21	// 0 = normal, 1 = anon, 2 = roleplay
#define AT_GuildID		22
#define AT_GuildRank	23	// 0=member, 1=officer, 2=leader
#define AT_AFK			24	// 0 = normal, 1 = afk
#define AT_Pet			25	// Param is EntityID of owner, or 0 for when charm breaks
#define AT_Split		28	// 0 = normal, 1 = autosplit on
#define AT_Size			29	// spawn's size
#define AT_NPCName		31	// change PC's name's color to NPC color 0 = normal, 1 = npc name
#define AT_ShowHelm		43	// 0 = do not show helmet graphic, 1 = show graphic
#define AT_DamageState	44	// The damage state of a destructible object (0 through 4)
//#define AT_Trader		300	// Bazzar Trader Mode

// solar: animations for AT_Anim
#define ANIM_FREEZE	102
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

// msg_type's for custom usercolors
#define MT_Say					256
#define MT_Tell					257
#define MT_Group				258
#define MT_Guild				259
#define MT_OOC					260
#define MT_Auction				261
#define MT_Shout				262
#define MT_Emote				263
#define MT_Spells				264
#define MT_YouHitOther			265
#define MT_OtherHitsYou			266
#define MT_YouMissOther			267
#define MT_OtherMissesYou		268
#define MT_Broadcasts			269
#define MT_Skills				270
#define MT_Disciplines			271
#define	MT_Unused1				272
#define MT_DefaultText			273
#define MT_Unused2				274
#define MT_MerchantOffer		275
#define MT_MerchantBuySell		276
#define	MT_YourDeath			277
#define MT_OtherDeath			278
#define MT_OtherHits			279
#define MT_OtherMisses			280
#define	MT_Who					281
#define MT_YellForHelp			282
#define MT_NonMelee				283
#define MT_WornOff				284
#define MT_MoneySplit			285
#define MT_LootMessages			286
#define MT_DiceRoll				287
#define MT_OtherSpells			288
#define MT_SpellFailure			289
#define MT_Chat					290
#define MT_Channel1				291
#define MT_Channel2				292
#define MT_Channel3				293
#define MT_Channel4				294
#define MT_Channel5				295
#define MT_Channel6				296
#define MT_Channel7				297
#define MT_Channel8				298
#define MT_Channel9				299
#define MT_Channel10			300
#define MT_CritMelee			301
#define MT_SpellCrits			302
#define MT_TooFarAway			303
#define MT_NPCRampage			304
#define MT_NPCFlurry			305
#define MT_NPCEnrage			306
#define MT_SayEcho				307
#define MT_TellEcho				308
#define MT_GroupEcho			309
#define MT_GuildEcho			310
#define MT_OOCEcho				311
#define MT_AuctionEcho			312
#define MT_ShoutECho			313
#define MT_EmoteEcho			314
#define MT_Chat1Echo			315
#define MT_Chat2Echo			316
#define MT_Chat3Echo			317
#define MT_Chat4Echo			318
#define MT_Chat5Echo			319
#define MT_Chat6Echo			320
#define MT_Chat7Echo			321
#define MT_Chat8Echo			322
#define MT_Chat9Echo			323
#define MT_Chat10Echo			324
#define MT_DoTDamage			325
#define MT_ItemLink				326
#define MT_RaidSay				327
#define MT_MyPet				328
#define MT_DS					329
#define MT_Leadership			330
#define MT_PetFlurry			331
#define MT_PetCrit				332
#define MT_FocusEffect			333
#define MT_Experience			334
#define MT_System				335
#define MT_PetSpell				336
#define MT_PetResponse			337
#define MT_ItemSpeech			338
#define MT_StrikeThrough		339
#define MT_Stun					340

//from showeq
enum ChatColor
{
	CC_Default					= 0,
	CC_DarkGrey					= 1,
	CC_DarkGreen				= 2,
	CC_DarkBlue					= 3,
	CC_Purple					= 5,
	CC_LightGrey				= 6,
	CC_User_Say					= 256,
	CC_User_Tell				= 257,
	CC_User_Group				= 258,
	CC_User_Guild				= 259,
	CC_User_OOC					= 260,
	CC_User_Auction				= 261,
	CC_User_Shout				= 262,
	CC_User_Emote				= 263,
	CC_User_Spells				= 264,
	CC_User_YouHitOther			= 265,
	CC_User_OtherHitYou			= 266,
	CC_User_YouMissOther		= 267,
	CC_User_OtherMissYou		= 268,
	CC_User_Duels				= 269,
	CC_User_Skills				= 270,
	CC_User_Disciplines			= 271,
	CC_User_Default				= 273,
	CC_User_MerchantOffer		= 275,
	CC_User_MerchantExchange	= 276,
	CC_User_YourDeath			= 277,
	CC_User_OtherDeath			= 278,
	CC_User_OtherHitOther		= 279,
	CC_User_OtherMissOther		= 280,
	CC_User_Who					= 281,
	CC_User_Yell				= 282,
	CC_User_NonMelee			= 283,
	CC_User_SpellWornOff		= 284,
	CC_User_MoneySplit			= 285,
	CC_User_Loot				= 286,
	CC_User_Random				= 287,
	CC_User_OtherSpells			= 288,
	CC_User_SpellFailure		= 289,
	CC_User_ChatChannel			= 290,
	CC_User_Chat1				= 291,
	CC_User_Chat2				= 292,
	CC_User_Chat3				= 293,
	CC_User_Chat4				= 294,
	CC_User_Chat5				= 295,
	CC_User_Chat6				= 296,
	CC_User_Chat7				= 297,
	CC_User_Chat8				= 298,
	CC_User_Chat9				= 299,
	CC_User_Chat10				= 300,
	CC_User_MeleeCrit			= 301,
	CC_User_SpellCrit			= 302,
	CC_User_TooFarAway			= 303,
	CC_User_NPCRampage			= 304,
	CC_User_NPCFurry			= 305,
	CC_User_NPCEnrage			= 306,
	CC_User_EchoSay				= 307,
	CC_User_EchoTell			= 308,
	CC_User_EchoGroup			= 309,
	CC_User_EchoGuild			= 310,
	CC_User_EchoOOC				= 311,
	CC_User_EchoAuction			= 312,
	CC_User_EchoShout			= 313,
	CC_User_EchoEmote			= 314,
	CC_User_EchoChat1			= 315,
	CC_User_EchoChat2			= 316,
	CC_User_EchoChat3			= 317,
	CC_User_EchoChat4			= 318,
	CC_User_EchoChat5			= 319,
	CC_User_EchoChat6			= 320,
	CC_User_EchoChat7			= 321,
	CC_User_EchoChat8			= 322,
	CC_User_EchoChat9			= 323,
	CC_User_EchoChat10			= 324,
	CC_User_UnusedAtThisTime	= 325,
	CC_User_ItemTags			= 326,
	CC_User_RaidSay				= 327,
	CC_User_MyPet				= 328,
	CC_User_DamageShield		= 329,
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
static const uint8 SkillDamageTypes[HIGHEST_SKILL + 1] = // change to _SkillServerArraySize once activated
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
/*Frenzy*/					74 //,
// /*RemoveTrap*/				DamageTypeUnknown,	// Needs research (set for SenseTrap value)
// /*TripleAttack*/			DamageTypeUnknown,	// Needs research (set for DoubleAttack value)
// /*2HPiercing*/				36					// Needs research (set for 1HPiercing value - similar to slash/blunt)
};

/*
**	Material use slots
**
*/
enum MaterialUseSlots : uint8
{
	MaterialHead = 0,
	MaterialChest,
	MaterialArms,
	MaterialWrist,
	MaterialHands,
	MaterialLegs,
	MaterialFeet,
	MaterialPrimary,
	MaterialSecondary,
	_MaterialCount,
	_MaterialInvalid = 255
};

// Used for worn NPC inventory tracking. NPCs don't use
// augments, so only the basic slots need to be kept track of.
#define MAX_WORN_INVENTORY	22


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

enum InventorySlot
{
	////////////////////////
	// Equip slots
	////////////////////////

	SLOT_CHARM		= 0,
	SLOT_EAR01		= 1,
	SLOT_HEAD		= 2,
	SLOT_FACE		= 3,
	SLOT_EAR02		= 4,
	SLOT_NECK		= 5,
	SLOT_SHOULDER	= 6,
	SLOT_ARMS		= 7,
	SLOT_BACK		= 8,
	SLOT_BRACER01	= 9,
	SLOT_BRACER02	= 10,
	SLOT_RANGE		= 11,
	SLOT_HANDS		= 12,
	SLOT_PRIMARY	= 13,
	SLOT_SECONDARY	= 14,
	SLOT_RING01		= 15,
	SLOT_RING02		= 16,
	SLOT_CHEST		= 17,
	SLOT_LEGS		= 18,
	SLOT_FEET		= 19,
	SLOT_WAIST		= 20,
	SLOT_AMMO		= 21,

	////////////////////////
	// All other slots
	////////////////////////
	SLOT_PERSONAL_BEGIN = 22,
	SLOT_PERSONAL_END = 29,

	SLOT_CURSOR		= 30,

	SLOT_CURSOR_END	= (int16)0xFFFE,	// Last item on cursor queue
	// Cursor bag slots are 331->340 (10 slots)

	// Personal Inventory Slots
	// Slots 1 through 8 are slots 22->29
	// Inventory bag slots are 251->330 (10 slots per bag)

	// Tribute slots are 400-404? (upper bound unknown)
	// storing these in worn item's map

	// Bank slots
	// Bank slots 1 through 16 are slots 2000->2015
	// Bank bag slots are 2031->2190

	// Shared bank slots
	// Shared bank slots 1 through 2 are slots 2500->2501
	// Shared bank bag slots are 2531->2550

	// Trade session slots
	// Trade slots 1 through 8 are slots 3000->3007
	// Trade bag slots are technically 0->79 when passed to client,
	// but in our code, we treat them as slots 3100->3179

	// Slot used in OP_TradeSkillCombine for world tradeskill containers
	SLOT_TRADESKILL = 1000,
	SLOT_AUGMENT = 1001,
	SLOT_POWER_SOURCE = 9999,
	// Value recognized by client for destroying an item
	SLOT_INVALID = (int16)0xFFFF
};


#endif
