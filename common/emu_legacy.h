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

#ifndef COMMON_EMU_LEGACY_H
#define COMMON_EMU_LEGACY_H

#include "types.h"

#include <stdlib.h>


namespace EQEmu
{
	// this is for perl and other legacy systems
	namespace legacy {
		enum InventorySlot {
			SLOT_CHARM = 0,
			SLOT_EAR01 = 1,
			SLOT_HEAD = 2,
			SLOT_FACE = 3,
			SLOT_EAR02 = 4,
			SLOT_NECK = 5,
			SLOT_SHOULDER = 6,
			SLOT_ARMS = 7,
			SLOT_BACK = 8,
			SLOT_BRACER01 = 9,
			SLOT_BRACER02 = 10,
			SLOT_RANGE = 11,
			SLOT_HANDS = 12,
			SLOT_PRIMARY = 13,
			SLOT_SECONDARY = 14,
			SLOT_RING01 = 15,
			SLOT_RING02 = 16,
			SLOT_CHEST = 17,
			SLOT_LEGS = 18,
			SLOT_FEET = 19,
			SLOT_WAIST = 20,
			SLOT_POWER_SOURCE = 9999,
			SLOT_AMMO = 21,
			SLOT_GENERAL_1 = 22,
			SLOT_GENERAL_2 = 23,
			SLOT_GENERAL_3 = 24,
			SLOT_GENERAL_4 = 25,
			SLOT_GENERAL_5 = 26,
			SLOT_GENERAL_6 = 27,
			SLOT_GENERAL_7 = 28,
			SLOT_GENERAL_8 = 29,
			SLOT_CURSOR = 30,
			SLOT_CURSOR_END = (int16)0xFFFE, // I hope no one is using this...
			SLOT_TRADESKILL = 1000,
			SLOT_AUGMENT = 1001,
			SLOT_INVALID = (int16)0xFFFF,
			SLOT_POSSESSIONS_BEGIN = 0,
			SLOT_POSSESSIONS_END = 30,
			SLOT_EQUIPMENT_BEGIN = 0,
			SLOT_EQUIPMENT_END = 21,
			SLOT_PERSONAL_BEGIN = 22,
			SLOT_PERSONAL_END = 29,
			SLOT_PERSONAL_BAGS_BEGIN = 251,
			SLOT_PERSONAL_BAGS_END = 330,
			SLOT_CURSOR_BAG_BEGIN = 331,
			SLOT_CURSOR_BAG_END = 340,
			SLOT_TRIBUTE_BEGIN = 400,
			SLOT_TRIBUTE_END = 404,
			SLOT_GUILD_TRIBUTE_BEGIN = 450,
			SLOT_GUILD_TRIBUTE_END = 451,
			SLOT_BANK_BEGIN = 2000,
			SLOT_BANK_END = 2023,
			SLOT_BANK_BAGS_BEGIN = 2031,
			SLOT_BANK_BAGS_END = 2270,
			SLOT_SHARED_BANK_BEGIN = 2500,
			SLOT_SHARED_BANK_END = 2501,
			SLOT_SHARED_BANK_BAGS_BEGIN = 2531,
			SLOT_SHARED_BANK_BAGS_END = 2550,
			SLOT_TRADE_BEGIN = 3000,
			SLOT_TRADE_END = 3007,
			SLOT_TRADE_BAGS_BEGIN = 3031,
			SLOT_TRADE_BAGS_END = 3110,
			SLOT_WORLD_BEGIN = 4000,
			SLOT_WORLD_END = 4009
		};

		// these are currently hard-coded for existing inventory system..do not use in place of special client version handlers until ready
		static const uint16	TYPE_POSSESSIONS_SIZE = 31;
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
		static const uint16 TYPE_CORPSE_SIZE = 31; // no bitmask use..limits to size of client corpse window (see EQLimits::InventoryMapSize(MapCorpse, <EQClientVersion))
		static const uint16 TYPE_BAZAAR_SIZE = 80;
		static const uint16 TYPE_INSPECT_SIZE = 22;
		static const uint16 TYPE_REAL_ESTATE_SIZE = 0;
		static const uint16 TYPE_VIEW_MOD_PC_SIZE = 0;
		static const uint16 TYPE_VIEW_MOD_BANK_SIZE = 0;
		static const uint16 TYPE_VIEW_MOD_SHARED_BANK_SIZE = 0;
		static const uint16 TYPE_VIEW_MOD_LIMBO_SIZE = 0;
		static const uint16 TYPE_ALT_STORAGE_SIZE = 0;
		static const uint16 TYPE_ARCHIVED_SIZE = 0;
		static const uint16 TYPE_MAIL_SIZE = 0;
		static const uint16 TYPE_GUILD_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_KRONO_SIZE = 0;
		static const uint16 TYPE_OTHER_SIZE = 0;

		// most of these definitions will go away with the structure-based system..this maintains compatibility for now
		// (these are mainly to assign specific values to constants used in conversions and to identify per-client ranges/offsets)
		static const int16 EQUIPMENT_BEGIN = 0;
		static const int16 EQUIPMENT_END = 21;
		static const uint16 EQUIPMENT_SIZE = 22; // does not account for 'Power Source' - used mainly for npc equipment arrays

		static const int16 GENERAL_BEGIN = 22;
		static const int16 GENERAL_END = 29;
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

		// BANDOLIERS_SIZE sets maximum limit..active limit will need to be handled by the appropriate AA or spell (or item?)
		static const size_t BANDOLIERS_SIZE = 20;		// number of bandolier instances
		static const size_t BANDOLIER_ITEM_COUNT = 4;	// number of equipment slots in bandolier instance

		// POTION_BELT_SIZE sets maximum limit..active limit will need to be handled by the appropriate AA or spell (or item?)
		static const size_t POTION_BELT_ITEM_COUNT = 5;

		static const size_t TEXT_LINK_BODY_LENGTH = 56;
	}

}

#endif /* COMMON_EMU_LEGACY_H */
