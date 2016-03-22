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

#ifndef ROF2_CONSTANTS_H_
#define ROF2_CONSTANTS_H_

#include "../types.h"

namespace RoF2 {
	namespace maps {
		typedef enum : int16 {
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
			MapKrono,
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
			MainPowerSource,
			MainAmmo,
			MainGeneral1,
			MainGeneral2,
			MainGeneral3,
			MainGeneral4,
			MainGeneral5,
			MainGeneral6,
			MainGeneral7,
			MainGeneral8,
			MainGeneral9,
			MainGeneral10,
			MainCursor,
			_MainCount,
			_MainEquipmentBegin = MainCharm,
			_MainEquipmentEnd = MainAmmo,
			_MainEquipmentCount = (_MainEquipmentEnd - _MainEquipmentBegin + 1),
			_MainGeneralBegin = MainGeneral1,
			_MainGeneralEnd = MainGeneral10,
			_MainGeneralCount = (_MainGeneralEnd - _MainGeneralBegin + 1)
		} EquipmentSlots;
	}

	namespace consts {
		static const size_t CHARACTER_CREATION_LIMIT = 12;

		static const uint16	MAP_POSSESSIONS_SIZE = slots::_MainCount;
		static const uint16 MAP_BANK_SIZE = 24;
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
		static const uint16 MAP_BAZAAR_SIZE = 200;
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

		// most of these definitions will go away with the structure-based system..this maintains compatibility for now
		// (bag slots and main slots beyond Possessions are assigned for compatibility with current server coding)
		static const int16 EQUIPMENT_BEGIN = slots::MainCharm;
		static const int16 EQUIPMENT_END = slots::MainAmmo;
		static const uint16 EQUIPMENT_SIZE = slots::_MainEquipmentCount;

		static const int16 GENERAL_BEGIN = slots::MainGeneral1;
		static const int16 GENERAL_END = slots::MainGeneral10;
		static const uint16 GENERAL_SIZE = slots::_MainGeneralCount;
		static const int16 GENERAL_BAGS_BEGIN = 251;
		static const int16 GENERAL_BAGS_END_OFFSET = 99;
		static const int16 GENERAL_BAGS_END = GENERAL_BAGS_BEGIN + GENERAL_BAGS_END_OFFSET;

		static const int16 CURSOR = slots::MainCursor;
		static const int16 CURSOR_BAG_BEGIN = 351;
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

		static const int16 TRIBUTE_BEGIN = 400;
		static const int16 TRIBUTE_END = 404;

		static const int16 CORPSE_BEGIN = slots::MainGeneral1;
		static const int16 CORPSE_END = slots::MainGeneral1 + slots::MainCursor;

		static const uint16 ITEM_COMMON_SIZE = 6;
		static const uint16 ITEM_CONTAINER_SIZE = 255; // 255; (server max will be 255..unsure what actual client is - test)

		static const size_t BANDOLIERS_SIZE = 20;		// number of bandolier instances
		static const size_t BANDOLIER_ITEM_COUNT = 4;	// number of equipment slots in bandolier instance

		static const size_t POTION_BELT_ITEM_COUNT = 5;

		static const size_t TEXT_LINK_BODY_LENGTH = 56;

		static const size_t PLAYER_PROFILE_SKILL_MAX = Skill2HPiercing;
	}

	namespace limits {
		static const bool ALLOWS_EMPTY_BAG_IN_BAG = true;
		static const bool ALLOWS_CLICK_CAST_FROM_BAG = true;
		static const bool COIN_HAS_WEIGHT = false;
	}

};	//end namespace RoF2

#endif /*ROF2_CONSTANTS_H_*/

/*
RoF2 Notes:
	** Structure-based inventory **
ok	Possessions: ( 0, { 0 .. 33 }, -1, -1 ) (Corpse: { 23 .. 56 } [Offset 23])
ok		[Equipment: ( 0, { 0 .. 22 }, -1, -1 )]
ok		[General: ( 0, { 23 .. 32 }, -1, -1 )]
ok		[Cursor: ( 0, 33, -1, -1 )]
	General Bags: ( 0, { 23 .. 32 }, { 0 .. (maxsize - 1) }, -1 )
	Cursor Bags: ( 0, 33, { 0 .. (maxsize - 1) }, -1 )

	Bank: ( 1, { 0 .. 23 }, -1, -1 )
	Bank Bags: ( 1, { 0 .. 23 }, { 0 .. (maxsize - 1)}, -1 )

	Shared Bank: ( 2, { 0 .. 1 }, -1, -1 )
	Shared Bank Bags: ( 2, { 0 .. 1 }, { 0 .. (maxsize - 1) }, -1 )

	Trade: ( 3, { 0 .. 8 }, -1, -1 )
	(Trade Bags: 3031 - 3110 -- server values)

	World: ( 4, { 0 .. 10 }, -1, -1 )

*/
