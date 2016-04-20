/*	EQEMu:  Everquest Server Emulator
	
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

#include "emu_constants.h"


// client validation
bool EQEmu::limits::IsValidPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_PC_CLIENT)
		return true;

	return false;
}

ClientVersion EQEmu::limits::ValidatePCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_PC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// npc validation
bool EQEmu::limits::IsValidNPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > LAST_PC_CLIENT && clientVersion <= LAST_NPC_CLIENT)
		return true;

	return false;
}

ClientVersion EQEmu::limits::ValidateNPCClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > LAST_PC_CLIENT && clientVersion <= LAST_NPC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// mob validation
bool EQEmu::limits::IsValidMobClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_NPC_CLIENT)
		return true;

	return false;
}

ClientVersion EQEmu::limits::ValidateMobClientVersion(ClientVersion clientVersion)
{
	if (clientVersion > ClientVersion::Unknown && clientVersion <= LAST_NPC_CLIENT)
		return clientVersion;

	return ClientVersion::Unknown;
}

// database
size_t EQEmu::limits::CharacterCreationLimit(ClientVersion clientVersion)
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
uint16 EQEmu::limits::InventoryMapSize(int16 indexMap, ClientVersion clientVersion)
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
	// when setting NPC-based values, try to adhere to an constants::<property> or NOT_USED value to avoid unnecessary issues

	static const uint16 local[TypeCount][CLIENT_VERSION_COUNT] = {
		// server and database are sync'd to current TypePossessions's client as set in 'using namespace RoF::slots;' and
		// 'constants::TYPE_POSSESSIONS_SIZE' - use/update EquipmentBitmask(), GeneralBitmask() and CursorBitmask()
		// for partial range validation checks and 'constants::TYPE_POSSESSIONS_SIZE' for full range iterations
		{ // local[TypePossessions]
/*Unknown*/		NOT_USED,
/*62*/			constants::TYPE_POSSESSIONS_SIZE,
/*Titanium*/	constants::TYPE_POSSESSIONS_SIZE,
/*SoF*/			constants::TYPE_POSSESSIONS_SIZE,
/*SoD*/			constants::TYPE_POSSESSIONS_SIZE,
/*Underfoot*/	constants::TYPE_POSSESSIONS_SIZE,
/*RoF*/			constants::TYPE_POSSESSIONS_SIZE,
/*RoF2*/		constants::TYPE_POSSESSIONS_SIZE,

/*NPC*/			constants::TYPE_POSSESSIONS_SIZE,
/*Merc*/		constants::TYPE_POSSESSIONS_SIZE,
/*Bot*/			constants::TYPE_POSSESSIONS_SIZE,
/*Pet*/			constants::TYPE_POSSESSIONS_SIZE
		},
		{ // local[TypeBank]
/*Unknown*/		NOT_USED,
/*62*/			NOT_USED,
/*Titanium*/	Titanium::consts::TYPE_BANK_SIZE,
/*SoF*/			constants::TYPE_BANK_SIZE,
/*SoD*/			constants::TYPE_BANK_SIZE,
/*Underfoot*/	constants::TYPE_BANK_SIZE,
/*RoF*/			constants::TYPE_BANK_SIZE,
/*RoF2*/		constants::TYPE_BANK_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeSharedBank]
/*Unknown*/		NOT_USED,
/*62*/			constants::TYPE_SHARED_BANK_SIZE,
/*Titanium*/	constants::TYPE_SHARED_BANK_SIZE,
/*SoF*/			constants::TYPE_SHARED_BANK_SIZE,
/*SoD*/			constants::TYPE_SHARED_BANK_SIZE,
/*Underfoot*/	constants::TYPE_SHARED_BANK_SIZE,
/*RoF*/			constants::TYPE_SHARED_BANK_SIZE,
/*RoF2*/		constants::TYPE_SHARED_BANK_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeTrade]
/*Unknown*/		NOT_USED,
/*62*/			constants::TYPE_TRADE_SIZE,
/*Titanium*/	constants::TYPE_TRADE_SIZE,
/*SoF*/			constants::TYPE_TRADE_SIZE,
/*SoD*/			constants::TYPE_TRADE_SIZE,
/*Underfoot*/	constants::TYPE_TRADE_SIZE,
/*RoF*/			constants::TYPE_TRADE_SIZE,
/*RoF2*/		constants::TYPE_TRADE_SIZE,

