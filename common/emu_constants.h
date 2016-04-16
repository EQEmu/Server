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

#ifndef EMU_CONSTANTS_H
#define EMU_CONSTANTS_H

#include "eq_limits.h"
// (future use)
//using namespace RoF2::maps;	// server inventory maps enumeration (code and database sync'd to reference)
//using namespace RoF::slots;	// server possessions slots enumeration (code and database sync'd to reference)

#include "emu_legacy.h"
#include "light_source.h"
#include "deity.h"
#include "say_link.h"

#include <string>


// *** DO NOT CHANGE without a full understanding of the consequences..the server is set up to use these settings explicitly!! ***
// *** You will cause compilation failures and corrupt your database if partial or incorrect attempts to change them are made!! ***

// Hard-coded values usually indicate that further research is needed and the values given are from the old (known) system


namespace EQEmu
{
	// an immutable value is required to initialize arrays, etc... use this class as a repository for those
	class Constants {
	public:
		// database
		static const ClientVersion CHARACTER_CREATION_CLIENT = ClientVersion::RoF2; // adjust according to starting item placement and target client

		static const size_t CHARACTER_CREATION_LIMIT = RoF2::consts::CHARACTER_CREATION_LIMIT;

		// inventory
		static uint16 InventoryTypeSize(int16 type_index);
		//static const char* InventoryLocationName(Location_Struct location);
		static const char* InventoryTypeName(int16 type_index);
		static const char* InventorySlotName(int16 slot_index);
		static const char* InventorySubName(int16 sub_index);
		static const char* InventoryAugName(int16 aug_index);

		// these are currently hard-coded for existing inventory system..do not use in place of special client version handlers until ready
		static const uint16	TYPE_POSSESSIONS_SIZE = SlotCount;
		static const uint16 TYPE_BANK_SIZE = 24;
		static const uint16 TYPE_SHARED_BANK_SIZE = 2;
		static const uint16 TYPE_TRADE_SIZE = 8;
		static const uint16 TYPE_WORLD_SIZE = 10;
		static const uint16 TYPE_LIMBO_SIZE = 36;
		static const uint16 TYPE_TRIBUTE_SIZE = 5; // (need client values)
		static const uint16 TYPE_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_GUILD_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_MERCHANT_SIZE = 0;
		static const uint16 TYPE_DELETED_SIZE = 0;
		static const uint16 TYPE_CORPSE_SIZE = SlotCount; // no bitmask use..limits to size of client corpse window (see EQLimits::InventoryMapSize(MapCorpse, <EQClientVersion))
		static const uint16 TYPE_BAZAAR_SIZE = 80;
		static const uint16 TYPE_INSPECT_SIZE = 22;
		static const uint16 TYPE_REAL_ESTATE_SIZE = 0;
		static const uint16 TYPE_VIEW_MOD_PC_SIZE = NOT_USED;
		static const uint16 TYPE_VIEW_MOD_BANK_SIZE = NOT_USED;
		static const uint16 TYPE_VIEW_MOD_SHARED_BANK_SIZE = NOT_USED;
		static const uint16 TYPE_VIEW_MOD_LIMBO_SIZE = NOT_USED;
		static const uint16 TYPE_ALT_STORAGE_SIZE = 0;
		static const uint16 TYPE_ARCHIVED_SIZE = 0;
		static const uint16 TYPE_MAIL_SIZE = 0;
		static const uint16 TYPE_GUILD_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_KRONO_SIZE = 0;
		static const uint16 TYPE_OTHER_SIZE = 0;

		// most of these definitions will go away with the structure-based system..this maintains compatibility for now
		// (these are mainly to assign specific values to constants used in conversions and to identify per-client ranges/offsets)
		static const int16 EQUIPMENT_BEGIN = SlotCharm;
		static const int16 EQUIPMENT_END = SlotAmmo;
		static const uint16 EQUIPMENT_SIZE = 22; // does not account for 'Power Source' - used mainly for npc equipment arrays

