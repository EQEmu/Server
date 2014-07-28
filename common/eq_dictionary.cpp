/*
EQEMu:  Everquest Server Emulator

Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

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
#include "StringUtil.h"

//
// class ServerConstants
//
uint16 EmuConstants::InventoryMapSize(int16 map) {
	switch (map) {
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
std::string EmuConstants::InventoryLocationName(Location_Struct location) {
	// not ready for implementation...
	std::string ret_str;
	StringFormat(ret_str, "%s, %s, %s, %s", InventoryMapName(location.map), InventoryMainName(location.main), InventorySubName(location.sub), InventoryAugName(location.aug));
	return  ret_str;
}
*/

std::string EmuConstants::InventoryMapName(int16 map) {
	switch (map) {
	case INVALID_INDEX:
		return "Invalid Map";
	case MapPossessions:
		return "Possessions";
	case MapBank:
		return "Bank";
	case MapSharedBank:
		return "Shared Bank";
	case MapTrade:
		return "Trade";
	case MapWorld:
		return "World";
	case MapLimbo:
		return "Limbo";
	case MapTribute:
		return "Tribute";
	case MapTrophyTribute:
		return "Trophy Tribute";
	case MapGuildTribute:
		return "Guild Tribute";
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
		return "Real Estate";
	case MapViewMODPC:
		return "View MOD PC";
	case MapViewMODBank:
		return "View MOD Bank";
	case MapViewMODSharedBank:
		return "View MOD Shared Bank";
	case MapViewMODLimbo:
		return "View MOD Limbo";
	case MapAltStorage:
		return "Alt Storage";
	case MapArchived:
		return "Archived";
	case MapMail:
		return "Mail";
	case MapGuildTrophyTribute:
		return "Guild Trophy Tribute";
	case MapKrono:
		return "Krono";
	case MapOther:
		return "Other";
	default:
		return "Unknown Map";
	}
}

std::string EmuConstants::InventoryMainName(int16 main) {
	switch (main) {
	case INVALID_INDEX:
		return "Invalid Main";
	case MainCharm:
		return "Charm";
	case MainEar1:
		return "Ear 1";
	case MainHead:
		return "Head";
	case MainFace:
		return "Face";
	case MainEar2:
		return "Ear 2";
	case MainNeck:
		return "Neck";
	case MainShoulders:
		return "Shoulders";
	case MainArms:
		return "Arms";
	case MainBack:
		return "Back";
	case MainWrist1:
		return "Wrist 1";
	case MainWrist2:
		return "Wrist 2";
	case MainRange:
		return "Range";
	case MainHands:
		return "Hands";
	case MainPrimary:
		return "Primary";
	case MainSecondary:
		return "Secondary";
	case MainFinger1:
		return "Finger 1";
	case MainFinger2:
		return "Finger 2";
	case MainChest:
		return "Chest";
	case MainLegs:
		return "Legs";
	case MainFeet:
		return "Feet";
	case MainWaist:
		return "Waist";
	case MainPowerSource:
		return "Power Source";
	case MainAmmo:
		return "Ammo";
	case MainGeneral1:
		return "General 1";
	case MainGeneral2:
		return "General 2";
	case MainGeneral3:
		return "General 3";
	case MainGeneral4:
		return "General 4";
	case MainGeneral5:
		return "General 5";
	case MainGeneral6:
		return "General 6";
	case MainGeneral7:
		return "General 7";
	case MainGeneral8:
		return "General 8";
	/*
	case MainGeneral9:
		return "General 9";
	case MainGeneral10:
		return "General 10";
	*/
	case MainCursor:
		return "Cursor";
	default:
		return "Unknown Main";
	}
}

std::string EmuConstants::InventorySubName(int16 sub) {
	if (sub == INVALID_INDEX)
		return "Invalid Sub";

	if ((uint16)sub >= ITEM_CONTAINER_SIZE)
		return "Unknown Sub";

	std::string ret_str;
	StringFormat(ret_str, "Container %i", (sub + 1)); // zero-based index..but, count starts at one

	return ret_str;
}

std::string EmuConstants::InventoryAugName(int16 aug) {
	if (aug == INVALID_INDEX)
		return "Invalid Aug";

	if ((uint16)aug >= ITEM_COMMON_SIZE)
		return "Unknown Aug";

	std::string ret_str;
	StringFormat(ret_str, "Augment %i", (aug + 1)); // zero-based index..but, count starts at one

	return ret_str;
}

// 
// class ClientLimits
//
// client validation
bool EQLimits::IsValidClientVersion(uint32 version) {
	if (version < _EQClientCount)
		return true;

	return false;
}

uint32 EQLimits::ValidateClientVersion(uint32 version) {
	if (version < _EQClientCount)
		return version;

	return EQClientUnknown;
}

