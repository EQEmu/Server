/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_ROF2_LIMITS_H
#define COMMON_ROF2_LIMITS_H

#include "../types.h"
#include "../emu_versions.h"
#include "../skills.h"


namespace RoF2
{
	enum : int { Invalid = -1, Null, Safety };

	enum : bool { False = false, True = true };

	// pre-declarations
	namespace inventory {
		inline EQEmu::versions::ClientVersion GetInventoryRef() { return EQEmu::versions::ClientVersion::RoF2; }

	} /*inventory*/

	namespace invtype {
		inline EQEmu::versions::ClientVersion GetInvTypeRef() { return EQEmu::versions::ClientVersion::RoF2; }

		enum : int { InvTypeInvalid = -1, InvTypeBegin };

		enum InventoryType : int {
			InvTypePossessions = InvTypeBegin,
			InvTypeBank,
			InvTypeSharedBank,
			InvTypeTrade,
			InvTypeWorld,
			InvTypeLimbo,
			InvTypeTribute,
			InvTypeTrophyTribute,
			InvTypeGuildTribute,
			InvTypeMerchant,
			InvTypeDeleted,
			InvTypeCorpse,
			InvTypeBazaar,
			InvTypeInspect,
			InvTypeRealEstate,
			InvTypeViewMODPC,
			InvTypeViewMODBank,
			InvTypeViewMODSharedBank,
			InvTypeViewMODLimbo,
			InvTypeAltStorage,
			InvTypeArchived,
			InvTypeMail,
			InvTypeGuildTrophyTribute,
			InvTypeKrono,
			InvTypeOther,
			InvTypeCount
		};

	} /*invtype*/

	namespace invslot {
		inline EQEmu::versions::ClientVersion GetInvSlotRef() { return EQEmu::versions::ClientVersion::RoF2; }

		enum : int { InvSlotInvalid = -1, InvSlotBegin };

		enum PossessionsSlot : int {
			PossessionsCharm = InvSlotBegin,
			PossessionsEar1,
			PossessionsHead,
			PossessionsFace,
			PossessionsEar2,
			PossessionsNeck,
			PossessionsShoulders,
			PossessionsArms,
			PossessionsBack,
			PossessionsWrist1,
			PossessionsWrist2,
			PossessionsRange,
			PossessionsHands,
			PossessionsPrimary,
			PossessionsSecondary,
			PossessionsFinger1,
			PossessionsFinger2,
			PossessionsChest,
			PossessionsLegs,
			PossessionsFeet,
			PossessionsWaist,
			PossessionsPowerSource,
			PossessionsAmmo,
			PossessionsGeneral1,
			PossessionsGeneral2,
			PossessionsGeneral3,
			PossessionsGeneral4,
			PossessionsGeneral5,
			PossessionsGeneral6,
			PossessionsGeneral7,
			PossessionsGeneral8,
			PossessionsGeneral9,
			PossessionsGeneral10,
			PossessionsCursor,
			PossessionsCount
		};

		const int EquipmentBegin = PossessionsCharm;
		const int EquipmentEnd = PossessionsAmmo;
		const int EquipmentCount = (EquipmentEnd - EquipmentBegin + 1);

		const int GeneralBegin = PossessionsGeneral1;
		const int GeneralEnd = PossessionsGeneral10;
		const int GeneralCount = (GeneralEnd - GeneralBegin + 1);

	} /*invslot*/

	namespace invbag {
		inline EQEmu::versions::ClientVersion GetInvBagRef() { return EQEmu::versions::ClientVersion::RoF2; }

		enum : int { InvBagInvalid = -1, InvBagBegin };

	} /*invbag*/

	namespace invaug {
		inline EQEmu::versions::ClientVersion GetInvAugRef() { return EQEmu::versions::ClientVersion::RoF2; }

		enum : int { InvAugInvalid = -1, InvAugBegin };

	} /*invaug*/

	namespace item {
		inline EQEmu::versions::ClientVersion GetItemRef() { return EQEmu::versions::ClientVersion::RoF2; }

		//enum Unknown : int { // looks like item class..but, RoF has it too - nothing in UF-
		//	Unknown1 = 0,
		//	Unknown2 = 1,
		//	Unknown3 = 2,
		//	Unknown4 = 5 // krono?
		//};

		enum ItemPacketType : int {
			ItemPacketMerchant = 100,
			ItemPacketTradeView = 101,
			ItemPacketLoot = 102,
			ItemPacketTrade = 103,
			ItemPacketCharInventory = 105,
			ItemPacketLimbo = 106,
			ItemPacketWorldContainer = 107,
			ItemPacketTributeItem = 108,
			ItemPacketGuildTribute = 109,
			ItemPacket10 = 110,
			ItemPacket11 = 111,
			ItemPacket12 = 112,
			ItemPacketRecovery = 113,
			ItemPacket14 = 115 // Parcel? adds to merchant window too
		};

	} /*item*/

