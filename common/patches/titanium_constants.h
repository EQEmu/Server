/*
EQEMu:  Everquest Server Emulator

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

#ifndef TITANIUM_CONSTANTS_H_
#define TITANIUM_CONSTANTS_H_

#include "../types.h"

namespace Titanium {
	namespace inventory {
		typedef enum : int16 {
			TypePossessions = 0,
			TypeBank,
			TypeSharedBank,
			TypeTrade,
			TypeWorld,
			TypeLimbo,
			TypeTribute,
			TypeTrophyTribute,
			TypeGuildTribute,
			TypeMerchant,
			TypeDeleted,
			TypeCorpse,
			TypeBazaar,
			TypeInspect,
			TypeRealEstate,
			TypeViewMODPC,
			TypeViewMODBank,
			TypeViewMODSharedBank,
			TypeViewMODLimbo,
			TypeAltStorage,
			TypeArchived,
			TypeMail,
			TypeGuildTrophyTribute,
			TypeOther,
			TypeCount
		} InventoryTypes;

		typedef enum : int16 {
			SlotCharm = 0,
			SlotEar1,
			SlotHead,
			SlotFace,
			SlotEar2,
			SlotNeck,
			SlotShoulders,
			SlotArms,
			SlotBack,
			SlotWrist1,
			SlotWrist2,
			SlotRange,
			SlotHands,
			SlotPrimary,
			SlotSecondary,
			SlotFinger1,
			SlotFinger2,
			SlotChest,
			SlotLegs,
			SlotFeet,
			SlotWaist,
			SlotAmmo,
			SlotGeneral1,
			SlotGeneral2,
			SlotGeneral3,
			SlotGeneral4,
			SlotGeneral5,
			SlotGeneral6,
			SlotGeneral7,
			SlotGeneral8,
			SlotCursor,
			SlotCount,
			SlotEquipmentBegin = SlotCharm,
			SlotEquipmentEnd = SlotAmmo,
			SlotEquipmentCount = (SlotEquipmentEnd - SlotEquipmentBegin + 1),
			SlotGeneralBegin = SlotGeneral1,
			SlotGeneralEnd = SlotGeneral8,
			SlotGeneralCount = (SlotGeneralEnd - SlotGeneralBegin + 1)
		} PossessionsSlots;
	}

	namespace consts {
		static const size_t CHARACTER_CREATION_LIMIT = 8; // Hard-coded in client - DO NOT ALTER

		static const uint16	TYPE_POSSESSIONS_SIZE = inventory::SlotCount;
		static const uint16 TYPE_BANK_SIZE = 16;
		static const uint16 TYPE_SHARED_BANK_SIZE = 2;
		static const uint16 TYPE_TRADE_SIZE = 8;
		static const uint16 TYPE_WORLD_SIZE = 10;
		static const uint16 TYPE_LIMBO_SIZE = 36;
		static const uint16 TYPE_TRIBUTE_SIZE = 0; //?
		static const uint16 TYPE_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_GUILD_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_MERCHANT_SIZE = 0;
		static const uint16 TYPE_DELETED_SIZE = 0;
		static const uint16 TYPE_CORPSE_SIZE = inventory::SlotCount;
		static const uint16 TYPE_BAZAAR_SIZE = 80;
		static const uint16 TYPE_INSPECT_SIZE = inventory::SlotEquipmentCount;
		static const uint16 TYPE_REAL_ESTATE_SIZE = 0;
		static const uint16 TYPE_VIEW_MOD_PC_SIZE = TYPE_POSSESSIONS_SIZE;
		static const uint16 TYPE_VIEW_MOD_BANK_SIZE = TYPE_BANK_SIZE;
		static const uint16 TYPE_VIEW_MOD_SHARED_BANK_SIZE = TYPE_SHARED_BANK_SIZE;
		static const uint16 TYPE_VIEW_MOD_LIMBO_SIZE = TYPE_LIMBO_SIZE;
		static const uint16 TYPE_ALT_STORAGE_SIZE = 0;
		static const uint16 TYPE_ARCHIVED_SIZE = 0;
		static const uint16 TYPE_MAIL_SIZE = 0;
		static const uint16 TYPE_GUILD_TROPHY_TRIBUTE_SIZE = 0;
		static const uint16 TYPE_KRONO_SIZE = NOT_USED;
		static const uint16 TYPE_OTHER_SIZE = 0;

		static const int16 EQUIPMENT_BEGIN = inventory::SlotCharm;
		static const int16 EQUIPMENT_END = inventory::SlotAmmo;
		static const uint16 EQUIPMENT_SIZE = inventory::SlotEquipmentCount;

		static const int16 GENERAL_BEGIN = inventory::SlotGeneral1;
		static const int16 GENERAL_END = inventory::SlotGeneral8;
		static const uint16 GENERAL_SIZE = inventory::SlotGeneralCount;
		static const int16 GENERAL_BAGS_BEGIN = 251;
		static const int16 GENERAL_BAGS_END_OFFSET = 79;
		static const int16 GENERAL_BAGS_END = GENERAL_BAGS_BEGIN + GENERAL_BAGS_END_OFFSET;

		static const int16 CURSOR = inventory::SlotCursor;
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

		static const int16 CORPSE_BEGIN = inventory::SlotGeneral1;
		static const int16 CORPSE_END = inventory::SlotGeneral1 + inventory::SlotCursor;

		static const uint16 ITEM_COMMON_SIZE = 5;
		static const uint16 ITEM_CONTAINER_SIZE = 10;

		static const size_t BANDOLIERS_SIZE = 4;		// number of bandolier instances
		static const size_t BANDOLIER_ITEM_COUNT = 4;	// number of equipment slots in bandolier instance

		static const size_t POTION_BELT_ITEM_COUNT = 4;

		static const size_t TEXT_LINK_BODY_LENGTH = 45;

		static const size_t PLAYER_PROFILE_SKILL_MAX = SkillFrenzy;
	}

	namespace limits {
		static const bool ALLOWS_EMPTY_BAG_IN_BAG = false;
		static const bool ALLOWS_CLICK_CAST_FROM_BAG = false;
		static const bool COIN_HAS_WEIGHT = true;
	}

};	//end namespace Titanium

#endif /*TITANIUM_CONSTANTS_H_*/

/*
Titanium Notes:
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