EQClientVersion EQLimits::ValidateClientVersion(EQClientVersion version) {
	if (version >= EQClientUnknown)
		if (version < _EQClientCount)
			return version;

	return EQClientUnknown;
}

// npc validation
bool EQLimits::IsValidNPCVersion(uint32 version) {
	if (version >= _EQClientCount)
		if (version < _EmuClientCount)
			return true;

	return false;
}

uint32 EQLimits::ValidateNPCVersion(uint32 version) {
	if (version >= _EQClientCount)
		if (version < _EmuClientCount)
			return version;

	return EQClientUnknown;
}

EQClientVersion EQLimits::ValidateNPCVersion(EQClientVersion version) {
	if (version >= _EQClientCount)
		if (version < _EmuClientCount)
			return version;

	return EQClientUnknown;
}

// mob validation
bool EQLimits::IsValidMobVersion(uint32 version) {
	if (version < _EmuClientCount)
		return true;

	return false;
}

uint32 EQLimits::ValidateMobVersion(uint32 version) {
	if (version < _EmuClientCount)
		return version;

	return EQClientUnknown;
}

EQClientVersion EQLimits::ValidateMobVersion(EQClientVersion version) {
	if (version >= EQClientUnknown)
		if (version < _EmuClientCount)
			return version;

	return EQClientUnknown;
}

// inventory
uint16 EQLimits::InventoryMapSize(int16 map, uint32 version) {
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

	static const uint16 local[_MapCount][_EmuClientCount] = {
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
/*RoF2*/		0,

/*NPC*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*Merc*/		EmuConstants::MAP_POSSESSIONS_SIZE,
/*Bot*/			EmuConstants::MAP_POSSESSIONS_SIZE,
/*Pet*/			EmuConstants::MAP_POSSESSIONS_SIZE
		},
		{ // local[MapBank]
/*Unknown*/		NOT_USED,
/*62*/			Client62::consts::MAP_BANK_SIZE,
/*Titanium*/	Titanium::consts::MAP_BANK_SIZE,
/*SoF*/			EmuConstants::MAP_BANK_SIZE,
/*SoD*/			EmuConstants::MAP_BANK_SIZE,
/*Underfoot*/	EmuConstants::MAP_BANK_SIZE,
/*RoF*/			EmuConstants::MAP_BANK_SIZE,
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapCorpse]
/*Unknown*/		NOT_USED,
/*62*/			Client62::consts::MAP_CORPSE_SIZE,
/*Titanium*/	Titanium::consts::MAP_CORPSE_SIZE,
/*SoF*/			SoF::consts::MAP_CORPSE_SIZE,
/*SoD*/			SoD::consts::MAP_CORPSE_SIZE,
/*Underfoot*/	Underfoot::consts::MAP_CORPSE_SIZE,
/*RoF*/			RoF::consts::MAP_CORPSE_SIZE,
/*RoF2*/		0,

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
/*RoF2*/		0,

/*NPC*/			0, // this may need to be 'EmuConstants::MAP_BAZAAR_SIZE' if offline client traders respawn as an npc
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		},
		{ // local[MapInspect]
/*Unknown*/		NOT_USED,
/*62*/			Client62::consts::MAP_INSPECT_SIZE,
/*Titanium*/	Titanium::consts::MAP_INSPECT_SIZE,
/*SoF*/			SoF::consts::MAP_INSPECT_SIZE,
/*SoD*/			SoD::consts::MAP_INSPECT_SIZE,
/*Underfoot*/	Underfoot::consts::MAP_INSPECT_SIZE,
/*RoF*/			RoF::consts::MAP_INSPECT_SIZE,
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

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
/*RoF2*/		0,

/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
		}
	};

	if ((uint16)map < _MapCount)
		return local[map][ValidateMobVersion(version)];

	return NOT_USED;
}

