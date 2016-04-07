/*
EQEMu:  Everquest Server Emulator

Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "eq_dictionary.h"
#include "string_util.h"

//
// class EQEmu::Constants
//
uint16 EQEmu::Constants::InventoryTypeSize(int16 type_index)
{
	switch (type_index) {
	case TypePossessions:
		return TYPE_POSSESSIONS_SIZE;
	case TypeBank:
		return TYPE_BANK_SIZE;
	case TypeSharedBank:
		return TYPE_SHARED_BANK_SIZE;
	case TypeTrade:
		return TYPE_TRADE_SIZE;
	case TypeWorld:
		return TYPE_WORLD_SIZE;
	case TypeLimbo:
		return TYPE_LIMBO_SIZE;
	case TypeTribute:
		return TYPE_TRIBUTE_SIZE;
	case TypeTrophyTribute:
		return TYPE_TROPHY_TRIBUTE_SIZE;
	case TypeGuildTribute:
		return TYPE_GUILD_TRIBUTE_SIZE;
	case TypeMerchant:
		return TYPE_MERCHANT_SIZE;
	case TypeDeleted:
		return TYPE_DELETED_SIZE;
	case TypeCorpse:
		return TYPE_CORPSE_SIZE;
	case TypeBazaar:
		return TYPE_BAZAAR_SIZE;
	case TypeInspect:
		return TYPE_INSPECT_SIZE;
	case TypeRealEstate:
		return TYPE_REAL_ESTATE_SIZE;
	case TypeViewMODPC:
		return TYPE_VIEW_MOD_PC_SIZE;
	case TypeViewMODBank:
		return TYPE_VIEW_MOD_BANK_SIZE;
	case TypeViewMODSharedBank:
		return TYPE_VIEW_MOD_SHARED_BANK_SIZE;
	case TypeViewMODLimbo:
		return TYPE_VIEW_MOD_LIMBO_SIZE;
	case TypeAltStorage:
		return TYPE_ALT_STORAGE_SIZE;
	case TypeArchived:
		return TYPE_ARCHIVED_SIZE;
	case TypeMail:
		return TYPE_MAIL_SIZE;
	case TypeGuildTrophyTribute:
		return TYPE_GUILD_TROPHY_TRIBUTE_SIZE;
	case TypeKrono:
		return TYPE_KRONO_SIZE;
	case TypeOther:
		return TYPE_OTHER_SIZE;
	default:
		return NOT_USED;
	}
}

/*
const char* EQEmu::Constants::InventoryLocationName(Location_Struct location)
{
	// not ready for implementation...
	std::string ret_str;
	StringFormat(ret_str, "%s, %s, %s, %s", InventoryMapName(location.map), InventoryMainName(location.main), InventorySubName(location.sub), InventoryAugName(location.aug));
	return  ret_str;
}
*/

const char* EQEmu::Constants::InventoryTypeName(int16 type_index)
{
	switch (type_index) {
	case INVALID_INDEX:
		return "Invalid Type";
	case TypePossessions:
		return "Possessions";
	case TypeBank:
		return "Bank";
	case TypeSharedBank:
		return "SharedBank";
	case TypeTrade:
		return "Trade";
	case TypeWorld:
		return "World";
	case TypeLimbo:
		return "Limbo";
	case TypeTribute:
		return "Tribute";
	case TypeTrophyTribute:
		return "TrophyTribute";
	case TypeGuildTribute:
		return "GuildTribute";
	case TypeMerchant:
		return "Merchant";
	case TypeDeleted:
		return "Deleted";
	case TypeCorpse:
		return "Corpse";
	case TypeBazaar:
		return "Bazaar";
	case TypeInspect:
		return "Inspect";
	case TypeRealEstate:
		return "RealEstate";
	case TypeViewMODPC:
		return "ViewMODPC";
	case TypeViewMODBank:
		return "ViewMODBank";
	case TypeViewMODSharedBank:
		return "ViewMODSharedBank";
	case TypeViewMODLimbo:
		return "ViewMODLimbo";
	case TypeAltStorage:
		return "AltStorage";
	case TypeArchived:
		return "Archived";
	case TypeMail:
		return "Mail";
	case TypeGuildTrophyTribute:
		return "GuildTrophyTribute";
	case TypeKrono:
		return "Krono";
	case TypeOther:
		return "Other";
	default:
		return "Unknown Type";
	}
}