/*NPC*/			4,
/*Merc*/		4,
/*Bot*/			constants::TYPE_TRADE_SIZE, // client thinks this is another client
/*Pet*/			4
		},
		{ // local[TypeWorld]
/*Unknown*/		NOT_USED,
/*62*/			constants::TYPE_WORLD_SIZE,
/*Titanium*/	constants::TYPE_WORLD_SIZE,
/*SoF*/			constants::TYPE_WORLD_SIZE,
/*SoD*/			constants::TYPE_WORLD_SIZE,
/*Underfoot*/	constants::TYPE_WORLD_SIZE,
/*RoF*/			constants::TYPE_WORLD_SIZE,
/*RoF2*/		constants::TYPE_WORLD_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeLimbo]
/*Unknown*/		NOT_USED,
/*62*/			constants::TYPE_LIMBO_SIZE,
/*Titanium*/	constants::TYPE_LIMBO_SIZE,
/*SoF*/			constants::TYPE_LIMBO_SIZE,
/*SoD*/			constants::TYPE_LIMBO_SIZE,
/*Underfoot*/	constants::TYPE_LIMBO_SIZE,
/*RoF*/			constants::TYPE_LIMBO_SIZE,
/*RoF2*/		constants::TYPE_LIMBO_SIZE,

/*NPC*/			NOT_USED,
/*Merc*/		NOT_USED,
/*Bot*/			NOT_USED,
/*Pet*/			NOT_USED
		},
		{ // local[TypeTribute]
/*Unknown*/		NOT_USED,
/*62*/			constants::TYPE_TRIBUTE_SIZE,
/*Titanium*/	constants::TYPE_TRIBUTE_SIZE,
/*SoF*/			constants::TYPE_TRIBUTE_SIZE,
/*SoD*/			constants::TYPE_TRIBUTE_SIZE,
/*Underfoot*/	constants::TYPE_TRIBUTE_SIZE,
/*RoF*/			constants::TYPE_TRIBUTE_SIZE,
/*RoF2*/		constants::TYPE_TRIBUTE_SIZE,

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
/*RoF*/			constants::TYPE_TROPHY_TRIBUTE_SIZE,
/*RoF2*/		constants::TYPE_TROPHY_TRIBUTE_SIZE,

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
/*RoF*/			constants::TYPE_GUILD_TRIBUTE_SIZE,
/*RoF2*/		constants::TYPE_GUILD_TRIBUTE_SIZE,

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
/*RoF*/			constants::TYPE_MERCHANT_SIZE,
/*RoF2*/		constants::TYPE_MERCHANT_SIZE,

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
/*RoF*/			constants::TYPE_DELETED_SIZE,
/*RoF2*/		constants::TYPE_DELETED_SIZE,

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
/*62*/			constants::TYPE_BAZAAR_SIZE,
/*Titanium*/	constants::TYPE_BAZAAR_SIZE,
/*SoF*/			constants::TYPE_BAZAAR_SIZE,
/*SoD*/			constants::TYPE_BAZAAR_SIZE,
/*Underfoot*/	constants::TYPE_BAZAAR_SIZE,
/*RoF*/			constants::TYPE_BAZAAR_SIZE,
/*RoF2*/		constants::TYPE_BAZAAR_SIZE,

/*NPC*/			0, // this may need to be 'constants::TYPE_BAZAAR_SIZE' if offline client traders respawn as an npc
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
/*RoF*/			constants::TYPE_REAL_ESTATE_SIZE,
/*RoF2*/		constants::TYPE_REAL_ESTATE_SIZE,

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
/*RoF*/			constants::TYPE_VIEW_MOD_PC_SIZE,
/*RoF2*/		constants::TYPE_VIEW_MOD_PC_SIZE,

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
/*RoF*/			constants::TYPE_VIEW_MOD_BANK_SIZE,
/*RoF2*/		constants::TYPE_VIEW_MOD_BANK_SIZE,

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
/*RoF*/			constants::TYPE_VIEW_MOD_SHARED_BANK_SIZE,
/*RoF2*/		constants::TYPE_VIEW_MOD_SHARED_BANK_SIZE,

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
/*RoF*/			constants::TYPE_VIEW_MOD_LIMBO_SIZE,
/*RoF2*/		constants::TYPE_VIEW_MOD_LIMBO_SIZE,

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
/*RoF*/			constants::TYPE_ALT_STORAGE_SIZE,
/*RoF2*/		constants::TYPE_ALT_STORAGE_SIZE,

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
/*RoF*/			constants::TYPE_ARCHIVED_SIZE,
/*RoF2*/		constants::TYPE_ARCHIVED_SIZE,

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
/*RoF*/			constants::TYPE_MAIL_SIZE,
/*RoF2*/		constants::TYPE_MAIL_SIZE,

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
/*RoF*/			constants::TYPE_GUILD_TROPHY_TRIBUTE_SIZE,
/*RoF2*/		constants::TYPE_GUILD_TROPHY_TRIBUTE_SIZE,

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
/*RoF*/			constants::TYPE_KRONO_SIZE,
/*RoF2*/		constants::TYPE_KRONO_SIZE,

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
/*RoF*/			constants::TYPE_OTHER_SIZE,
/*RoF2*/		constants::TYPE_OTHER_SIZE,

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

uint64 EQEmu::limits::PossessionsBitmask(ClientVersion clientVersion)
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

uint64 EQEmu::limits::EquipmentBitmask(ClientVersion clientVersion)
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

uint64 EQEmu::limits::GeneralBitmask(ClientVersion clientVersion)
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

uint64 EQEmu::limits::CursorBitmask(ClientVersion clientVersion)
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

bool EQEmu::limits::AllowsEmptyBagInBag(ClientVersion clientVersion)
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

bool EQEmu::limits::AllowsClickCastFromBag(ClientVersion clientVersion)
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
uint16 EQEmu::limits::ItemCommonSize(ClientVersion clientVersion)
{
	static const uint16 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			constants::ITEM_COMMON_SIZE,
/*Titanium*/	constants::ITEM_COMMON_SIZE,
/*SoF*/			constants::ITEM_COMMON_SIZE,
/*SoD*/			constants::ITEM_COMMON_SIZE,
/*Underfoot*/	constants::ITEM_COMMON_SIZE,
/*RoF*/			constants::ITEM_COMMON_SIZE,
/*RoF2*/		constants::ITEM_COMMON_SIZE,

/*NPC*/			constants::ITEM_COMMON_SIZE,
/*Merc*/		constants::ITEM_COMMON_SIZE,
/*Bot*/			constants::ITEM_COMMON_SIZE,
/*Pet*/			constants::ITEM_COMMON_SIZE
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

uint16 EQEmu::limits::ItemContainerSize(ClientVersion clientVersion)
{
	static const uint16 local[CLIENT_VERSION_COUNT] = {
/*Unknown*/		NOT_USED,
/*62*/			constants::ITEM_CONTAINER_SIZE,
/*Titanium*/	constants::ITEM_CONTAINER_SIZE,
/*SoF*/			constants::ITEM_CONTAINER_SIZE,
/*SoD*/			constants::ITEM_CONTAINER_SIZE,
/*Underfoot*/	constants::ITEM_CONTAINER_SIZE,
/*RoF*/			constants::ITEM_CONTAINER_SIZE,
/*RoF2*/		constants::ITEM_CONTAINER_SIZE,

/*NPC*/			constants::ITEM_CONTAINER_SIZE,
/*Merc*/		constants::ITEM_CONTAINER_SIZE,
/*Bot*/			constants::ITEM_CONTAINER_SIZE,
/*Pet*/			constants::ITEM_CONTAINER_SIZE
	};

	return local[static_cast<unsigned int>(ValidateMobClientVersion(clientVersion))];
}

bool EQEmu::limits::CoinHasWeight(ClientVersion clientVersion)
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
