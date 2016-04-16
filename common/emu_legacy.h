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

#ifndef EMU_LEGACY_H
#define EMU_LEGACY_H

#include "types.h"


namespace EQEmu
{
	// this is for perl and other legacy systems
	class Legacy {
	public:
		typedef enum {
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
			//SLOT_GENERAL_9 = not supported
			//SLOT_GENERAL_10 = not supported
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
		} InventorySlot;
	};
}

#endif /* EMU_LEGACY_H */
