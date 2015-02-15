/*
EQEMu:  Everquest Server Emulator

Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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
// class EmuConstants
//
uint16 EmuConstants::InventoryMapSize(int16 indexMap)
{
	switch (indexMap)
	{
	case MapPossessions:
		return MAP_POSSESSIONS_SIZE;
	case MapBank:
		return MAP_BANK_SIZE;
	case MapSharedBank:
		return MAP_SHARED_BANK_SIZE;
	case MapTrade:
		return MAP_TRADE_SIZE;
	case MapWorld:
		return MAP_WORLD_SIZE;
	case MapLimbo:
		return MAP_LIMBO_SIZE;
	case MapTribute:
		return MAP_TRIBUTE_SIZE;
	case MapTrophyTribute:
		return MAP_TROPHY_TRIBUTE_SIZE;
	case MapGuildTribute:
		return MAP_GUILD_TRIBUTE_SIZE;
	case MapMerchant:
		return MAP_MERCHANT_SIZE;
	case MapDeleted:
		return MAP_DELETED_SIZE;
	case MapCorpse:
		return MAP_CORPSE_SIZE;
	case MapBazaar:
		return MAP_BAZAAR_SIZE;
	case MapInspect:
		return MAP_INSPECT_SIZE;
	case MapRealEstate:
		return MAP_REAL_ESTATE_SIZE;
	case MapViewMODPC:
		return MAP_VIEW_MOD_PC_SIZE;
	case MapViewMODBank:
		return MAP_VIEW_MOD_BANK_SIZE;
	case MapViewMODSharedBank:
		return MAP_VIEW_MOD_SHARED_BANK_SIZE;
	case MapViewMODLimbo:
		return MAP_VIEW_MOD_LIMBO_SIZE;
	case MapAltStorage:
		return MAP_ALT_STORAGE_SIZE;
	case MapArchived:
		return MAP_ARCHIVED_SIZE;
	case MapMail:
		return MAP_MAIL_SIZE;
	case MapGuildTrophyTribute:
		return MAP_GUILD_TROPHY_TRIBUTE_SIZE;
	case MapKrono:
		return MAP_KRONO_SIZE;
	case MapOther:
		return MAP_OTHER_SIZE;
	default:
		return NOT_USED;
	}
}

/*
std::string EmuConstants::InventoryLocationName(Location_Struct location)
{
	// not ready for implementation...
	std::string ret_str;
	StringFormat(ret_str, "%s, %s, %s, %s", InventoryMapName(location.map), InventoryMainName(location.main), InventorySubName(location.sub), InventoryAugName(location.aug));
	return  ret_str;
}
*/

std::string EmuConstants::InventoryMapName(int16 indexMap)
{
	switch (indexMap)
	{
	case INVALID_INDEX:
		return "Invalid Map";
	case MapPossessions:
		return "Possessions";
	case MapBank:
		return "Bank";
	case MapSharedBank:
		return "SharedBank";
	case MapTrade:
		return "Trade";
	case MapWorld:
		return "World";
	case MapLimbo:
		return "Limbo";
	case MapTribute:
		return "Tribute";
	case MapTrophyTribute:
		return "TrophyTribute";
	case MapGuildTribute:
		return "GuildTribute";
	case MapMerchant:
		return "Merchant";
	case MapDeleted:
		return "Deleted";
	case MapCorpse:
		return "Corpse";
	case MapBazaar:
		return "Bazaar";
	case MapInspect:
		return "Inspect";
	case MapRealEstate:
		return "RealEstate";
	case MapViewMODPC:
		return "ViewMODPC";
	case MapViewMODBank:
		return "ViewMODBank";
	case MapViewMODSharedBank:
		return "ViewMODSharedBank";
	case MapViewMODLimbo:
		return "ViewMODLimbo";
	case MapAltStorage:
		return "AltStorage";
	case MapArchived:
		return "Archived";
	case MapMail:
		return "Mail";
	case MapGuildTrophyTribute:
		return "GuildTrophyTribute";
	case MapKrono:
		return "Krono";
	case MapOther:
		return "Other";
	default:
		return "Unknown Map";
	}
}

