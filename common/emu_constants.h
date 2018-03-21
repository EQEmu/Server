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
#include "emu_legacy.h"
#include "emu_versions.h"

#include <string.h>


namespace EQEmu
{
	namespace inventory {
		//using namespace RoF2::invtype;
		//using namespace RoF2::invslot;
		//using namespace RoF2::invbag;
		//using namespace RoF2::invaug;

		enum : int16 { typeInvalid = -1, slotInvalid = -1, containerInvalid = -1, socketInvalid = -1 }; // temporary
		enum : int16 { typeBegin = 0, slotBegin = 0, containerBegin = 0, socketBegin = 0 }; // temporary

		enum PossessionsSlots : int16 { // temporary
			slotCharm = 0,
			slotEar1,
			slotHead,
			slotFace,
			slotEar2,
			slotNeck, // 5
			slotShoulders,
			slotArms,
			slotBack,
			slotWrist1,
			slotWrist2, // 10
			slotRange,
			slotHands,
			slotPrimary,
			slotSecondary,
			slotFinger1, // 15
			slotFinger2,
			slotChest,
			slotLegs,
			slotFeet,
			slotWaist, // 20
			slotPowerSource = 9999,
			slotAmmo = 21,
			slotGeneral1,
			slotGeneral2,
			slotGeneral3,
			slotGeneral4, // 25
			slotGeneral5,
			slotGeneral6,
			slotGeneral7,
			slotGeneral8,
			slotCursor, // 30
			slotCount
		};

		enum InventoryTypes : int16 { // temporary
			typePossessions = 0,
			typeBank,
			typeSharedBank,
			typeTrade,
			typeWorld,
			typeLimbo, // 5
			typeTribute,
			typeTrophyTribute,
			typeGuildTribute,
			typeMerchant,
			typeDeleted, // 10
			typeCorpse,
			typeBazaar,
			typeInspect,
			typeRealEstate,
			typeViewMODPC, // 15
			typeViewMODBank,
			typeViewMODSharedBank,
			typeViewMODLimbo,
			typeAltStorage,
			typeArchived, // 20
			typeMail,
			typeGuildTrophyTribute,
			typeKrono,
			typeOther,
			typeCount
		};

		static int16 SlotCount(int16 type_index) { return 0; } // temporary

		const int16 ContainerCount = 10; // temporary
		const int16 SocketCount = 6; // temporary

	} /*inventory*/

	namespace constants {
		const EQEmu::versions::ClientVersion CharacterCreationClient = EQEmu::versions::ClientVersion::RoF2;
		const size_t CharacterCreationMax = RoF2::constants::CharacterCreationLimit;

		const size_t SayLinkOpenerSize = 1;
		const size_t SayLinkBodySize = RoF2::constants::SayLinkBodySize;
		const size_t SayLinkTextSize = 256; // this may be varied until it breaks something (tested:374) - the others are constant
		const size_t SayLinkCloserSize = 1;
		const size_t SayLinkMaximumSize = (SayLinkOpenerSize + SayLinkBodySize + SayLinkTextSize + SayLinkCloserSize);

		const int LongBuffs = RoF2::constants::LongBuffs;
		const int ShortBuffs = RoF2::constants::ShortBuffs;
		const int DiscBuffs = RoF2::constants::DiscBuffs;
		const int TotalBuffs = RoF2::constants::TotalBuffs;
		const int NPCBuffs = RoF2::constants::NPCBuffs;
		const int PetBuffs = RoF2::constants::PetBuffs;
		const int MercBuffs = RoF2::constants::MercBuffs;

	} /*constants*/

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

	enum class CastingSlot : uint32 {
		Gem1       = 0,
		Gem2       = 1,
		Gem3       = 2,
		Gem4       = 3,
		Gem5       = 4,
		Gem6       = 5,
		Gem7       = 6,
		Gem8       = 7,
		Gem9       = 8,
		Gem10      = 9,
		Gem11      = 10,
		Gem12      = 11,
		MaxGems    = 12,
		Ability    = 20, // HT/LoH for Tit
		PotionBelt = 21, // Tit uses a different slot for PB
		Item       = 22,
		Discipline = 23,
		AltAbility = 0xFF
	};

} /*EQEmu*/

#endif /*COMMON_EMU_CONSTANTS_H*/

/*	hack list to prevent circular references
	
	eq_limits.h:EQEmu::inventory::LookupEntry::InventoryTypeSize[n];

*/
