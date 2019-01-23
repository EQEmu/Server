/*	EQEMu: Everquest Server Emulator
	
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

#ifndef COMMON_EMU_CONSTANTS_H
#define COMMON_EMU_CONSTANTS_H

#include "eq_limits.h"
#include "emu_versions.h"

#include <string.h>


// local definitions are the result of using hybrid-client or server-only values and methods
namespace EQEmu
{
	using RoF2::IINVALID;
	using RoF2::INULL;
	
	namespace inventory {
		
	} /*inventory*/

	namespace invtype {
		using namespace RoF2::invtype::enum_;

		using RoF2::invtype::POSSESSIONS_SIZE;
		using RoF2::invtype::BANK_SIZE;
		using RoF2::invtype::SHARED_BANK_SIZE;
		using RoF2::invtype::TRADE_SIZE;
		using RoF2::invtype::WORLD_SIZE;
		using RoF2::invtype::LIMBO_SIZE;
		using RoF2::invtype::TRIBUTE_SIZE;
		using RoF2::invtype::TROPHY_TRIBUTE_SIZE;
		using RoF2::invtype::GUILD_TRIBUTE_SIZE;
		using RoF2::invtype::MERCHANT_SIZE;
		using RoF2::invtype::DELETED_SIZE;
		using RoF2::invtype::CORPSE_SIZE;
		using RoF2::invtype::BAZAAR_SIZE;
		using RoF2::invtype::INSPECT_SIZE;
		using RoF2::invtype::REAL_ESTATE_SIZE;
		using RoF2::invtype::VIEW_MOD_PC_SIZE;
		using RoF2::invtype::VIEW_MOD_BANK_SIZE;
		using RoF2::invtype::VIEW_MOD_SHARED_BANK_SIZE;
		using RoF2::invtype::VIEW_MOD_LIMBO_SIZE;
		using RoF2::invtype::ALT_STORAGE_SIZE;
		using RoF2::invtype::ARCHIVED_SIZE;
		using RoF2::invtype::MAIL_SIZE;
		using RoF2::invtype::GUILD_TROPHY_TRIBUTE_SIZE;
		using RoF2::invtype::KRONO_SIZE;
		using RoF2::invtype::OTHER_SIZE;

		using Titanium::invtype::TRADE_NPC_SIZE;

		using RoF2::invtype::TYPE_INVALID;
		using RoF2::invtype::TYPE_BEGIN;
		using RoF2::invtype::TYPE_END;
		using RoF2::invtype::TYPE_COUNT;

		int16 GetInvTypeSize(int16 inv_type);
		using RoF2::invtype::GetInvTypeName;

	} // namespace invtype

	namespace popupresponse {
		const int32 SERVER_INTERNAL_USE_BASE = 2000000000;
		const int32 MOB_INFO_DISMISS         = 2000000001;
	}

	namespace invslot {
		using namespace RoF2::invslot::enum_;

		using RoF2::invslot::SLOT_INVALID;
		using RoF2::invslot::SLOT_BEGIN;

		using Titanium::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;

		const int16 SLOT_AUGMENT_GENERIC_RETURN = 1001; // clients don't appear to use this method... (internal inventory return value)

		using RoF2::invslot::POSSESSIONS_BEGIN;
		using RoF2::invslot::POSSESSIONS_END;
		using RoF2::invslot::POSSESSIONS_COUNT;

		using RoF2::invslot::EQUIPMENT_BEGIN;
		using RoF2::invslot::EQUIPMENT_END;
		using RoF2::invslot::EQUIPMENT_COUNT;

		using RoF2::invslot::GENERAL_BEGIN;
		using RoF2::invslot::GENERAL_END;
		using RoF2::invslot::GENERAL_COUNT;

		using RoF2::invslot::BONUS_BEGIN;
		using RoF2::invslot::BONUS_STAT_END;
		using RoF2::invslot::BONUS_SKILL_END;

		using Titanium::invslot::BANK_BEGIN;
		using SoF::invslot::BANK_END;

		using Titanium::invslot::SHARED_BANK_BEGIN;
		using Titanium::invslot::SHARED_BANK_END;

		using Titanium::invslot::TRADE_BEGIN;
		using Titanium::invslot::TRADE_END;

		using Titanium::invslot::TRADE_NPC_END;

		using Titanium::invslot::WORLD_BEGIN;
		using Titanium::invslot::WORLD_END;

		using Titanium::invslot::TRIBUTE_BEGIN;
		using Titanium::invslot::TRIBUTE_END;

		using Titanium::invslot::GUILD_TRIBUTE_BEGIN;
		using Titanium::invslot::GUILD_TRIBUTE_END;

		const int16 CORPSE_BEGIN = invslot::slotGeneral1;
		const int16 CORPSE_END = CORPSE_BEGIN + invslot::slotCursor;

		using RoF2::invslot::EQUIPMENT_BITMASK;
		using RoF2::invslot::GENERAL_BITMASK;
		using RoF2::invslot::CURSOR_BITMASK;
		using RoF2::invslot::POSSESSIONS_BITMASK;
		using RoF2::invslot::CORPSE_BITMASK;

		using RoF2::invslot::GetInvPossessionsSlotName;
		using RoF2::invslot::GetInvSlotName;

	} // namespace invslot

	namespace invbag {
		using Titanium::invbag::SLOT_INVALID;
		using Titanium::invbag::SLOT_BEGIN;
		using Titanium::invbag::SLOT_END;
		using Titanium::invbag::SLOT_COUNT;

		using Titanium::invbag::GENERAL_BAGS_BEGIN;
		const int16 GENERAL_BAGS_COUNT = invslot::GENERAL_COUNT * SLOT_COUNT;
		const int16 GENERAL_BAGS_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_COUNT) - 1;

		const int16 GENERAL_BAGS_8_COUNT = 8 * SLOT_COUNT;
		const int16 GENERAL_BAGS_8_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_8_COUNT) - 1;

		const int16 CURSOR_BAG_BEGIN = 351;
		const int16 CURSOR_BAG_COUNT = SLOT_COUNT;
		const int16 CURSOR_BAG_END = (CURSOR_BAG_BEGIN + CURSOR_BAG_COUNT) - 1;

		using Titanium::invbag::BANK_BAGS_BEGIN;
		const int16 BANK_BAGS_COUNT = (invtype::BANK_SIZE * SLOT_COUNT);
		const int16 BANK_BAGS_END = (BANK_BAGS_BEGIN + BANK_BAGS_COUNT) - 1;

		const int16 BANK_BAGS_16_COUNT = 16 * SLOT_COUNT;
		const int16 BANK_BAGS_16_END = (BANK_BAGS_BEGIN + BANK_BAGS_16_COUNT) - 1;

		using Titanium::invbag::SHARED_BANK_BAGS_BEGIN;
		const int16 SHARED_BANK_BAGS_COUNT = invtype::SHARED_BANK_SIZE * SLOT_COUNT;
		const int16 SHARED_BANK_BAGS_END = (SHARED_BANK_BAGS_BEGIN + SHARED_BANK_BAGS_COUNT) - 1;

		using Titanium::invbag::TRADE_BAGS_BEGIN;
		const int16 TRADE_BAGS_COUNT = invtype::TRADE_SIZE * SLOT_COUNT;
		const int16 TRADE_BAGS_END = (TRADE_BAGS_BEGIN + TRADE_BAGS_COUNT) - 1;

		using Titanium::invbag::GetInvBagIndexName;

	} // namespace invbag

	namespace invaug {
		using RoF2::invaug::SOCKET_INVALID;
		using RoF2::invaug::SOCKET_BEGIN;
		using RoF2::invaug::SOCKET_END;
		using RoF2::invaug::SOCKET_COUNT;

		using RoF2::invaug::GetInvAugIndexName;

	} // namespace invaug

	namespace constants {
		const EQEmu::versions::ClientVersion CHARACTER_CREATION_CLIENT = EQEmu::versions::ClientVersion::Titanium;

		using RoF2::constants::EXPANSION;
		using RoF2::constants::EXPANSION_BIT;
		using RoF2::constants::EXPANSIONS_MASK;

		using RoF2::constants::CHARACTER_CREATION_LIMIT;
		
		const size_t SAY_LINK_OPENER_SIZE = 1;
		using RoF2::constants::SAY_LINK_BODY_SIZE;
		const size_t SAY_LINK_TEXT_SIZE = 256; // this may be varied until it breaks something (tested:374) - the others are constant
		const size_t SAY_LINK_CLOSER_SIZE = 1;
		const size_t SAY_LINK_MAXIMUM_SIZE = (SAY_LINK_OPENER_SIZE + SAY_LINK_BODY_SIZE + SAY_LINK_TEXT_SIZE + SAY_LINK_CLOSER_SIZE);

	} /*constants*/

	namespace profile {
		using RoF2::profile::BANDOLIERS_SIZE;
		using RoF2::profile::BANDOLIER_ITEM_COUNT;
		
		using RoF2::profile::POTION_BELT_SIZE;

		using RoF2::profile::SKILL_ARRAY_SIZE;

	} // namespace profile

	namespace behavior {
		using RoF2::behavior::CoinHasWeight;

	} // namespace behavior

	namespace spells {
		enum class CastingSlot : uint32 { // hybrid declaration
			Gem1 = 0,
			Gem2 = 1,
			Gem3 = 2,
			Gem4 = 3,
			Gem5 = 4,
			Gem6 = 5,
			Gem7 = 6,
			Gem8 = 7,
			Gem9 = 8,
			Gem10 = 9,
			Gem11 = 10,
			Gem12 = 11,
			MaxGems = 12,
			Ability = 20, // HT/LoH for Tit
			PotionBelt = 21, // Tit uses a different slot for PB
			Item = 22,
			Discipline = 23,
			AltAbility = 0xFF
		};

		using RoF2::spells::SPELL_ID_MAX;
		using SoD::spells::SPELLBOOK_SIZE;
		using UF::spells::SPELL_GEM_COUNT; // RoF+ clients define more than UF client..but, they are not valid beyond UF

		using RoF2::spells::LONG_BUFFS;
		using RoF2::spells::SHORT_BUFFS;
		using RoF2::spells::DISC_BUFFS;
		using RoF2::spells::TOTAL_BUFFS;
		using RoF2::spells::NPC_BUFFS;
		using RoF2::spells::PET_BUFFS;
		using RoF2::spells::MERC_BUFFS;

	} // namespace spells

	namespace bug {
		enum CategoryID : uint32 {
			catOther = 0,
			catVideo,
			catAudio,
			catPathing,
			catQuest,
			catTradeskills,
			catSpellStacking,
			catDoorsPortals,
			catItems,
			catNPC,
			catDialogs,
			catLoNTCG,
			catMercenaries
		};

		enum OptionalInfoFlag : uint32 {
			infoNoOptionalInfo = 0x0,
			infoCanDuplicate = 0x1,
			infoCrashBug = 0x2,
			infoTargetInfo = 0x4,
			infoCharacterFlags = 0x8,
			infoUnknownValue = 0xFFFFFFF0
		};

		const char* CategoryIDToCategoryName(CategoryID category_id);
		CategoryID CategoryNameToCategoryID(const char* category_name);

	} // namespace bug

} /*EQEmu*/

#endif /*COMMON_EMU_CONSTANTS_H*/

/*	hack list to prevent circular references
	
	eq_limits.h:EQEmu::inventory::LookupEntry::InventoryTypeSize[n];

*/