		static const int16 GENERAL_BEGIN = SlotGeneral1;
		static const int16 GENERAL_END = SlotGeneral8;
		static const uint16 GENERAL_SIZE = 8;
		static const int16 GENERAL_BAGS_BEGIN = 251;
		static const int16 GENERAL_BAGS_END_OFFSET = 79;
		static const int16 GENERAL_BAGS_END = GENERAL_BAGS_BEGIN + GENERAL_BAGS_END_OFFSET;

		static const int16 CURSOR_BAG_BEGIN = 331;
		static const int16 CURSOR_BAG_END_OFFSET = 9;
		static const int16 CURSOR_BAG_END = CURSOR_BAG_BEGIN + CURSOR_BAG_END_OFFSET;

		static const int16 BANK_BEGIN = 2000;
		static const int16 BANK_END = 2023;
		static const int16 BANK_BAGS_BEGIN = 2031;
		static const int16 BANK_BAGS_END_OFFSET = 239;
		static const int16 BANK_BAGS_END = BANK_BAGS_BEGIN + BANK_BAGS_END_OFFSET;

		static const int16 SHARED_BANK_BEGIN = 2500;
		static const int16 SHARED_BANK_END = 2501;
		static const int16 SHARED_BANK_BAGS_BEGIN = 2531;
		static const int16 SHARED_BANK_BAGS_END_OFFSET = 19;
		static const int16 SHARED_BANK_BAGS_END = SHARED_BANK_BAGS_BEGIN + SHARED_BANK_BAGS_END_OFFSET;

		static const int16 TRADE_BEGIN = 3000;
		static const int16 TRADE_END = 3007;
		static const int16 TRADE_NPC_END = 3003;
		static const int16 TRADE_BAGS_BEGIN = 3031;
		static const int16 TRADE_BAGS_END_OFFSET = 79;
		static const int16 TRADE_BAGS_END = TRADE_BAGS_BEGIN + TRADE_BAGS_END_OFFSET;

		static const int16 WORLD_BEGIN = 4000;
		static const int16 WORLD_END = 4009;
		static const int16 WORLD_SIZE = TYPE_WORLD_SIZE;

		static const int16 TRIBUTE_BEGIN = 400;
		static const int16 TRIBUTE_END = 404;
		static const int16 TRIBUTE_SIZE = TYPE_TRIBUTE_SIZE;

		static const int16 CORPSE_BEGIN = 22;
		//static const int16 CORPSE_END = RoF::consts::CORPSE_END; // not ready for use

		static const int16 MATERIAL_BEGIN = MaterialHead;
		static const int16 MATERIAL_END = MaterialSecondary;
		static const int16 MATERIAL_TINT_END = MaterialFeet;
		static const int16 MATERIAL_SIZE = MaterialCount;

		// items
		// common and container sizes will not increase until the new 'location' struct is implemented
		static const uint16 ITEM_COMMON_SIZE = RoF::consts::ITEM_COMMON_SIZE;
		static const uint16 ITEM_CONTAINER_SIZE = Titanium::consts::ITEM_CONTAINER_SIZE;

		// BANDOLIERS_SIZE sets maximum limit..active limit will need to be handled by the appropriate AA or spell (or item?)
		static const size_t BANDOLIERS_SIZE = RoF2::consts::BANDOLIERS_SIZE;			// number of bandolier instances
		static const size_t BANDOLIER_ITEM_COUNT = RoF2::consts::BANDOLIER_ITEM_COUNT;	// number of equipment slots in bandolier instance

		// POTION_BELT_SIZE sets maximum limit..active limit will need to be handled by the appropriate AA or spell (or item?)
		static const size_t POTION_BELT_ITEM_COUNT = RoF2::consts::POTION_BELT_ITEM_COUNT;

		static const size_t TEXT_LINK_BODY_LENGTH = RoF2::consts::TEXT_LINK_BODY_LENGTH;
	};
}

#endif /* EMU_CONSTANTS_H */