std::string EmuConstants::InventoryMainName(int16 indexMain)
{
	switch (indexMain)
	{
	case INVALID_INDEX:
		return "Invalid Main";
	case MainCharm:
		return "Charm";
	case MainEar1:
		return "Ear1";
	case MainHead:
		return "Head";
	case MainFace:
		return "Face";
	case MainEar2:
		return "Ear2";
	case MainNeck:
		return "Neck";
	case MainShoulders:
		return "Shoulders";
	case MainArms:
		return "Arms";
	case MainBack:
		return "Back";
	case MainWrist1:
		return "Wrist1";
	case MainWrist2:
		return "Wrist2";
	case MainRange:
		return "Range";
	case MainHands:
		return "Hands";
	case MainPrimary:
		return "Primary";
	case MainSecondary:
		return "Secondary";
	case MainFinger1:
		return "Finger1";
	case MainFinger2:
		return "Finger2";
	case MainChest:
		return "Chest";
	case MainLegs:
		return "Legs";
	case MainFeet:
		return "Feet";
	case MainWaist:
		return "Waist";
	case MainPowerSource:
		return "PowerSource";
	case MainAmmo:
		return "Ammo";
	case MainGeneral1:
		return "General1";
	case MainGeneral2:
		return "General2";
	case MainGeneral3:
		return "General3";
	case MainGeneral4:
		return "General4";
	case MainGeneral5:
		return "General5";
	case MainGeneral6:
		return "General6";
	case MainGeneral7:
		return "General7";
	case MainGeneral8:
		return "General8";
	/*
	case MainGeneral9:
		return "General9";
	case MainGeneral10:
		return "General10";
	*/
	case MainCursor:
		return "Cursor";
	default:
		return "Unknown Main";
	}
}

std::string EmuConstants::InventorySubName(int16 indexSub)
{
	if (indexSub == INVALID_INDEX)
		return "Invalid Sub";

	if ((uint16)indexSub >= ITEM_CONTAINER_SIZE)
		return "Unknown Sub";

	std::string ret_str;
	ret_str = StringFormat("Container%i", (indexSub + 1)); // zero-based index..but, count starts at one

	return ret_str;
}

std::string EmuConstants::InventoryAugName(int16 indexAug)
{
	if (indexAug == INVALID_INDEX)
		return "Invalid Aug";

	if ((uint16)indexAug >= ITEM_COMMON_SIZE)
		return "Unknown Aug";

	std::string ret_str;
	ret_str = StringFormat("Augment%i", (indexAug + 1)); // zero-based index..but, count starts at one

	return ret_str;
}


// 
// class EQLimits
//
// client validation
bool EQLimits::IsValidPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_PC_CLIENT)
		return true;

	return false;
}

ClientVersion EQLimits::ValidatePCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_PC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// npc validation
bool EQLimits::IsValidNPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > LAST_PC_CLIENT && clientVersion <= LAST_NPC_CLIENT)
		return true;

	return false;
}

ClientVersion EQLimits::ValidateNPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > LAST_PC_CLIENT && clientVersion <= LAST_NPC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// mob validation
bool EQLimits::IsValidMobClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_NPC_CLIENT)
		return true;

	return false;
}

