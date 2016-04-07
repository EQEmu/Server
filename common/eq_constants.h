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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef EQ_CONSTANTS_H
#define EQ_CONSTANTS_H

#include "skills.h"
#include "types.h"

/*
** Light Types
**
*/
enum LightTypes
{
	lightTypeNone = 0,
	lightTypeCandle,
	lightTypeTorch,
	lightTypeTinyGlowingSkull,
	lightTypeSmallLantern,
	lightTypeSteinOfMoggok, // 5
	lightTypeLargeLantern,
	lightTypeFlamelessLantern,
	lightTypeGlobeOfStars,
	lightTypeLightGlobe,
	lightTypeLightstone, // 10
	lightTypeGreaterLightstone,
	lightTypeFireBeetleEye,
	lightTypeColdlight,
	lightTypeUnknown1,
	lightTypeUnknown2 // 15
};

#define LIGHT_TYPES_COUNT 16

/*
** Light Levels
**
*/
enum LightLevels
{
	lightLevelUnlit = 0,
	lightLevelCandle,
	lightLevelTorch,
	lightLevelSmallMagic,
	lightLevelRedLight,
	lightLevelBlueLight, // 5
	lightLevelSmallLantern,
	lightLevelMagicLantern,
	lightLevelLargeLantern,
	lightLevelLargeMagic,
	lightLevelBrilliant // 10
};