	namespace profile {
		inline EQEmu::versions::ClientVersion GetProfileRef() { return EQEmu::versions::ClientVersion::RoF2; }

	} /*profile*/

	namespace constants {
		inline EQEmu::versions::ClientVersion GetConstantsRef() { return EQEmu::versions::ClientVersion::RoF2; }

	} /*constants*/

	namespace behavior {
		inline EQEmu::versions::ClientVersion GetBehaviorRef() { return EQEmu::versions::ClientVersion::RoF2; }

	} /*behavior*/

	namespace skills {
		inline EQEmu::versions::ClientVersion GetSkillsRef() { return EQEmu::versions::ClientVersion::RoF2; }

	} /*skills*/


	// declarations
	namespace inventory {
		const bool ConcatenateInvTypeLimbo = false;

		const bool AllowOverLevelEquipment = true;

		const bool AllowEmptyBagInBag = true;
		const bool AllowClickCastFromBag = true;

	} /*inventory*/

	namespace invtype {
		const size_t InvTypePossessionsSize = invslot::PossessionsCount;
		const size_t InvTypeBankSize = 24;
		const size_t InvTypeSharedBankSize = 2;
		const size_t InvTypeTradeSize = 8;
		const size_t InvTypeWorldSize = 10;
		const size_t InvTypeLimboSize = 36;
		const size_t InvTypeTributeSize = 5;
		const size_t InvTypeTrophyTributeSize = 0;//unknown
		const size_t InvTypeGuildTributeSize = 2;//unverified
		const size_t InvTypeMerchantSize = 200;
		const size_t InvTypeDeletedSize = 0;//unknown - "Recovery Tab"
		const size_t InvTypeCorpseSize = InvTypePossessionsSize;
		const size_t InvTypeBazaarSize = 200;
		const size_t InvTypeInspectSize = invslot::EquipmentCount;
		const size_t InvTypeRealEstateSize = 0;//unknown
		const size_t InvTypeViewMODPCSize = InvTypePossessionsSize;
		const size_t InvTypeViewMODBankSize = InvTypeBankSize;
		const size_t InvTypeViewMODSharedBankSize = InvTypeSharedBankSize;
		const size_t InvTypeViewMODLimboSize = InvTypeLimboSize;
		const size_t InvTypeAltStorageSize = 0;//unknown - "Shroud Bank"
		const size_t InvTypeArchivedSize = 0;//unknown
		const size_t InvTypeMailSize = 0;//unknown
		const size_t InvTypeGuildTrophyTributeSize = 0;//unknown
		const size_t InvTypeKronoSize = 0;//unknown
		const size_t InvTypeOtherSize = 0;//unknown

		extern size_t GetInvTypeSize(int inv_type);
		extern const char* GetInvTypeName(int inv_type);

		extern bool IsInvTypePersistent(int inv_type);

	} /*invtype*/

	namespace invslot {
		extern const char* GetInvPossessionsSlotName(int inv_slot);
		extern const char* GetInvSlotName(int inv_type, int inv_slot);

	} /*invslot*/

	namespace invbag {
		const size_t ItemBagSize = 255; // server Size will be 255..unsure what actual client is (test)

		extern const char* GetInvBagIndexName(int bag_index);

	} /*invbag*/

	namespace invaug {
		const size_t ItemAugSize = 6;

		extern const char* GetInvAugIndexName(int aug_index);

	} /*invaug*/

	namespace item {

	} /*item*/

	namespace profile {
		const size_t TributeSize = invtype::InvTypeTributeSize;
		const size_t GuildTributeSize = invtype::InvTypeGuildTributeSize;

		const size_t BandoliersSize = 20;		// number of bandolier instances
		const size_t BandolierItemCount = 4;	// number of equipment slots in bandolier instance

		const size_t PotionBeltSize = 5;

		const size_t SkillArraySize = 100;

	} /*profile*/

	namespace constants {
		const size_t CharacterCreationLimit = 12;

		const size_t SayLinkBodySize = 56;

		const int LongBuffs = 42;
		const int ShortBuffs = 20;
		const int DiscBuffs = 1;
		const int TotalBuffs = LongBuffs + ShortBuffs + DiscBuffs;
		const int NPCBuffs = 97;
		const int PetBuffs = NPCBuffs;
		const int MercBuffs = LongBuffs;

	} /*constants*/

	namespace behavior {
		const bool CoinHasWeight = false;

	} /*behavior*/

	namespace skills {
		const size_t LastUsableSkill = EQEmu::skills::Skill2HPiercing;

	} /*skills*/

}; /*RoF2*/

#endif /*COMMON_ROF2_LIMITS_H*/