const char* EQEmu::Constants::InventorySlotName(int16 slot_index)
{
	switch (slot_index) {
	case INVALID_INDEX:
		return "Invalid Slot";
	case SlotCharm:
		return "Charm";
	case SlotEar1:
		return "Ear1";
	case SlotHead:
		return "Head";
	case SlotFace:
		return "Face";
	case SlotEar2:
		return "Ear2";
	case SlotNeck:
		return "Neck";
	case SlotShoulders:
		return "Shoulders";
	case SlotArms:
		return "Arms";
	case SlotBack:
		return "Back";
	case SlotWrist1:
		return "Wrist1";
	case SlotWrist2:
		return "Wrist2";
	case SlotRange:
		return "Range";
	case SlotHands:
		return "Hands";
	case SlotPrimary:
		return "Primary";
	case SlotSecondary:
		return "Secondary";
	case SlotFinger1:
		return "Finger1";
	case SlotFinger2:
		return "Finger2";
	case SlotChest:
		return "Chest";
	case SlotLegs:
		return "Legs";
	case SlotFeet:
		return "Feet";
	case SlotWaist:
		return "Waist";
	case SlotPowerSource:
		return "PowerSource";
	case SlotAmmo:
		return "Ammo";
	case SlotGeneral1:
		return "General1";
	case SlotGeneral2:
		return "General2";
	case SlotGeneral3:
		return "General3";
	case SlotGeneral4:
		return "General4";
	case SlotGeneral5:
		return "General5";
	case SlotGeneral6:
		return "General6";
	case SlotGeneral7:
		return "General7";
	case SlotGeneral8:
		return "General8";
	/*
	case SlotGeneral9:
		return "General9";
	case SlotGeneral10:
		return "General10";
	*/
	case SlotCursor:
		return "Cursor";
	default:
		return "Unknown Slot";
	}
}

const char* EQEmu::Constants::InventorySubName(int16 sub_index)
{
	if (sub_index == INVALID_INDEX)
		return "Invalid Sub";

	if ((uint16)sub_index >= ITEM_CONTAINER_SIZE)
		return "Unknown Sub";

	static std::string ret_str;
	ret_str = StringFormat("Container%i", (sub_index + 1)); // zero-based index..but, count starts at one

	return ret_str.c_str();
}

const char* EQEmu::Constants::InventoryAugName(int16 aug_index)
{
	if (aug_index == INVALID_INDEX)
		return "Invalid Aug";

	if ((uint16)aug_index >= ITEM_COMMON_SIZE)
		return "Unknown Aug";

	static std::string ret_str;
	ret_str = StringFormat("Augment%i", (aug_index + 1)); // zero-based index..but, count starts at one

	return ret_str.c_str();
}


// 
// class EQEmu::Limits
//
// client validation
bool EQEmu::Limits::IsValidPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_PC_CLIENT)
		return true;

	return false;
}

ClientVersion EQEmu::Limits::ValidatePCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_PC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// npc validation
bool EQEmu::Limits::IsValidNPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > LAST_PC_CLIENT && clientVersion <= LAST_NPC_CLIENT)
		return true;

	return false;
}

ClientVersion EQEmu::Limits::ValidateNPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > LAST_PC_CLIENT && clientVersion <= LAST_NPC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// mob validation
bool EQEmu::Limits::IsValidMobClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_NPC_CLIENT)
		return true;

	return false;
}

