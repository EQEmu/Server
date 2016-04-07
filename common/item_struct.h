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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 04111-1307 USA
*/

#ifndef ITEM_STRUCT_H
#define ITEM_STRUCT_H

/*
 * Note: (Doodman)
 *	This structure has field names that match the DB name exactly.
 *	Please take care as to not mess this up as it should make
 *	everyones life (i.e. mine) much easier. And the DB names
 *	match the field name from the 13th floor (SEQ) item collectors,
 *	so please maintain that as well.
 *
 * Note #2: (Doodman)
 *	UnkXXX fields are left in here for completeness but commented
 *	out since they are really unknown and since the items are now
 *	preserialized they should not be needed. Conversly if they
 *	-are- needed, then they shouldn't be unkown.
 *
 * Note #3: (Doodman)
 *	Please take care when adding new found data fields to add them
 *	to the appropriate structure. Item_Struct has elements that are
 *	global to all types of items only.
 *
 * Note #4: (Doodman)
 *	Made ya look! Ha!
 */

//#include "eq_constants.h"
#include "eq_dictionary.h"

/*
** Child struct of Item_Struct:
**	Effect data: Click, Proc, Focus, Worn, Scroll
**
*/
struct ItemEffect_Struct {
	int32	Effect;
	uint8	Type;
	uint8	Level;
	uint8	Level2;
	//MaxCharges
	//CastTime
	//RecastDelay
	//RecastType
	//ProcRate
};

class ItemInst;

struct InternalSerializedItem_Struct {
	int16 slot_id;
	const void * inst;
};

// use EmuConstants::ITEM_COMMON_SIZE
//#define MAX_AUGMENT_SLOTS 5

struct Item_Struct {
	bool IsEquipable(uint16 Race, uint16 Class) const;
	// Non packet based fields
	uint8	MinStatus;

	// Packet based fields
	uint8	ItemClass;		// Item Type: 0=common, 1=container, 2=book
	char	Name[64];		// Name
	char	Lore[80];		// Lore Name: *=lore, &=summoned, #=artifact, ~=pending lore
	char	IDFile[30];		// Visible model
	uint32	ID;				// Unique ID (also PK for DB)
	int32	Weight;			// Item weight * 10
	uint8	NoRent;			// No Rent: 0=norent, 255=not norent
	uint8	NoDrop;			// No Drop: 0=nodrop, 255=not nodrop
	uint8	Size;			// Size: 0=tiny, 1=small, 2=medium, 3=large, 4=giant
	uint32	Slots;			// Bitfield for which slots this item can be used in
	uint32	Price;			// Item cost (?)
	uint32	Icon;			// Icon Number
	uint32	LoreGroup;		// Later items use LoreGroup instead of LoreFlag. we might want to see about changing this to int32 since it is commonly -1 and is constantly being cast from signed (-1) to unsigned (4294967295)
	bool	LoreFlag;		// This will be true if LoreGroup is non-zero
	bool	PendingLoreFlag;
	bool	ArtifactFlag;
	bool	SummonedFlag;
	uint8	FVNoDrop;		// Firiona Vie nodrop flag
	uint32	Favor;			// Individual favor
	uint32	GuildFavor;		// Guild favor
	uint32	PointType;

	//uint32	Unk117;
	//uint32	Unk118;
	//uint32	Unk121;
	//uint32	Unk124;

	uint8	BagType;		// 0:Small Bag, 1:Large Bag, 2:Quiver, 3:Belt Pouch ... there are 50 types
	uint8	BagSlots;		// Number of slots: can only be 2, 4, 6, 8, or 10
	uint8	BagSize;		// 0:TINY, 1:SMALL, 2:MEDIUM, 3:LARGE, 4:GIANT
	uint8	BagWR;			// 0->100

