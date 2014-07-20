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

#ifndef EQ_DICTIONARY_H
#define EQ_DICTIONARY_H

#include "types.h"
#include "eq_constants.h"
#include "clientversions.h"

// an immutable value is required to initialize arrays, etc... use this class as a repository for those
typedef class {
public:
	// database
	static const EQClientVersion CHARACTER_CREATION_CLIENT = _EQCreationClient;

	// inventory
	static uint16 InventoryMapSize(int16 map);

	static const uint16 MAP_POSSESSIONS_SIZE = 22; //_SlotCount;
	static const uint16 MAP_BANK_SIZE = 24;
	static const uint16 MAP_SHAREDBANK_SIZE = 2;
	static const uint16 MAP_TRADE_SIZE = 8;
	static const uint16 MAP_WORLD_SIZE = 10;
	static const uint16 MAP_LIMBO_SIZE = 36;
	static const uint16 MAP_TRIBUTE_SIZE = 5;
	static const uint16 MAP_TROPHYTRIBUTE_SIZE = 0;
	static const uint16 MAP_GUILDTRIBUTE_SIZE = 0;
	static const uint16 MAP_MERCHANT_SIZE = 0;
	static const uint16 MAP_DELETED_SIZE = 0;
	static const uint16 MAP_CORPSE_SIZE = 22; //_SlotCount; // actual code still needs lots of work...
	static const uint16 MAP_BAZAAR_SIZE = 80; //200;
	static const uint16 MAP_INSPECT_SIZE = 22; //_SlotEquipmentCount;
	static const uint16 MAP_REALESTATE_SIZE = 0;
	static const uint16 MAP_VIEWMODPC_SIZE = NOT_USED;
	static const uint16 MAP_VIEWMODBANK_SIZE = NOT_USED;
	static const uint16 MAP_VIEWMODSHAREDBANK_SIZE = NOT_USED;
	static const uint16 MAP_VIEWMODLIMBO_SIZE = NOT_USED;
	static const uint16 MAP_ALTSTORAGE_SIZE = 0;
	static const uint16 MAP_ARCHIVED_SIZE = 0;
	static const uint16 MAP_MAIL_SIZE = 0;
	static const uint16 MAP_GUILDTROPHYTRIBUTE_SIZE = 0;
	static const uint16 MAP_KRONO_SIZE = 0; // this will be '1' when RoF2 is implemented
	static const uint16 MAP_OTHER_SIZE = 0;

	//static const int16 EQUIPMENT_BEGIN = _SlotEquipmentBegin;
	//static const int16 EQUIPMENT_END = _SlotEquipmentEnd;
	static const uint16 EQUIPMENT_SIZE = 22; //_SlotEquipmentCount; // not ready for client usage..only equipment arrays for npcs...

	static const int16 GENERAL_BEGIN = 22; //_SlotGeneralBegin;
	static const int16 GENERAL_END = 29; //_SlotGeneralEnd;
	static const uint16 GENERAL_SIZE = 8; //_SlotGeneralCount;

	// items
	static const uint16 ITEM_COMMON_SIZE = 5;
	static const uint16 ITEM_CONTAINER_SIZE = 10;

	// player profile
	//static const uint32 CLASS_BITMASK = 0;	// needs value
	//static const uint32 RACE_BITMASK = 0;	// needs value

	// TODO: resolve naming convention and use for bandolier count versus size
	//static const uint32 BANDOLIER_COUNT = 4;
	static const uint32 BANDOLIER_SIZE = 4;
	static const uint32 POTION_BELT_SIZE = 5;
} EmuConstants;

typedef class {
public:
	// client version validation (checks to avoid crashing zone server when accessing reference arrays)
	// use this inside of class Client (limits to actual clients)
	static bool				IsValidClientVersion(uint32 version);
	static uint32			ValidateClientVersion(uint32 version);
	static EQClientVersion	ValidateClientVersion(EQClientVersion version);

	// basically..any non-client classes - do not when setting a valid client
	static bool				IsValidNPCVersion(uint32 version);
	static uint32			ValidateNPCVersion(uint32 version);
	static EQClientVersion	ValidateNPCVersion(EQClientVersion version);

	// these are 'universal' - do not when setting a valid client
	static bool				IsValidMobVersion(uint32 version);
	static uint32			ValidateMobVersion(uint32 version);
	static EQClientVersion	ValidateMobVersion(EQClientVersion version);

	// inventory
	static uint16	InventoryMapSize(int16 map, uint32 version);
	static uint64	PossessionsBitmask(uint32 version);
	static uint64	EquipmentBitmask(uint32 version);
	static uint64	GeneralBitmask(uint32 version);
	static uint64	CursorBitmask(uint32 version);

	static bool	AllowsEmptyBagInBag(uint32 version);

	// items
	static uint16	ItemCommonSize(uint32 version);
	static uint16	ItemContainerSize(uint32 version);

	// player profile
	static bool	CoinHasWeight(uint32 version);
} EQLimits;

#endif /* EQ_LIMITS_H */