ClientVersion EQEmu::Limits::ValidateMobClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_NPC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// database
size_t EQEmu::Limits::CharacterCreationLimit(ClientVersion clientVersion)
{
	static const size_t local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*Client62*/	NOT_USED,
/*Titanium*/	Titanium::consts::CHARACTER_CREATION_LIMIT,
/*SoF*/			SoF::consts::CHARACTER_CREATION_LIMIT,
/*SoD*/			SoD::consts::CHARACTER_CREATION_LIMIT,
/*UF*/			UF::consts::CHARACTER_CREATION_LIMIT,
/*RoF*/			RoF::consts::CHARACTER_CREATION_LIMIT,
/*RoF2*/		RoF2::consts::CHARACTER_CREATION_LIMIT,

/*MobNPC*/		NOT_USED,
/*MobMerc*/		NOT_USED,
/*MobBot*/		NOT_USED,
/*MobPet*/		NOT_USED
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

// inventory
uint16 EQEmu::Limits::InventoryMapSize(int16 indexMap, ClientVersion clientVersion)
{
	// not all maps will have an instantiated container..some are references for queue generators (i.e., bazaar, mail, etc...)
	// a zero '0' indicates a needed value..otherwise, change to '_NOTUSED' for a null value so indices requiring research can be identified
	// ALL of these values need to be verified before pushing to live
	//
	// make sure that you transcribe the actual value from 'defaults' to here before updating or client crashes will ensue..and/or...
	// insert older clients inside of the progression of client order
	//
	// TYPE_POSSESSIONS_SIZE does not reflect all actual <client>_constants size due to bitmask-use compatibility
	//
	// when setting NPC-based values, try to adhere to an Constants::<property> or NOT_USED value to avoid unnecessary issues

	static const uint16 local[TypeCount][CLIENT_VERSION_COUNT] = {
		// server and database are sync'd to current TypePossessions's client as set in 'using namespace RoF::slots;' and
		// 'Constants::TYPE_POSSESSIONS_SIZE' - use/update EquipmentBitmask(), GeneralBitmask() and CursorBitmask()
		// for partial range validation checks and 'Constants::TYPE_POSSESSIONS_SIZE' for full range iterations
		{ // local[TypePossessions]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_POSSESSIONS_SIZE,
/*Titanium*/	Constants::TYPE_POSSESSIONS_SIZE,
/*SoF*/			Constants::TYPE_POSSESSIONS_SIZE,
/*SoD*/			Constants::TYPE_POSSESSIONS_SIZE,
/*Underfoot*/	Constants::TYPE_POSSESSIONS_SIZE,
/*RoF*/			Constants::TYPE_POSSESSIONS_SIZE,
/*RoF2*/		Constants::TYPE_POSSESSIONS_SIZE,

/*NPC*/			Constants::TYPE_POSSESSIONS_SIZE,
/*Merc*/		Constants::TYPE_POSSESSIONS_SIZE,
/*Bot*/			Constants::TYPE_POSSESSIONS_SIZE,
/*Pet*/			Constants::TYPE_POSSESSIONS_SIZE
		},
		{ // local[TypeBank]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::TYPE_BANK_SIZE,
/*SoF*/			Constants::TYPE_BANK_SIZE,
/*SoD*/			Constants::TYPE_BANK_SIZE,
/*Underfoot*/	Constants::TYPE_BANK_SIZE,
/*RoF*/			Constants::TYPE_BANK_SIZE,
/*RoF2*/		Constants::TYPE_BANK_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeSharedBank]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_SHARED_BANK_SIZE,
/*Titanium*/	Constants::TYPE_SHARED_BANK_SIZE,
/*SoF*/			Constants::TYPE_SHARED_BANK_SIZE,
/*SoD*/			Constants::TYPE_SHARED_BANK_SIZE,
/*Underfoot*/	Constants::TYPE_SHARED_BANK_SIZE,
/*RoF*/			Constants::TYPE_SHARED_BANK_SIZE,
/*RoF2*/		Constants::TYPE_SHARED_BANK_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeTrade]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_TRADE_SIZE,
/*Titanium*/	Constants::TYPE_TRADE_SIZE,
/*SoF*/			Constants::TYPE_TRADE_SIZE,
/*SoD*/			Constants::TYPE_TRADE_SIZE,
/*Underfoot*/	Constants::TYPE_TRADE_SIZE,
/*RoF*/			Constants::TYPE_TRADE_SIZE,
/*RoF2*/		Constants::TYPE_TRADE_SIZE,

/*NPC*/			4,
/*Merc*/		4,
/*Bot*/			Constants::TYPE_TRADE_SIZE, // client thinks this is another client
/*Pet*/			4
		},
		{ // local[TypeWorld]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_WORLD_SIZE,
/*Titanium*/	Constants::TYPE_WORLD_SIZE,
/*SoF*/			Constants::TYPE_WORLD_SIZE,
/*SoD*/			Constants::TYPE_WORLD_SIZE,
/*Underfoot*/	Constants::TYPE_WORLD_SIZE,
/*RoF*/			Constants::TYPE_WORLD_SIZE,
/*RoF2*/		Constants::TYPE_WORLD_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeLimbo]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_LIMBO_SIZE,
/*Titanium*/	Constants::TYPE_LIMBO_SIZE,
/*SoF*/			Constants::TYPE_LIMBO_SIZE,
/*SoD*/			Constants::TYPE_LIMBO_SIZE,
/*Underfoot*/	Constants::TYPE_LIMBO_SIZE,
/*RoF*/			Constants::TYPE_LIMBO_SIZE,
/*RoF2*/		Constants::TYPE_LIMBO_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeTribute]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_TRIBUTE_SIZE,
/*Titanium*/	Constants::TYPE_TRIBUTE_SIZE,
/*SoF*/			Constants::TYPE_TRIBUTE_SIZE,
/*SoD*/			Constants::TYPE_TRIBUTE_SIZE,
/*Underfoot*/	Constants::TYPE_TRIBUTE_SIZE,
/*RoF*/			Constants::TYPE_TRIBUTE_SIZE,
/*RoF2*/		Constants::TYPE_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeTrophyTribute]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_TROPHY_TRIBUTE_SIZE,
/*RoF2*/		Constants::TYPE_TROPHY_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeGuildTribute]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_GUILD_TRIBUTE_SIZE,
/*RoF2*/		Constants::TYPE_GUILD_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeMerchant]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_MERCHANT_SIZE,
/*RoF2*/		Constants::TYPE_MERCHANT_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeDeleted]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_DELETED_SIZE,
/*RoF2*/		Constants::TYPE_DELETED_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeCorpse]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::TYPE_CORPSE_SIZE,
/*SoF*/			SoF::consts::TYPE_CORPSE_SIZE,
/*SoD*/			SoD::consts::TYPE_CORPSE_SIZE,
/*Underfoot*/	UF::consts::TYPE_CORPSE_SIZE,
/*RoF*/			RoF::consts::TYPE_CORPSE_SIZE,
/*RoF2*/		RoF2::consts::TYPE_CORPSE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeBazaar]
/*Unknown*/		NOT_USED,
/*62*/			Constants::TYPE_BAZAAR_SIZE,
/*Titanium*/	Constants::TYPE_BAZAAR_SIZE,
/*SoF*/			Constants::TYPE_BAZAAR_SIZE,
/*SoD*/			Constants::TYPE_BAZAAR_SIZE,
/*Underfoot*/	Constants::TYPE_BAZAAR_SIZE,
/*RoF*/			Constants::TYPE_BAZAAR_SIZE,
/*RoF2*/		Constants::TYPE_BAZAAR_SIZE,