ClientVersion EQLimits::ValidateMobClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_NPC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// database
size_t EQLimits::CharacterCreationLimit(ClientVersion clientVersion)
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
uint16 EQLimits::InventoryMapSize(int16 indexMap, ClientVersion clientVersion)
{
	// not all maps will have an instantiated container..some are references for queue generators (i.e., bazaar, mail, etc...)
	// a zero '0' indicates a needed value..otherwise, change to '_NOTUSED' for a null value so indices requiring research can be identified
	// ALL of these values need to be verified before pushing to live
	//
	// make sure that you transcribe the actual value from 'defaults' to here before updating or client crashes will ensue..and/or...
	// insert older clients inside of the progression of client order
	//
	// MAP_POSSESSIONS_SIZE does not reflect all actual <client>_constants size due to bitmask-use compatibility
	//
	// when setting NPC-based values, try to adhere to an EmuConstants::<property> or NOT_USED value to avoid unnecessary issues

	static const uint16 local[_MapCount][CLIENT_VERSION_COUNT] = {
		// server and database are sync'd to current MapPossessions's client as set in 'using namespace RoF::slots;' and
		// 'EmuConstants::MAP_POSSESSIONS_SIZE' - use/update EquipmentBitmask(), GeneralBitmask() and CursorBitmask()
		// for partial range validation checks and 'EmuConstants::MAP_POSSESSIONS_SIZE' for full range iterations
		{ // local[MainPossessions]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*Titanium*/	EmuConstants::MAP_POSSESSIONS_SIZE,
/*SoF*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*SoD*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*Underfoot*/	EmuConstants::MAP_POSSESSIONS_SIZE,
/*RoF*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*RoF2*/		EmuConstants::MAP_POSSESSIONS_SIZE,

/*NPC*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*Merc*/		EmuConstants::MAP_POSSESSIONS_SIZE,
/*Bot*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*Pet*/			EmuConstants::MAP_POSSESSIONS_SIZE
		},
		{ // local[MapBank]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::MAP_BANK_SIZE,
/*SoF*/			EmuConstants::MAP_BANK_SIZE,
/*SoD*/			EmuConstants::MAP_BANK_SIZE,
/*Underfoot*/	EmuConstants::MAP_BANK_SIZE,
/*RoF*/			EmuConstants::MAP_BANK_SIZE,
/*RoF2*/		EmuConstants::MAP_BANK_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[MapSharedBank]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_SHARED_BANK_SIZE,
/*Titanium*/	EmuConstants::MAP_SHARED_BANK_SIZE,
/*SoF*/			EmuConstants::MAP_SHARED_BANK_SIZE,
/*SoD*/			EmuConstants::MAP_SHARED_BANK_SIZE,
/*Underfoot*/	EmuConstants::MAP_SHARED_BANK_SIZE,
/*RoF*/			EmuConstants::MAP_SHARED_BANK_SIZE,
/*RoF2*/		EmuConstants::MAP_SHARED_BANK_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[MapTrade]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_TRADE_SIZE,
/*Titanium*/	EmuConstants::MAP_TRADE_SIZE,
/*SoF*/			EmuConstants::MAP_TRADE_SIZE,
/*SoD*/			EmuConstants::MAP_TRADE_SIZE,
/*Underfoot*/	EmuConstants::MAP_TRADE_SIZE,
/*RoF*/			EmuConstants::MAP_TRADE_SIZE,
/*RoF2*/		EmuConstants::MAP_TRADE_SIZE,

/*NPC*/			4,
/*Merc*/		4,
/*Bot*/			EmuConstants::MAP_TRADE_SIZE, // client thinks this is another client
/*Pet*/			4
		},
		{ // local[MapWorld]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_WORLD_SIZE,
/*Titanium*/	EmuConstants::MAP_WORLD_SIZE,
/*SoF*/			EmuConstants::MAP_WORLD_SIZE,
/*SoD*/			EmuConstants::MAP_WORLD_SIZE,
/*Underfoot*/	EmuConstants::MAP_WORLD_SIZE,
/*RoF*/			EmuConstants::MAP_WORLD_SIZE,
/*RoF2*/		EmuConstants::MAP_WORLD_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[MapLimbo]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_LIMBO_SIZE,
/*Titanium*/	EmuConstants::MAP_LIMBO_SIZE,
/*SoF*/			EmuConstants::MAP_LIMBO_SIZE,
/*SoD*/			EmuConstants::MAP_LIMBO_SIZE,
/*Underfoot*/	EmuConstants::MAP_LIMBO_SIZE,
/*RoF*/			EmuConstants::MAP_LIMBO_SIZE,
/*RoF2*/		EmuConstants::MAP_LIMBO_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[MapTribute]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_TRIBUTE_SIZE,
/*Titanium*/	EmuConstants::MAP_TRIBUTE_SIZE,
/*SoF*/			EmuConstants::MAP_TRIBUTE_SIZE,
/*SoD*/			EmuConstants::MAP_TRIBUTE_SIZE,
/*Underfoot*/	EmuConstants::MAP_TRIBUTE_SIZE,
/*RoF*/			EmuConstants::MAP_TRIBUTE_SIZE,
/*RoF2*/		EmuConstants::MAP_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapTrophyTribute]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_TROPHY_TRIBUTE_SIZE,
/*RoF2*/		EmuConstants::MAP_TROPHY_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapGuildTribute]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_GUILD_TRIBUTE_SIZE,
/*RoF2*/		EmuConstants::MAP_GUILD_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapMerchant]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_MERCHANT_SIZE,
/*RoF2*/		EmuConstants::MAP_MERCHANT_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapDeleted]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_DELETED_SIZE,
/*RoF2*/		EmuConstants::MAP_DELETED_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapCorpse]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::MAP_CORPSE_SIZE,
/*SoF*/			SoF::consts::MAP_CORPSE_SIZE,
/*SoD*/			SoD::consts::MAP_CORPSE_SIZE,
/*Underfoot*/	UF::consts::MAP_CORPSE_SIZE,
/*RoF*/			RoF::consts::MAP_CORPSE_SIZE,
/*RoF2*/		RoF2::consts::MAP_CORPSE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapBazaar]
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::MAP_BAZAAR_SIZE,
/*Titanium*/	EmuConstants::MAP_BAZAAR_SIZE,
/*SoF*/			EmuConstants::MAP_BAZAAR_SIZE,
/*SoD*/			EmuConstants::MAP_BAZAAR_SIZE,
/*Underfoot*/	EmuConstants::MAP_BAZAAR_SIZE,
/*RoF*/			EmuConstants::MAP_BAZAAR_SIZE,
/*RoF2*/		EmuConstants::MAP_BAZAAR_SIZE,