	bool	BenefitFlag;
	bool	Tradeskills;	// Is this a tradeskill item?
	int8	CR;				// Save vs Cold
	int8	DR;				// Save vs Disease
	int8	PR;				// Save vs Poison
	int8	MR;				// Save vs Magic
	int8	FR;				// Save vs Fire
	int8	AStr;			// Strength
	int8	ASta;			// Stamina
	int8	AAgi;			// Agility
	int8	ADex;			// Dexterity
	int8	ACha;			// Charisma
	int8	AInt;			// Intelligence
	int8	AWis;			// Wisdom
	int32	HP;				// HP
	int32	Mana;			// Mana
	int32	AC;				// AC
	uint32	Deity;			// Bitmask of Deities that can equip this item
	//uint32	Unk033
	int32	SkillModValue;	// % Mod to skill specified in SkillModType
	int32	SkillModMax;	// Max skill point modification
	uint32	SkillModType;	// Type of skill for SkillModValue to apply to
	uint32	BaneDmgRace;	// Bane Damage Race
	int8	BaneDmgAmt;		// Bane Damage Body Amount
	uint32	BaneDmgBody;	// Bane Damage Body
	bool	Magic;			// True=Magic Item, False=not
	int32	CastTime_;
	uint8	ReqLevel;		// Required Level to use item
	uint32	BardType;		// Bard Skill Type
	int32	BardValue;		// Bard Skill Amount
	int8	Light;			// Light
	uint8	Delay;			// Delay * 10
	uint8	RecLevel;		// Recommended level to use item
	uint8	RecSkill;		// Recommended skill to use item (refers to primary skill of item)
	uint8	ElemDmgType;	// Elemental Damage Type (1=magic, 2=fire)
	uint8	ElemDmgAmt;		// Elemental Damage
	uint8	Range;			// Range of item
	uint32	Damage;			// Delay between item usage (in 0.1 sec increments)
	uint32	Color;			// RR GG BB 00 <-- as it appears in pc
	uint32	Classes;		// Bitfield of classes that can equip item (1 << class#)
	uint32	Races;			// Bitfield of races that can equip item (1 << race#)
	//uint32	Unk054;
	int16	MaxCharges;		// Maximum charges items can hold: -1 if not a chargeable item
	uint8	ItemType;		// Item Type/Skill (itemClass* from above)
	uint8	Material;		// Item material type
	uint32	HerosForgeModel;// Hero's Forge Armor Model Type (2-13?)
	float	SellRate;		// Sell rate
	//uint32	Unk059;
	union {
		uint32 Fulfilment;	// Food fulfilment (How long it lasts)
		uint32 CastTime;		// Cast Time for clicky effects, in milliseconds
	};
	uint32 EliteMaterial;
	int32	ProcRate;
	int8	CombatEffects;	// PoP: Combat Effects +
	int8	Shielding;		// PoP: Shielding %
	int8	StunResist;		// PoP: Stun Resist %
	int8	StrikeThrough;	// PoP: Strike Through %
	uint32	ExtraDmgSkill;
	uint32	ExtraDmgAmt;
	int8	SpellShield;	// PoP: Spell Shield %
	int8	Avoidance;		// PoP: Avoidance +
	int8	Accuracy;		// PoP: Accuracy +
	uint32	CharmFileID;
	int32	FactionMod1;	// Faction Mod 1
	int32	FactionMod2;	// Faction Mod 2
	int32	FactionMod3;	// Faction Mod 3
	int32	FactionMod4;	// Faction Mod 4
	int32	FactionAmt1;	// Faction Amt 1
	int32	FactionAmt2;	// Faction Amt 2
	int32	FactionAmt3;	// Faction Amt 3
	int32	FactionAmt4;	// Faction Amt 4
	char	CharmFile[32];	// ?
	uint32	AugType;
	uint8	AugSlotType[EQEmu::Constants::ITEM_COMMON_SIZE];	// RoF: Augment Slot 1-6 Type
	uint8	AugSlotVisible[EQEmu::Constants::ITEM_COMMON_SIZE];	// RoF: Augment Slot 1-6 Visible
	uint8	AugSlotUnk2[EQEmu::Constants::ITEM_COMMON_SIZE];	// RoF: Augment Slot 1-6 Unknown Most likely Powersource related
	uint32	LDoNTheme;
	uint32	LDoNPrice;
	uint32	LDoNSold;
	uint32	BaneDmgRaceAmt;
	uint32	AugRestrict;
	uint32	Endur;
	uint32	DotShielding;
	uint32	Attack;
	uint32	Regen;
	uint32	ManaRegen;
	uint32	EnduranceRegen;
	uint32	Haste;
	uint32	DamageShield;
	uint32	RecastDelay;
	uint32	RecastType;
	uint32	AugDistiller;
	bool	Attuneable;
	bool	NoPet;
	bool	PotionBelt;
	bool	Stackable;
	bool	NoTransfer;
	bool	QuestItemFlag;
	int16	StackSize;
	uint8	PotionBeltSlots;
	ItemEffect_Struct Click, Proc, Worn, Focus, Scroll, Bard;

	uint8	Book;			// 0=Not book, 1=Book
	uint32	BookType;
	char	Filename[33];	// Filename for book data
	// Begin SoF Fields
	int32 SVCorruption;
	uint32 Purity;
	uint8 EvolvingItem;
	uint32 EvolvingID;
	uint8 EvolvingLevel;
	uint8 EvolvingMax;
	uint32 BackstabDmg;
	uint32 DSMitigation;
	int32 HeroicStr;
	int32 HeroicInt;
	int32 HeroicWis;
	int32 HeroicAgi;
	int32 HeroicDex;
	int32 HeroicSta;
	int32 HeroicCha;
	int32 HeroicMR;
	int32 HeroicFR;
	int32 HeroicCR;
	int32 HeroicDR;
	int32 HeroicPR;
	int32 HeroicSVCorrup;
	int32 HealAmt;
	int32 SpellDmg;
	uint32 LDoNSellBackRate;
	uint32 ScriptFileID;
	uint16 ExpendableArrow;
	uint32 Clairvoyance;
	char	ClickName[65];
	char	ProcName[65];
	char	WornName[65];
	char	FocusName[65];
	char	ScrollName[65];

};

#endif