/*NPC*/			0, // this may need to be 'Constants::TYPE_BAZAAR_SIZE' if offline client traders respawn as an npc
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeInspect]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::TYPE_INSPECT_SIZE,
/*SoF*/			SoF::consts::TYPE_INSPECT_SIZE,
/*SoD*/			SoD::consts::TYPE_INSPECT_SIZE,
/*Underfoot*/	UF::consts::TYPE_INSPECT_SIZE,
/*RoF*/			RoF::consts::TYPE_INSPECT_SIZE,
/*RoF2*/		RoF2::consts::TYPE_INSPECT_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeRealEstate]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_REAL_ESTATE_SIZE,
/*RoF2*/		Constants::TYPE_REAL_ESTATE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeViewMODPC]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_VIEW_MOD_PC_SIZE,
/*RoF2*/		Constants::TYPE_VIEW_MOD_PC_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeViewMODBank]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_VIEW_MOD_BANK_SIZE,
/*RoF2*/		Constants::TYPE_VIEW_MOD_BANK_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeViewMODSharedBank]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_VIEW_MOD_SHARED_BANK_SIZE,
/*RoF2*/		Constants::TYPE_VIEW_MOD_SHARED_BANK_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeViewMODLimbo]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_VIEW_MOD_LIMBO_SIZE,
/*RoF2*/		Constants::TYPE_VIEW_MOD_LIMBO_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeAltStorage]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_ALT_STORAGE_SIZE,
/*RoF2*/		Constants::TYPE_ALT_STORAGE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeArchived]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_ARCHIVED_SIZE,
/*RoF2*/		Constants::TYPE_ARCHIVED_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeMail]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_MAIL_SIZE,
/*RoF2*/		Constants::TYPE_MAIL_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeGuildTrophyTribute]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_GUILD_TROPHY_TRIBUTE_SIZE,
/*RoF2*/		Constants::TYPE_GUILD_TROPHY_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeKrono]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	NOT_USED,
/*SoF*/			NOT_USED,
/*SoD*/			NOT_USED,
/*Underfoot*/	NOT_USED,
/*RoF*/			Constants::TYPE_KRONO_SIZE,
/*RoF2*/		Constants::TYPE_KRONO_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[TypeOther]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			Constants::TYPE_OTHER_SIZE,
/*RoF2*/		Constants::TYPE_OTHER_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		}
	};

	if ((uint16)indexMap < TypeCount)
		return local[indexMap][static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];

	return NOT_USED;
}