/*NPC*/			0, // this may need to be 'EmuConstants::MAP_BAZAAR_SIZE' if offline client traders respawn as an npc
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapInspect]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::MAP_INSPECT_SIZE,
/*SoF*/			SoF::consts::MAP_INSPECT_SIZE,
/*SoD*/			SoD::consts::MAP_INSPECT_SIZE,
/*Underfoot*/	UF::consts::MAP_INSPECT_SIZE,
/*RoF*/			RoF::consts::MAP_INSPECT_SIZE,
/*RoF2*/		RoF2::consts::MAP_INSPECT_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[MapRealEstate]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_REAL_ESTATE_SIZE,
/*RoF2*/		EmuConstants::MAP_REAL_ESTATE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapViewMODPC]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_VIEW_MOD_PC_SIZE,
/*RoF2*/		EmuConstants::MAP_VIEW_MOD_PC_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapViewMODBank]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_VIEW_MOD_BANK_SIZE,
/*RoF2*/		EmuConstants::MAP_VIEW_MOD_BANK_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapViewMODSharedBank]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_VIEW_MOD_SHARED_BANK_SIZE,
/*RoF2*/		EmuConstants::MAP_VIEW_MOD_SHARED_BANK_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapViewMODLimbo]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_VIEW_MOD_LIMBO_SIZE,
/*RoF2*/		EmuConstants::MAP_VIEW_MOD_LIMBO_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapAltStorage]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_ALT_STORAGE_SIZE,
/*RoF2*/		EmuConstants::MAP_ALT_STORAGE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapArchived]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_ARCHIVED_SIZE,
/*RoF2*/		EmuConstants::MAP_ARCHIVED_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapMail]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_MAIL_SIZE,
/*RoF2*/		EmuConstants::MAP_MAIL_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapGuildTrophyTribute]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_GUILD_TROPHY_TRIBUTE_SIZE,
/*RoF2*/		EmuConstants::MAP_GUILD_TROPHY_TRIBUTE_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapKrono]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	NOT_USED,
/*SoF*/			NOT_USED,
/*SoD*/			NOT_USED,
/*Underfoot*/	NOT_USED,
/*RoF*/			EmuConstants::MAP_KRONO_SIZE,
/*RoF2*/		EmuConstants::MAP_KRONO_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapOther]
/*Unknown*/		NOT_USED,
/*62*/			0,
/*Titanium*/	0,
/*SoF*/			0,
/*SoD*/			0,
/*Underfoot*/	0,
/*RoF*/			EmuConstants::MAP_OTHER_SIZE,
/*RoF2*/		EmuConstants::MAP_OTHER_SIZE,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		}
	};

	if ((uint16)indexMap < _MapCount)
		return local[indexMap][static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];

	return NOT_USED;
}

uint64 EQLimits::PossessionsBitmask(ClientVersion clientVersion)
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

uint64 EQLimits::EquipmentBitmask(ClientVersion clientVersion)
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

uint64 EQLimits::GeneralBitmask(ClientVersion clientVersion)
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

uint64 EQLimits::CursorBitmask(ClientVersion clientVersion)
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

bool EQLimits::AllowsEmptyBagInBag(ClientVersion clientVersion)
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

bool EQLimits::AllowsClickCastFromBag(ClientVersion clientVersion)
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
uint16 EQLimits::ItemCommonSize(ClientVersion clientVersion)
{
	static const uint16 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::ITEM_COMMON_SIZE,
/*Titanium*/	EmuConstants::ITEM_COMMON_SIZE,
/*SoF*/			EmuConstants::ITEM_COMMON_SIZE,
/*SoD*/			EmuConstants::ITEM_COMMON_SIZE,
/*Underfoot*/	EmuConstants::ITEM_COMMON_SIZE,
/*RoF*/			EmuConstants::ITEM_COMMON_SIZE,
/*RoF2*/		EmuConstants::ITEM_COMMON_SIZE,

/*NPC*/			EmuConstants::ITEM_COMMON_SIZE,
/*Merc*/		EmuConstants::ITEM_COMMON_SIZE,
/*Bot*/			EmuConstants::ITEM_COMMON_SIZE,
/*Pet*/			EmuConstants::ITEM_COMMON_SIZE
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

uint16 EQLimits::ItemContainerSize(ClientVersion clientVersion)
{
	static const uint16 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Titanium*/	EmuConstants::ITEM_CONTAINER_SIZE,
/*SoF*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*SoD*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Underfoot*/	EmuConstants::ITEM_CONTAINER_SIZE,
/*RoF*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*RoF2*/		EmuConstants::ITEM_CONTAINER_SIZE,

/*NPC*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Merc*/		EmuConstants::ITEM_CONTAINER_SIZE,
/*Bot*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Pet*/			EmuConstants::ITEM_CONTAINER_SIZE
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

bool EQLimits::CoinHasWeight(ClientVersion clientVersion)
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
