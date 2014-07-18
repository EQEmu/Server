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

#include "eq_dictionary.h"

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
		return MAP_SHAREDBANK_SIZE;
	case MapTrade:
		return MAP_TRADE_SIZE;
	case MapWorld:
		return MAP_WORLD_SIZE;
	case MapLimbo:
		return MAP_LIMBO_SIZE;
	case MapTribute:
		return MAP_TRIBUTE_SIZE;
	case MapTrophyTribute:
		return MAP_TROPHYTRIBUTE_SIZE;
	case MapGuildTribute:
		return MAP_GUILDTRIBUTE_SIZE;
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
		return MAP_REALESTATE_SIZE;
	case MapViewMODPC:
		return MAP_VIEWMODPC_SIZE;
	case MapViewMODBank:
		return MAP_VIEWMODBANK_SIZE;
	case MapViewMODSharedBank:
		return MAP_VIEWMODSHAREDBANK_SIZE;
	case MapViewMODLimbo:
		return MAP_VIEWMODLIMBO_SIZE;
	case MapAltStorage:
		return MAP_ALTSTORAGE_SIZE;
	case MapArchived:
		return MAP_ARCHIVED_SIZE;
	case MapMail:
		return MAP_MAIL_SIZE;
	case MapGuildTrophyTribute:
		return MAP_GUILDTROPHYTRIBUTE_SIZE;
	case MapKrono:
		return MAP_KRONO_SIZE;
	case MapOther:
		return MAP_OTHER_SIZE;
	default:
		return NOT_USED;
	}
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

	static const uint16 local[_MapCount][_EmuClientCount] = {
	/*	{	Unknown,	62,			Titanium,	SoF,		SoD,		Underfoot,	RoF,										RoF2,	NPC,	Merc,	Bot,	Pet		}*/
		{	NOT_USED,	34,			34,			34,			34,			34,			EmuConstants::MAP_POSSESSIONS_SIZE,			0,		0,		0,		0,		0		}, // (requires bitmask use...)
		{	NOT_USED,	16,			16,			24,			24,			24,			EmuConstants::MAP_BANK_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	2,			2,			2,			2,			2,			EmuConstants::MAP_SHAREDBANK_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	8,			8,			8,			8,			8,			EmuConstants::MAP_TRADE_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	10,			10,			10,			10,			10,			EmuConstants::MAP_WORLD_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	36,			36,			36,			36,			36,			EmuConstants::MAP_LIMBO_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_TRIBUTE_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_TROPHYTRIBUTE_SIZE,		0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_GUILDTRIBUTE_SIZE,		0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_MERCHANT_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_DELETED_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			34,			EmuConstants::MAP_CORPSE_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			80,			EmuConstants::MAP_BAZAAR_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_INSPECT_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_REALESTATE_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_VIEWMODPC_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_VIEWMODBANK_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_VIEWMODSHAREDBANK_SIZE,	0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_VIEWMODLIMBO_SIZE,		0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_ALTSTORAGE_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_ARCHIVED_SIZE,			0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_MAIL_SIZE,				0,		0,		0,		0,		0		},
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_GUILDTROPHYTRIBUTE_SIZE,	0,		0,		0,		0,		0		},
		{	NOT_USED,	NOT_USED,	NOT_USED,	NOT_USED,	NOT_USED,	NOT_USED,	EmuConstants::MAP_KRONO_SIZE,				0,		0,		0,		0,		0		}, // (will be implemented in RoF2)
		{	NOT_USED,	0,			0,			0,			0,			0,			EmuConstants::MAP_OTHER_SIZE,				0,		0,		0,		0,		0		}
	};

	if ((uint16)map < _MapCount)
		return local[map][ValidateMobVersion(version)];

	return NOT_USED;
}

uint64 EQLimits::PossessionsBitmask(uint32 version) {
	// these are for the new inventory system..not the current one...
	// 0x0000000000200000 is SlotPowerSource (SoF+)
	// 0x0000000100000000 is SlotGeneral9 (RoF+)
	// 0x0000000200000000 is SlotGeneral10 (RoF+)

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

	return local[ValidateMobVersion(version)];
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

	return local[ValidateMobVersion(version)];
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

	return local[ValidateMobVersion(version)];
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

	return local[ValidateMobVersion(version)];
}

bool EQLimits::AllowsEmptyBagInBag(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		false,
/*62*/			false,
/*Titanium*/	false,
/*SoF*/			false,
/*SoD*/			false,
/*Underfoot*/	false,
/*RoF*/			true,
/*RoF2*/		true,
		
/*NPC*/			true,
/*Merc*/		true,
/*Bot*/			true,
/*Pet*/			true
	};
	
	return local[ValidateMobVersion(version)];
}

// items
uint16 EQLimits::ItemCommonSize(uint32 version) {
	static const uint16 local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			5,
/*Titanium*/	5,
/*SoF*/			5,
/*SoD*/			5,
/*Underfoot*/	5,
/*RoF*/			EmuConstants::ITEM_COMMON_SIZE,
/*RoF2*/		0,
		
/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
	};

	return local[ValidateMobVersion(version)];
}

uint16 EQLimits::ItemContainerSize(uint32 version) {
	static const uint16 local[_EmuClientCount] = {
/*Unknown*/		NOT_USED,
/*62*/			10,
/*Titanium*/	10,
/*SoF*/			10,
/*SoD*/			10,
/*Underfoot*/	10,
/*RoF*/			EmuConstants::ITEM_CONTAINER_SIZE,
/*RoF2*/		0,
		
/*NPC*/			0,
/*Merc*/		0,
/*Bot*/			0,
/*Pet*/			0
	};

	return local[ValidateMobVersion(version)];
}

bool EQLimits::CoinHasWeight(uint32 version) {
	static const bool local[_EmuClientCount] = {
/*Unknown*/		true,
/*62*/			true,
/*Titanium*/	true,
/*SoF*/			true,
/*SoD*/			false,
/*Underfoot*/	false,
/*RoF*/			false,
/*RoF2*/		false,
		
/*NPC*/			false,
/*Merc*/		false,
/*Bot*/			false,
/*Pet*/			false
	};

	return local[ValidateMobVersion(version)];
}