uint64 EQEmu::Limits::PossessionsBitmask(ClientVersion clientVersion)
{
	// these are for the new inventory system (RoF)..not the current (Ti) one...
	// 0x0000000000200000 is SlotPowerSource (SoF+)
	// 0x0000000080000000 is SlotGeneral9 (RoF+)
	// 0x0000000100000000 is SlotGeneral10 (RoF+)

	static const uint64 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			0x000000027FDFFFFF,
/*Titanium*/	0x000000027FDFFFFF,
/*SoF*/			0x000000027FFFFFFF,
/*SoD*/			0x000000027FFFFFFF,
/*Underfoot*/	0x000000027FFFFFFF,
/*RoF*/			0x00000003FFFFFFFF,
/*RoF2*/		0,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
	};

	return NOT_USED;
	//return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

uint64 EQEmu::Limits::EquipmentBitmask(ClientVersion clientVersion)
{
	static const uint64 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			0x00000000005FFFFF,
/*Titanium*/	0x00000000005FFFFF,
/*SoF*/			0x00000000007FFFFF,
/*SoD*/			0x00000000007FFFFF,
/*Underfoot*/	0x00000000007FFFFF,
/*RoF*/			0x00000000007FFFFF,
/*RoF2*/		0,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
	};

	return NOT_USED;
	//return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

uint64 EQEmu::Limits::GeneralBitmask(ClientVersion clientVersion)
{
	static const uint64 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			0x000000007F800000,
/*Titanium*/	0x000000007F800000,
/*SoF*/			0x000000007F800000,
/*SoD*/			0x000000007F800000,
/*Underfoot*/	0x000000007F800000,
/*RoF*/			0x00000001FF800000,
/*RoF2*/		0,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
	};

	return NOT_USED;
	//return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

uint64 EQEmu::Limits::CursorBitmask(ClientVersion clientVersion)
{
	static const uint64 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			0x0000000200000000,
/*Titanium*/	0x0000000200000000,
/*SoF*/			0x0000000200000000,
/*SoD*/			0x0000000200000000,
/*Underfoot*/	0x0000000200000000,
/*RoF*/			0x0000000200000000,
/*RoF2*/		0,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
	};

	return NOT_USED;
	//return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