#define LIGHT_LEVELS_COUNT 11

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
**	(Looking at a recent database, it's possible that some of the item values may be off [10-27-2013])
*/
enum ItemUseTypes : uint8
{
/*9138*/	ItemType1HSlash = 0,
/*9141*/	ItemType2HSlash,
/*9140*/	ItemType1HPiercing,
/*9139*/	ItemType1HBlunt,
/*9142*/	ItemType2HBlunt,
/*5504*/	ItemTypeBow, // 5
/*----*/	ItemTypeUnknown1,
/*----*/	ItemTypeLargeThrowing,
/*5505*/	ItemTypeShield,
/*5506*/	ItemTypeScroll,
/*5507*/	ItemTypeArmor, // 10
/*5508*/	ItemTypeMisc,			// a lot of random crap has this item use.
/*7564*/	ItemTypeLockPick,
/*----*/	ItemTypeUnknown2,
/*5509*/	ItemTypeFood,
/*5510*/	ItemTypeDrink, // 15
/*5511*/	ItemTypeLight,
/*5512*/	ItemTypeCombinable,		// not all stackable items are this use...
/*5513*/	ItemTypeBandage,
/*----*/	ItemTypeSmallThrowing,
/*----*/	ItemTypeSpell, // 20	// spells and tomes
/*5514*/	ItemTypePotion,
/*----*/	ItemTypeUnknown3,
/*0406*/	ItemTypeWindInstrument,
/*0407*/	ItemTypeStringedInstrument,
/*0408*/	ItemTypeBrassInstrument, // 25
/*0405*/	ItemTypePercussionInstrument,
/*5515*/	ItemTypeArrow,
/*----*/	ItemTypeUnknown4,
/*5521*/	ItemTypeJewelry,
/*----*/	ItemTypeSkull, // 30
/*5516*/	ItemTypeBook,			// skill-up tomes/books? (would probably need a pp flag if true...)
/*5517*/	ItemTypeNote,
/*5518*/	ItemTypeKey,
/*----*/	ItemTypeCoin,
/*5520*/	ItemType2HPiercing, // 35
/*----*/	ItemTypeFishingPole,
/*----*/	ItemTypeFishingBait,
/*5519*/	ItemTypeAlcohol,
/*----*/	ItemTypeKey2,			// keys and satchels?? (questable keys?)
/*----*/	ItemTypeCompass, // 40
/*----*/	ItemTypeUnknown5,
/*----*/	ItemTypePoison,			// might be wrong, but includes poisons
/*----*/	ItemTypeUnknown6,
/*----*/	ItemTypeUnknown7,
/*5522*/	ItemTypeMartial, // 45
/*----*/	ItemTypeUnknown8,
/*----*/	ItemTypeUnknown9,
/*----*/	ItemTypeUnknown10,
/*----*/	ItemTypeUnknown11,
/*----*/	ItemTypeSinging, // 50
/*5750*/	ItemTypeAllInstrumentTypes,
/*5776*/	ItemTypeCharm,
/*----*/	ItemTypeDye,
/*----*/	ItemTypeAugmentation,
/*----*/	ItemTypeAugmentationSolvent, // 55
/*----*/	ItemTypeAugmentationDistiller,
/*----*/	ItemTypeUnknown12,
/*----*/	ItemTypeFellowshipKit,
/*----*/	ItemTypeUnknown13,
/*----*/	ItemTypeRecipe, // 60
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
**	Augmentation use type bitmasks (1-based)
**
**	(ref: dbstr_us.txt)
**
*/
enum AugmentationUseTypeBitmasks : uint32 {
	AugUseNone					= 0x00000000,
	AugUseGeneralSingleStat		= 0x00000001,	/*1^16^1 (General: Single Stat)^0*/
	AugUseGeneralMultipleStat	= 0x00000002,	/*2^16^2 (General: Multiple Stat)^0*/
	AugUseGeneralSpellEffect	= 0x00000004,	/*3^16^3 (General: Spell Effect)^0*/
	AugUseWeaponGeneral			= 0x00000008,	/*4^16^4 (Weapon: General)^0*/
	AugUseWeaponElemDamage		= 0x00000010,	/*5^16^5 (Weapon: Elem Damage)^0*/
	AugUseWeaponBaseDamage		= 0x00000020,	/*6^16^6 (Weapon: Base Damage)^0*/
	AugUseGeneralGroup			= 0x00000040,	/*7^16^7 (General: Group)^0*/
	AugUseGeneralRaid			= 0x00000080,	/*8^16^8 (General: Raid)^0*/
	AugUseGeneralDragonsPoints	= 0x00000100,	/*9^16^9 (General: Dragons Points)^0*/
	AugUseCraftedCommon			= 0x00000200,	/*10^16^10 (Crafted: Common)^0*/
	AugUseCraftedGroup1			= 0x00000400,	/*11^16^11 (Crafted: Group)^0*/
	AugUseCraftedRaid1			= 0x00000800,	/*12^16^12 (Crafted: Raid)^0*/
	AugUseEnergeiacGroup		= 0x00001000,	/*13^16^13 (Energeiac: Group)^0*/
	AugUseEnergeiacRaid			= 0x00002000,	/*14^16^14 (Energeiac: Raid)^0*/
	AugUseEmblem				= 0x00004000,	/*15^16^15 (Emblem)^0*/
	AugUseCraftedGroup2			= 0x00008000,	/*16^16^16 (Crafted: Group)^0*/
	AugUseCraftedRaid2			= 0x00010000,	/*17^16^17 (Crafted: Raid)^0*/
	AugUseUnknown1				= 0x00020000,	/*18^16^18^0*/
	AugUseUnknown2				= 0x00040000,	/*19^16^19^0*/
	AugUseOrnamentation			= 0x00080000,	/*20^16^20 (Ornamentation)^0*/
	AugUseSpecialOrnamentation	= 0x00100000,	/*21^16^21 (Special Ornamentation)^0*/
	AugUseUnknown3				= 0x00200000,	/*22^16^22^0*/
	AugUseUnknown4				= 0x00400000,	/*23^16^23^0*/
	AugUseUnknown5				= 0x00800000,	/*24^16^24^0*/
	AugUseUnknown6				= 0x01000000,	/*25^16^25^0*/
	AugUseUnknown7				= 0x02000000,	/*26^16^26^0*/
	AugUseUnknown8				= 0x04000000,	/*27^16^27^0*/
	AugUseUnknown9				= 0x08000000,	/*28^16^28^0*/
	AugUseUnknown10				= 0x10000000,	/*29^16^29^0*/
	AugUseEpic25				= 0x20000000,	/*30^16^30^0*/
	AugUseTest					= 0x40000000,	/*31^16^Test^0*/ // listed as 31^16^31^0 in 5-10 client
	AugUseAll					= 0xFFFFFFFF
};

/*
**	Augmentation use types (enumerated)
**
*/
enum AugmentationUseTypes : uint8 {
	AugTypeNone = 0,
	AugTypeGeneralSingleStat,
	AugTypeGeneralMultipleStat,
	AugTypeGeneralSpellEffect,
	AugTypeWeaponGeneral,
	AugTypeWeaponElemDamage, // 5
	AugTypeWeaponBaseDamage,
	AugTypeGeneralGroup,
	AugTypeGeneralRaid,
	AugTypeGeneralDragonsPoints,
	AugTypeCraftedCommon, // 10
	AugTypeCraftedGroup1,
	AugTypeCraftedRaid1,
	AugTypeEnergeiacGroup,
	AugTypeEnergeiacRaid,
	AugTypeEmblem, // 15
	AugTypeCraftedGroup2,
	AugTypeCraftedRaid2,
	AugTypeUnknown1,
	AugTypeUnknown2,
	AugTypeOrnamentation, // 20
	AugTypeSpecialOrnamentation,
	AugTypeUnknown3,
	AugTypeUnknown4,
	AugTypeUnknown5,
	AugTypeUnknown6, // 25
	AugTypeUnknown7,
	AugTypeUnknown8,
	AugTypeUnknown9,
	AugTypeUnknown10,
	AugTypeEpic25, // 30
	AugTypeTest,
	_AugTypeCount,
	AugTypeAll = 255
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
/*9138*/	AugRestr1HSlash, // 5
/*9139*/	AugRestr1HBlunt,
/*9140*/	AugRestrPiercing,
/*9148*/	AugRestrHandToHand,
/*9141*/	AugRestr2HSlash,
/*9142*/	AugRestr2HBlunt, // 10
/*9143*/	AugRestr2HPierce,
/*9144*/	AugRestrBows,
/*9145*/	AugRestrShields,
/*8052*/	AugRestr1HSlash1HBluntOrHandToHand,
/*9200*/	AugRestr1HBluntOrHandToHand, // 15	// no listed peq entries

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
/*3405*/	BagTypeBackPack, // 5
/*3406*/	BagTypeSmallChest,
/*3407*/	BagTypeLargeChest,
/*----*/	BagTypeBandolier,				// <*Database Reference Only>
/*3408*/	BagTypeMedicineBag,
/*3409*/	BagTypeToolBox, // 10
/*3410*/	BagTypeLexicon,
/*3411*/	BagTypeMortar,
/*3412*/	BagTypeSelfDusting,				// Quest container (Auto-clear contents?)
/*3413*/	BagTypeMixingBowl,
/*3414*/	BagTypeOven, // 15
/*3415*/	BagTypeSewingKit,
/*3416*/	BagTypeForge,
/*3417*/	BagTypeFletchingKit,
/*3418*/	BagTypeBrewBarrel,
/*3419*/	BagTypeJewelersKit, // 20
/*3420*/	BagTypePotteryWheel,
/*3421*/	BagTypeKiln,
/*3422*/	BagTypeKeymaker,				// (no database entries as of peq rev 69)
/*3423*/	BagTypeWizardsLexicon,
/*3424*/	BagTypeMagesLexicon, // 25
/*3425*/	BagTypeNecromancersLexicon,
/*3426*/	BagTypeEnchantersLexicon,
/*----*/	BagTypeUnknown1,				// (a coin pouch/purse?) (no database entries as of peq rev 69)
/*----*/	BagTypeConcordanceofResearch,	// <*Database Reference Only>
/*3427*/	BagTypeAlwaysWorks, // 30		// Quest container (Never-fail combines?)
/*3428*/	BagTypeKoadaDalForge,			// High Elf
/*3429*/	BagTypeTeirDalForge,			// Dark Elf
/*3430*/	BagTypeOggokForge,				// Ogre
/*3431*/	BagTypeStormguardForge,			// Dwarf
/*3432*/	BagTypeAkanonForge, // 35		// Gnome
/*3433*/	BagTypeNorthmanForge,			// Barbarian
/*----*/	BagTypeUnknown2,				// (no database entries as of peq rev 69)
/*3434*/	BagTypeCabilisForge,			// Iksar
/*3435*/	BagTypeFreeportForge,			// Human 1
/*3436*/	BagTypeRoyalQeynosForge, // 40	// Human 2
/*3439*/	BagTypeHalflingTailoringKit,
/*3438*/	BagTypeErudTailoringKit,
/*3440*/	BagTypeFierDalTailoringKit,		// Wood Elf
/*3441*/	BagTypeFierDalFletchingKit,		// Wood Elf
/*3437*/	BagTypeIksarPotteryWheel, // 45
/*3442*/	BagTypeTackleBox,
/*3443*/	BagTypeTrollForge,
/*3445*/	BagTypeFierDalForge,			// Wood Elf
/*3444*/	BagTypeValeForge,				// Halfling
/*3446*/	BagTypeErudForge, // 50
/*----*/	BagTypeTradersSatchel,			// <*Database Reference Only> (db: Yellow Trader's Satchel Token?)
/*5785*/	BagTypeGuktaForge,				// Froglok (no database entries as of peq rev 69)
/*3359*/	BagTypeAugmentationSealer,
/*----*/	BagTypeIceCreamChurn,			// <*Database Reference Only>
/*6325*/	BagTypeTransformationmold, // 55	// Ornamentation
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

//SpawnAppearance types: (compared two clients for server-originating types: SoF & RoF2)
#define AT_Die 0			// this causes the client to keel over and zone to bind point (default action)
#define AT_WhoLevel 1		// the level that shows up on /who
//#define AT_2 2			// unknown
#define AT_Invis 3			// 0 = visible, 1 = invisible
#define AT_PVP 4			// 0 = blue, 1 = pvp (red)
#define AT_Light 5			// light type emitted by player (lightstone, shiny shield)
#define AT_Anim 14			// 100=standing, 110=sitting, 111=ducking, 115=feigned, 105=looting
#define AT_Sneak 15			// 0 = normal, 1 = sneaking
#define AT_SpawnID 16		// server to client, sets player spawn id
#define AT_HP 17			// Client->Server, my HP has changed (like regen tic)
#define AT_Linkdead 18		// 0 = normal, 1 = linkdead
#define AT_Levitate 19		// 0=off, 1=flymode, 2=levitate
#define AT_GM 20			// 0 = normal, 1 = GM - all odd numbers seem to make it GM
#define AT_Anon 21			// 0 = normal, 1 = anon, 2 = roleplay
#define AT_GuildID 22
#define AT_GuildRank 23		// 0=member, 1=officer, 2=leader
#define AT_AFK 24			// 0 = normal, 1 = afk
#define AT_Pet 25			// Param is EntityID of owner, or 0 for when charm breaks
//#define AT_27 27			// unknown
#define AT_Split 28			// 0 = normal, 1 = autosplit on (not showing in SoF+) (client-to-server only)
#define AT_Size 29			// spawn's size (present: SoF, absent: RoF2)
//#define AT_30 30			// unknown
#define AT_NPCName 31		// change PC's name's color to NPC color 0 = normal, 1 = npc name
//#define AT_32 32			// unknown
//#define AT_33 33			// unknown
//#define AT_34 34			// unknown (present: SoF, absent: RoF2)
//#define AT_35 35			// unknown
//#define AT_36 36			// unknown
//#define AT_37 37			// unknown
//#define AT_38 38			// unknown
//#define AT_39 39			// unknown
#define AT_ShowHelm 43		// 0 = hide graphic, 1 = show graphic
#define AT_DamageState 44	// The damage state of a destructible object (0 through 4)
//#define AT_46 46			// unknown
//#define AT_48 48			// unknown
//#define AT_49 49			// unknown
//#define AT_52 52			// (absent: SoF, present: RoF2) (not a replacement for RoF absent 29 or 34)
//#define AT_53 53			// (absent: SoF, present: RoF2) (not a replacement for RoF absent 29 or 34)

//#define AT_Trader 300		// Bazaar Trader Mode (not present in SoF or RoF2)

// animations for AT_Anim
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

// TODO: Really should combine above and below into one

//from showeq
enum ChatColor
{
	/*
	CC_Default					= 0,
	CC_DarkGrey					= 1,
	CC_DarkGreen				= 2,
	CC_DarkBlue					= 3,
	CC_Purple					= 5,
	CC_LightGrey				= 6,
	*/

	CC_WhiteSmoke				= 0,	// FF|F0F0F0
	CC_Green					= 2,	// FF|008000
	CC_BrightBlue				= 3,	// FF|0040FF
	CC_Magenta					= 5,	// FF|F000F0
	CC_Gray						= 6,	// FF|808080
	CC_LightGray				= 7,	// FF|E0E0E0
	//CC_WhiteSmoke2				= 10,	// FF|F0F0F0
	CC_DarkGray					= 12,	// FF|A0A0A0
	CC_Red						= 13,	// FF|F00000
	CC_Lime						= 14,	// FF|00F000
	CC_Yellow					= 15,	// FF|F0F000
	CC_Blue						= 16,	// FF|0000F0
	CC_LightNavy				= 17,	// FF|0000AF
	CC_Cyan						= 18,	// FF|00F0F0
	CC_Black					= 20,	// FF|000000
	
	// any index <= 255 that is not defined above
	CC_DimGray					= 1,	// FF|606060
	CC_Default					= 1,

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
/*Frenzy*/					74,
/*RemoveTrap*/				DamageTypeUnknown,	// Needs research (set for SenseTrap value)
/*TripleAttack*/			DamageTypeUnknown,	// Needs research (set for DoubleAttack value)
/*2HPiercing*/				36					// Needs research (set for 1HPiercing value - similar to slash/blunt)
};

/*
**	Material use slots
**
*/
enum MaterialSlots : uint8
{
	MaterialHead = 0,
	MaterialChest,
	MaterialArms,
	MaterialWrist,
	MaterialHands,
	MaterialLegs, // 5
	MaterialFeet,
	MaterialPrimary,
	MaterialSecondary,
	MaterialCount,
	MaterialInvalid = 255
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

enum InventoryTypes : int16
{
	TypePossessions = 0,
	TypeBank,
	TypeSharedBank,
	TypeTrade,
	TypeWorld,
	TypeLimbo, // 5
	TypeTribute,
	TypeTrophyTribute,
	TypeGuildTribute,
	TypeMerchant,
	TypeDeleted, // 10
	TypeCorpse,
	TypeBazaar,
	TypeInspect,
	TypeRealEstate,
	TypeViewMODPC, // 15
	TypeViewMODBank,
	TypeViewMODSharedBank,
	TypeViewMODLimbo,
	TypeAltStorage,
	TypeArchived, // 20
	TypeMail,
	TypeGuildTrophyTribute,
	TypeKrono,
	TypeOther,
	TypeCount
};

enum PossessionsSlots : int16
{
	SlotCharm = 0,
	SlotEar1,
	SlotHead,
	SlotFace,
	SlotEar2,
	SlotNeck, // 5
	SlotShoulders,
	SlotArms,
	SlotBack,
	SlotWrist1,
	SlotWrist2, // 10
	SlotRange,
	SlotHands,
	SlotPrimary,
	SlotSecondary,
	SlotFinger1, // 15
	SlotFinger2,
	SlotChest,
	SlotLegs,
	SlotFeet,
	SlotWaist, // 20
	SlotPowerSource = 9999, // temp
	SlotAmmo = 21, // temp
	SlotGeneral1,
	SlotGeneral2,
	SlotGeneral3,
	SlotGeneral4, // 25
	SlotGeneral5,
	SlotGeneral6,
	SlotGeneral7,
	SlotGeneral8,
	//SlotGeneral9,
	//SlotGeneral10,
	SlotCursor, // 30
	SlotCount
};

#define INVALID_INDEX	-1
#define NOT_USED		0
#define NO_ITEM			0

// yes..these are redundant... but, they help to identify and define what is actually being performed
// plus, since they're pre-op's, they don't affect the actual binary size
#define TYPE_BEGIN	0
#define SLOT_BEGIN	0
#define SUB_BEGIN	0
#define AUG_BEGIN	0

namespace legacy {
	// this is for perl and other legacy systems

	typedef enum {
		SLOT_CHARM			= 0,
		SLOT_EAR01			= 1,
		SLOT_HEAD			= 2,
		SLOT_FACE			= 3,
		SLOT_EAR02			= 4,
		SLOT_NECK			= 5,
		SLOT_SHOULDER		= 6,
		SLOT_ARMS			= 7,
		SLOT_BACK			= 8,
		SLOT_BRACER01		= 9,
		SLOT_BRACER02		= 10,
		SLOT_RANGE			= 11,
		SLOT_HANDS			= 12,
		SLOT_PRIMARY		= 13,
		SLOT_SECONDARY		= 14,
		SLOT_RING01			= 15,
		SLOT_RING02			= 16,
		SLOT_CHEST			= 17,
		SLOT_LEGS			= 18,
		SLOT_FEET			= 19,
		SLOT_WAIST			= 20,
		SLOT_POWER_SOURCE	= 9999,
		SLOT_AMMO			= 21,
		SLOT_GENERAL_1		= 22,
		SLOT_GENERAL_2		= 23,
		SLOT_GENERAL_3		= 24,
		SLOT_GENERAL_4		= 25,
		SLOT_GENERAL_5		= 26,
		SLOT_GENERAL_6		= 27,
		SLOT_GENERAL_7		= 28,
		SLOT_GENERAL_8		= 29,
		//SLOT_GENERAL_9	= not supported
		//SLOT_GENERAL_10	= not supported
		SLOT_CURSOR			= 30,
		SLOT_CURSOR_END		= (int16)0xFFFE, // I hope no one is using this...
		SLOT_TRADESKILL		= 1000,
		SLOT_AUGMENT		= 1001,
		SLOT_INVALID		= (int16)0xFFFF,

		SLOT_POSSESSIONS_BEGIN	= 0,
		SLOT_POSSESSIONS_END	= 30,

		SLOT_EQUIPMENT_BEGIN	= 0,
		SLOT_EQUIPMENT_END		= 21,

		SLOT_PERSONAL_BEGIN		= 22,
		SLOT_PERSONAL_END		= 29,
		SLOT_PERSONAL_BAGS_BEGIN	= 251,
		SLOT_PERSONAL_BAGS_END		= 330,

		SLOT_CURSOR_BAG_BEGIN		= 331,
		SLOT_CURSOR_BAG_END			= 340,

		SLOT_TRIBUTE_BEGIN	= 400,
		SLOT_TRIBUTE_END	= 404,

		SLOT_BANK_BEGIN			= 2000,
		SLOT_BANK_END			= 2023,
		SLOT_BANK_BAGS_BEGIN	= 2031,
		SLOT_BANK_BAGS_END		= 2270,

		SLOT_SHARED_BANK_BEGIN		= 2500,
		SLOT_SHARED_BANK_END		= 2501,
		SLOT_SHARED_BANK_BAGS_BEGIN	= 2531,
		SLOT_SHARED_BANK_BAGS_END	= 2550,

		SLOT_TRADE_BEGIN		= 3000,
		SLOT_TRADE_END			= 3007,
		SLOT_TRADE_BAGS_BEGIN	= 3031,
		SLOT_TRADE_BAGS_END		= 3110,

		SLOT_WORLD_BEGIN	= 4000,
		SLOT_WORLD_END		= 4009
	} InventorySlot;
}

static const uint32 MAX_SPELL_DB_ID_VAL = 65535;

#endif
