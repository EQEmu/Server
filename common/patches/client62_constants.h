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

#ifndef CLIENT62_CONSTANTS_H_
#define CLIENT62_CONSTANTS_H_

#include "../common/types.h"

namespace Client62 {
	namespace maps {
		typedef enum : int16 {
			// this needs work to match actual client equivilents
			MapPossessions = 0,
			MapBank,
			MapSharedBank,
			MapTrade,
			MapWorld,
			MapLimbo,
			MapTribute,
			MapTrophyTribute,
			MapGuildTribute,
			MapMerchant,
			MapDeleted,
			MapCorpse,
			MapBazaar,
			MapInspect,
			MapRealEstate,
			MapViewMODPC,
			MapViewMODBank,
			MapViewMODSharedBank,
			MapViewMODLimbo,
			MapAltStorage,
			MapArchived,
			MapMail,
			MapGuildTrophyTribute,
			MapOther,
			_MapCount
		} InventoryMaps;
	}

	namespace slots {
		typedef enum : int16 {
			MainCharm = 0,
			MainEar1,
			MainHead,
			MainFace,
			MainEar2,
			MainNeck,
			MainShoulders,
			MainArms,
			MainBack,
			MainWrist1,
			MainWrist2,
			MainRange,
			MainHands,
			MainPrimary,
			MainSecondary,
			MainFinger1,
			MainFinger2,
			MainChest,
			MainLegs,
			MainFeet,
			MainWaist,
			MainAmmo,
			MainGeneral1,
			MainGeneral2,
			MainGeneral3,
			MainGeneral4,
			MainGeneral5,
			MainGeneral6,
			MainGeneral7,
			MainGeneral8,
			MainCursor,
			_MainCount,
			_MainEquipmentBegin = MainCharm,
			_MainEquipmentEnd = MainAmmo,
			_MainEquipmentCount = (_MainEquipmentEnd - _MainEquipmentBegin + 1),
			_MainGeneralBegin = MainGeneral1,
			_MainGeneralEnd = MainGeneral8,
			_MainGeneralCount = (_MainGeneralEnd - _MainGeneralBegin + 1)
		} EquipmentSlots;
	}

	namespace consts {
		static const uint16	MAP_POSSESSIONS_SIZE = slots::_MainCount;
		static const uint16 MAP_BANK_SIZE = 16;
		static const uint16 MAP_SHARED_BANK_SIZE = 2;
		static const uint16 MAP_TRADE_SIZE = 8;
		static const uint16 MAP_WORLD_SIZE = 10;
		static const uint16 MAP_LIMBO_SIZE = 36;
		static const uint16 MAP_TRIBUTE_SIZE = 0; //?
		static const uint16 MAP_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 MAP_GUILD_TRIBUTE_SIZE = 0;
		static const uint16 MAP_MERCHANT_SIZE = 0;
		static const uint16 MAP_DELETED_SIZE = 0;
		static const uint16 MAP_CORPSE_SIZE = slots::_MainCount;
		static const uint16 MAP_BAZAAR_SIZE = 80;
		static const uint16 MAP_INSPECT_SIZE = slots::_MainEquipmentCount;
		static const uint16 MAP_REAL_ESTATE_SIZE = 0;
		static const uint16 MAP_VIEW_MOD_PC_SIZE = MAP_POSSESSIONS_SIZE;
		static const uint16 MAP_VIEW_MOD_BANK_SIZE = MAP_BANK_SIZE;
		static const uint16 MAP_VIEW_MOD_SHARED_BANK_SIZE = MAP_SHARED_BANK_SIZE;
		static const uint16 MAP_VIEW_MOD_LIMBO_SIZE = MAP_LIMBO_SIZE;
		static const uint16 MAP_ALT_STORAGE_SIZE = 0;
		static const uint16 MAP_ARCHIVED_SIZE = 0;
		static const uint16 MAP_MAIL_SIZE = 0;
		static const uint16 MAP_GUILD_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 MAP_KRONO_SIZE = NOT_USED;
		static const uint16 MAP_OTHER_SIZE = 0;

		static const int16 EQUIPMENT_BEGIN = slots::MainCharm;
		static const int16 EQUIPMENT_END = slots::MainAmmo;
		static const uint16 EQUIPMENT_SIZE = slots::_MainEquipmentCount;

		static const int16 GENERAL_BEGIN = slots::MainGeneral1;
		static const int16 GENERAL_END = slots::MainGeneral8;
		static const uint16 GENERAL_SIZE = slots::_MainGeneralCount;
		static const int16 GENERAL_BAGS_BEGIN = 251;
		static const int16 GENERAL_BAGS_END_OFFSET = 79;
		static const int16 GENERAL_BAGS_END = GENERAL_BAGS_BEGIN + GENERAL_BAGS_END_OFFSET;

		static const int16 CURSOR = slots::MainCursor;
		static const int16 CURSOR_BAG_BEGIN = 331;
		static const int16 CURSOR_BAG_END_OFFSET = 9;
		static const int16 CURSOR_BAG_END = CURSOR_BAG_BEGIN + CURSOR_BAG_END_OFFSET;

		static const int16 BANK_BEGIN = 2000;
		static const int16 BANK_END = 2015;
		static const int16 BANK_BAGS_BEGIN = 2031;
		static const int16 BANK_BAGS_END_OFFSET = 159;
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

		static const int16 TRIBUTE_BEGIN = 400;
		static const int16 TRIBUTE_END = 404;

		static const int16 CORPSE_BEGIN = slots::MainGeneral1;
		static const int16 CORPSE_END = slots::MainGeneral1 + slots::MainCursor;

		static const uint16 ITEM_COMMON_SIZE = 5;
		static const uint16 ITEM_CONTAINER_SIZE = 10;

		static const uint32 BANDOLIERS_COUNT = 4;	// count = number of bandolier instances
		static const uint32 BANDOLIER_SIZE = 4;		// size = number of equipment slots in bandolier instance
		static const uint32 POTION_BELT_SIZE = 4;
	}

	namespace limits {
		static const bool ALLOWS_EMPTY_BAG_IN_BAG = false;
		static const bool COIN_HAS_WEIGHT = true;
	}

};	//end namespace Client62

#endif /*CLIENT62_CONSTANTS_H_*/

/*
Client62 Notes:
	** Integer-based inventory **
ok	Possessions: 0 - 30 (Corpse: 22 - 52 [Offset 22])
ok		[Equipment: 0 - 21]
ok		[General: 22 - 29]
ok		[Cursor: 30]
ok	General Bags: 251 - 330
ok	Cursor Bags: 331 - 340

ok	Bank: 2000 - 2015
ok	Bank Bags: 2031 - 2190

ok	Shared Bank: 2500 - 2501
ok	Shared Bank Bags: 2531 - 2550

	Trade: 3000 - 3007
	(Trade Bags: 3031 - 3110 -- server values)

	World: 4000 - 4009

*/