bool EQEmu::Limits::AllowsEmptyBagInBag(ClientVersion clientVersion)
{
	static const bool local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		false,
/*62*/			false,
/*Titanium*/	Titanium::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*SoF*/			SoF::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*SoD*/			SoD::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*Underfoot*/	UF::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*RoF*/			RoF::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*RoF2*/		RoF2::limits::ALLOWS_EMPTY_BAG_IN_BAG,

/*NPC*/			false,
/*Merc*/		false,
/*Bot*/			false,
/*Pet*/			false
	};

	return false; // not implemented
	//return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

bool EQEmu::Limits::AllowsClickCastFromBag(ClientVersion clientVersion)
{
	static const bool local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		false,
/*62*/			false,
/*Titanium*/	Titanium::limits::ALLOWS_CLICK_CAST_FROM_BAG,
/*SoF*/			SoF::limits::ALLOWS_CLICK_CAST_FROM_BAG,
/*SoD*/			SoD::limits::ALLOWS_CLICK_CAST_FROM_BAG,
/*Underfoot*/	UF::limits::ALLOWS_CLICK_CAST_FROM_BAG,
/*RoF*/			RoF::limits::ALLOWS_CLICK_CAST_FROM_BAG,
/*RoF2*/		RoF2::limits::ALLOWS_CLICK_CAST_FROM_BAG,

/*NPC*/			false,
/*Merc*/		false,
/*Bot*/			false,
/*Pet*/			false
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

// items
uint16 EQEmu::Limits::ItemCommonSize(ClientVersion clientVersion)
{
	static const uint16 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			Constants::ITEM_COMMON_SIZE,
/*Titanium*/	Constants::ITEM_COMMON_SIZE,
/*SoF*/			Constants::ITEM_COMMON_SIZE,
/*SoD*/			Constants::ITEM_COMMON_SIZE,
/*Underfoot*/	Constants::ITEM_COMMON_SIZE,
/*RoF*/			Constants::ITEM_COMMON_SIZE,
/*RoF2*/		Constants::ITEM_COMMON_SIZE,

/*NPC*/			Constants::ITEM_COMMON_SIZE,
/*Merc*/		Constants::ITEM_COMMON_SIZE,
/*Bot*/			Constants::ITEM_COMMON_SIZE,
/*Pet*/			Constants::ITEM_COMMON_SIZE
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

uint16 EQEmu::Limits::ItemContainerSize(ClientVersion clientVersion)
{
	static const uint16 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			Constants::ITEM_CONTAINER_SIZE,
/*Titanium*/	Constants::ITEM_CONTAINER_SIZE,
/*SoF*/			Constants::ITEM_CONTAINER_SIZE,
/*SoD*/			Constants::ITEM_CONTAINER_SIZE,
/*Underfoot*/	Constants::ITEM_CONTAINER_SIZE,
/*RoF*/			Constants::ITEM_CONTAINER_SIZE,
/*RoF2*/		Constants::ITEM_CONTAINER_SIZE,

/*NPC*/			Constants::ITEM_CONTAINER_SIZE,
/*Merc*/		Constants::ITEM_CONTAINER_SIZE,
/*Bot*/			Constants::ITEM_CONTAINER_SIZE,
/*Pet*/			Constants::ITEM_CONTAINER_SIZE
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

bool EQEmu::Limits::CoinHasWeight(ClientVersion clientVersion)
{
	static const bool local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		true,
/*62*/			true,
/*Titanium*/	Titanium::limits::COIN_HAS_WEIGHT,
/*SoF*/			SoF::limits::COIN_HAS_WEIGHT,
/*SoD*/			SoD::limits::COIN_HAS_WEIGHT,
/*Underfoot*/	UF::limits::COIN_HAS_WEIGHT,
/*RoF*/			RoF::limits::COIN_HAS_WEIGHT,
/*RoF2*/		RoF::limits::COIN_HAS_WEIGHT,

/*NPC*/			true,
/*Merc*/		true,
/*Bot*/			true,
/*Pet*/			true
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}