uint64 EQLimits::PossessionsBitmask(uint32 version) {
	// these are for the new inventory system (RoF)..not the current (Ti) one...
	// 0x0000000000200000 is SlotPowerSource (SoF+)
	// 0x0000000080000000 is SlotGeneral9 (RoF+)
	// 0x0000000100000000 is SlotGeneral10 (RoF+)

	static const uint64 local[_EmuClientCount] = {
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
	//return local[ValidateMobVersion(version)];
}

uint64 EQLimits::EquipmentBitmask(uint32 version) {
	static const uint64 local[_EmuClientCount] = {
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
	//return local[ValidateMobVersion(version)];
}

uint64 EQLimits::GeneralBitmask(uint32 version) {
	static const uint64 local[_EmuClientCount] = {
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
	//return local[ValidateMobVersion(version)];
}

uint64 EQLimits::CursorBitmask(uint32 version) {
	static const uint64 local[_EmuClientCount] = {
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
	//return local[ValidateMobVersion(version)];
}

bool EQLimits::AllowsEmptyBagInBag(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		false,
/*62*/			Client62::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*Titanium*/	Titanium::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*SoF*/			SoF::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*SoD*/			SoD::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*Underfoot*/	Underfoot::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*RoF*/			RoF::limits::ALLOWS_EMPTY_BAG_IN_BAG,
/*RoF2*/		false,
		
/*NPC*/			false,
/*Merc*/		false,
/*Bot*/			false,
/*Pet*/			false
	};
	
	return false; // not implemented
	//return local[ValidateMobVersion(version)];
}

// items
uint16 EQLimits::ItemCommonSize(uint32 version) {
	static const uint16 local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::ITEM_COMMON_SIZE,
/*Titanium*/	EmuConstants::ITEM_COMMON_SIZE,
/*SoF*/			EmuConstants::ITEM_COMMON_SIZE,
/*SoD*/			EmuConstants::ITEM_COMMON_SIZE,
/*Underfoot*/	EmuConstants::ITEM_COMMON_SIZE,
/*RoF*/			EmuConstants::ITEM_COMMON_SIZE,
/*RoF2*/		0,
		
/*NPC*/			EmuConstants::ITEM_COMMON_SIZE,
/*Merc*/		EmuConstants::ITEM_COMMON_SIZE,
/*Bot*/			EmuConstants::ITEM_COMMON_SIZE,
/*Pet*/			EmuConstants::ITEM_COMMON_SIZE
	};

	return local[ValidateMobVersion(version)];
}

uint16 EQLimits::ItemContainerSize(uint32 version) {
	static const uint16 local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Titanium*/	EmuConstants::ITEM_CONTAINER_SIZE,
/*SoF*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*SoD*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Underfoot*/	EmuConstants::ITEM_CONTAINER_SIZE,
/*RoF*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*RoF2*/		0,
		
/*NPC*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Merc*/		EmuConstants::ITEM_CONTAINER_SIZE,
/*Bot*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*Pet*/			EmuConstants::ITEM_CONTAINER_SIZE
	};

	return local[ValidateMobVersion(version)];
}

bool EQLimits::CoinHasWeight(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		true,
/*62*/			Client62::limits::COIN_HAS_WEIGHT,
/*Titanium*/	Titanium::limits::COIN_HAS_WEIGHT,
/*SoF*/			SoF::limits::COIN_HAS_WEIGHT,
/*SoD*/			SoD::limits::COIN_HAS_WEIGHT,
/*Underfoot*/	Underfoot::limits::COIN_HAS_WEIGHT,
/*RoF*/			RoF::limits::COIN_HAS_WEIGHT,
/*RoF2*/		true,
		
/*NPC*/			true,
/*Merc*/		true,
/*Bot*/			true,
/*Pet*/			true
	};

	return local[ValidateMobVersion(version)];
}

uint32 EQLimits::BandoliersCount(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::BANDOLIERS_COUNT,
/*Titanium*/	EmuConstants::BANDOLIERS_COUNT,
/*SoF*/			EmuConstants::BANDOLIERS_COUNT,
/*SoD*/			EmuConstants::BANDOLIERS_COUNT,
/*Underfoot*/	EmuConstants::BANDOLIERS_COUNT,
/*RoF*/			EmuConstants::BANDOLIERS_COUNT,
/*RoF2*/		0,
		
/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
	};

	return local[ValidateMobVersion(version)];
}

uint32 EQLimits::BandolierSize(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::BANDOLIER_SIZE,
/*Titanium*/	EmuConstants::BANDOLIER_SIZE,
/*SoF*/			EmuConstants::BANDOLIER_SIZE,
/*SoD*/			EmuConstants::BANDOLIER_SIZE,
/*Underfoot*/	EmuConstants::BANDOLIER_SIZE,
/*RoF*/			EmuConstants::BANDOLIER_SIZE,
/*RoF2*/		0,
		
/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
	};

	return local[ValidateMobVersion(version)];
}

uint32 EQLimits::PotionBeltSize(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			EmuConstants::POTION_BELT_SIZE,
/*Titanium*/	EmuConstants::POTION_BELT_SIZE,
/*SoF*/			EmuConstants::POTION_BELT_SIZE,
/*SoD*/			EmuConstants::POTION_BELT_SIZE,
/*Underfoot*/	EmuConstants::POTION_BELT_SIZE,
/*RoF*/			EmuConstants::POTION_BELT_SIZE,
/*RoF2*/		0,
		
/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
	};

	return local[ValidateMobVersion(version)];
}
