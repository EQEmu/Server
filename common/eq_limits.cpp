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

#include "emu_constants.h"
#include "emu_limits.h"


static const EQEmu::constants::LookupEntry constants_lookup_entries[EQEmu::versions::ClientVersionCount] =
{
	{
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null
	},
	{
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null
	},
	{
		Titanium::constants::CharacterCreationLimit,
		Titanium::constants::LongBuffs,
		Titanium::constants::ShortBuffs,
		Titanium::constants::DiscBuffs,
		Titanium::constants::TotalBuffs,
		Titanium::constants::NPCBuffs,
		Titanium::constants::PetBuffs,
		Titanium::constants::MercBuffs
	},
	{
		SoF::constants::CharacterCreationLimit,
		SoF::constants::LongBuffs,
		SoF::constants::ShortBuffs,
		SoF::constants::DiscBuffs,
		SoF::constants::TotalBuffs,
		SoF::constants::NPCBuffs,
		SoF::constants::PetBuffs,
		SoF::constants::MercBuffs
	},
	{
		SoD::constants::CharacterCreationLimit,
		SoD::constants::LongBuffs,
		SoD::constants::ShortBuffs,
		SoD::constants::DiscBuffs,
		SoD::constants::TotalBuffs,
		SoD::constants::NPCBuffs,
		SoD::constants::PetBuffs,
		SoD::constants::MercBuffs
	},
	{
		UF::constants::CharacterCreationLimit,
		UF::constants::LongBuffs,
		UF::constants::ShortBuffs,
		UF::constants::DiscBuffs,
		UF::constants::TotalBuffs,
		UF::constants::NPCBuffs,
		UF::constants::PetBuffs,
		UF::constants::MercBuffs
	},
	{
		RoF::constants::CharacterCreationLimit,
		RoF::constants::LongBuffs,
		RoF::constants::ShortBuffs,
		RoF::constants::DiscBuffs,
		RoF::constants::TotalBuffs,
		RoF::constants::NPCBuffs,
		RoF::constants::PetBuffs,
		RoF::constants::MercBuffs
	},
	{
		RoF2::constants::CharacterCreationLimit,
		RoF2::constants::LongBuffs,
		RoF2::constants::ShortBuffs,
		RoF2::constants::DiscBuffs,
		RoF2::constants::TotalBuffs,
		RoF2::constants::NPCBuffs,
		RoF2::constants::PetBuffs,
		RoF2::constants::MercBuffs
	}
};

const EQEmu::constants::LookupEntry* EQEmu::constants::Lookup(versions::ClientVersion client_version)
{
	return &constants_lookup_entries[static_cast<int>(versions::ValidateClientVersion(client_version))];
}

static const EQEmu::inventory::LookupEntry inventory_lookup_entries[EQEmu::versions::MobVersionCount] =
{
	{
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,

		ClientUnknown::Null,
		ClientUnknown::Null,
		ClientUnknown::Null,

		ClientUnknown::False,
		ClientUnknown::False,
		ClientUnknown::False,
		ClientUnknown::False
	},
	{
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,
		Client62::Null,

		Client62::Null,
		Client62::Null,
		Client62::Null,

		Client62::False,
		Client62::False,
		Client62::False,
		Client62::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*Titanium::invtype::InvTypePossessionsSize,*/
		Titanium::invtype::InvTypeBankSize,
		Titanium::invtype::InvTypeSharedBankSize,
		Titanium::invtype::InvTypeTradeSize,
		Titanium::invtype::InvTypeWorldSize,
		Titanium::invtype::InvTypeLimboSize,
		Titanium::invtype::InvTypeTributeSize,
		Titanium::Null,
		Titanium::Null, /*Titanium::invtype::InvTypeGuildTributeSize,*/
		Titanium::invtype::InvTypeMerchantSize,
		Titanium::Null,
		Titanium::invtype::InvTypeCorpseSize,
		EQEmu::legacy::TYPE_BAZAAR_SIZE, /*Titanium::invtype::InvTypeBazaarSize,*/
		Titanium::invtype::InvTypeInspectSize,
		Titanium::Null,
		Titanium::invtype::InvTypeViewMODPCSize,
		Titanium::invtype::InvTypeViewMODBankSize,
		Titanium::invtype::InvTypeViewMODSharedBankSize,
		Titanium::invtype::InvTypeViewMODLimboSize,
		Titanium::invtype::InvTypeAltStorageSize,
		Titanium::invtype::InvTypeArchivedSize,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::invtype::InvTypeOtherSize,

		Titanium::Null, /*0x000000027FDFFFFF,*/
		EQEmu::inventory::ContainerCount, /*Titanium::invbag::ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*Titanium::invaug::ItemAugSize,*/

		Titanium::inventory::AllowEmptyBagInBag,
		Titanium::inventory::AllowClickCastFromBag,
		Titanium::inventory::ConcatenateInvTypeLimbo,
		Titanium::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*SoF::invtype::InvTypePossessionsSize,*/
		SoF::invtype::InvTypeBankSize,
		SoF::invtype::InvTypeSharedBankSize,
		SoF::invtype::InvTypeTradeSize,
		SoF::invtype::InvTypeWorldSize,
		SoF::invtype::InvTypeLimboSize,
		SoF::invtype::InvTypeTributeSize,
		SoF::Null,
		SoF::Null, /*SoF::invtype::InvTypeGuildTributeSize,*/
		SoF::invtype::InvTypeMerchantSize,
		SoF::Null,
		SoF::invtype::InvTypeCorpseSize,
		EQEmu::legacy::TYPE_BAZAAR_SIZE, /*SoF::invtype::InvTypeBazaarSize,*/
		SoF::invtype::InvTypeInspectSize,
		SoF::Null,
		SoF::invtype::InvTypeViewMODPCSize,
		SoF::invtype::InvTypeViewMODBankSize,
		SoF::invtype::InvTypeViewMODSharedBankSize,
		SoF::invtype::InvTypeViewMODLimboSize,
		SoF::invtype::InvTypeAltStorageSize,
		SoF::invtype::InvTypeArchivedSize, SoF::Null, SoF::Null, SoF::Null, SoF::invtype::InvTypeOtherSize,

		SoF::Null, /*0x000000027FFFFFFF,*/
		EQEmu::inventory::ContainerCount, /*SoF::invbag::ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*SoF::invaug::ItemAugSize,*/

		SoF::inventory::AllowEmptyBagInBag,
		SoF::inventory::AllowClickCastFromBag,
		SoF::inventory::ConcatenateInvTypeLimbo,
		SoF::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*SoD::invtype::InvTypePossessionsSize,*/
		SoD::invtype::InvTypeBankSize,
		SoD::invtype::InvTypeSharedBankSize,
		SoD::invtype::InvTypeTradeSize,
		SoD::invtype::InvTypeWorldSize,
		SoD::invtype::InvTypeLimboSize,
		SoD::invtype::InvTypeTributeSize,
		SoD::Null,
		SoD::Null, /*SoD::invtype::InvTypeGuildTributeSize,*/
		SoD::invtype::InvTypeMerchantSize,
		SoD::Null,
		SoD::invtype::InvTypeCorpseSize,
		EQEmu::legacy::TYPE_BAZAAR_SIZE, /*SoD::invtype::InvTypeBazaarSize,*/
		SoD::invtype::InvTypeInspectSize,
		SoD::Null,
		SoD::invtype::InvTypeViewMODPCSize,
		SoD::invtype::InvTypeViewMODBankSize,
		SoD::invtype::InvTypeViewMODSharedBankSize,
		SoD::invtype::InvTypeViewMODLimboSize,
		SoD::invtype::InvTypeAltStorageSize,
		SoD::invtype::InvTypeArchivedSize,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::invtype::InvTypeOtherSize,

		SoD::Null, /*0x000000027FFFFFFF,*/
		EQEmu::inventory::ContainerCount, /*SoD::invbag::ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*SoD::invaug::ItemAugSize,*/

		SoD::inventory::AllowEmptyBagInBag,
		SoD::inventory::AllowClickCastFromBag,
		SoD::inventory::ConcatenateInvTypeLimbo,
		SoD::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*UF::invtype::InvTypePossessionsSize,*/
		UF::invtype::InvTypeBankSize,
		UF::invtype::InvTypeSharedBankSize,
		UF::invtype::InvTypeTradeSize,
		UF::invtype::InvTypeWorldSize,
		UF::invtype::InvTypeLimboSize,
		UF::invtype::InvTypeTributeSize,
		UF::Null,
		UF::Null, /*UF::invtype::InvTypeGuildTributeSize,*/
		UF::invtype::InvTypeMerchantSize,
		UF::Null,
		UF::invtype::InvTypeCorpseSize,
		EQEmu::legacy::TYPE_BAZAAR_SIZE, /*UF::invtype::InvTypeBazaarSize,*/
		UF::invtype::InvTypeInspectSize,
		UF::Null,
		UF::invtype::InvTypeViewMODPCSize,
		UF::invtype::InvTypeViewMODBankSize,
		UF::invtype::InvTypeViewMODSharedBankSize,
		UF::invtype::InvTypeViewMODLimboSize,
		UF::invtype::InvTypeAltStorageSize,
		UF::invtype::InvTypeArchivedSize,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::invtype::InvTypeOtherSize,

		UF::Null, /*0x000000027FFFFFFF,*/
		EQEmu::inventory::ContainerCount, /*UF::invbag::ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*UF::invaug::ItemAugSize,*/

		UF::inventory::AllowEmptyBagInBag,
		UF::inventory::AllowClickCastFromBag,
		UF::inventory::ConcatenateInvTypeLimbo,
		UF::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*RoF::invtype::InvTypePossessionsSize,*/
		RoF::invtype::InvTypeBankSize,
		RoF::invtype::InvTypeSharedBankSize,
		RoF::invtype::InvTypeTradeSize,
		RoF::invtype::InvTypeWorldSize,
		RoF::invtype::InvTypeLimboSize,
		RoF::invtype::InvTypeTributeSize,
		RoF::Null, /*RoF::invtype::InvTypeTrophyTributeSize,*/
		RoF::Null, /*RoF::invtype::InvTypeGuildTributeSize,*/
		RoF::invtype::InvTypeMerchantSize,
		RoF::Null, /*RoF::invtype::InvTypeDeletedSize,*/
		RoF::invtype::InvTypeCorpseSize,
		EQEmu::legacy::TYPE_BAZAAR_SIZE, /*RoF::invtype::InvTypeBazaarSize,*/
		RoF::invtype::InvTypeInspectSize,
		RoF::Null, /*RoF::invtype::InvTypeRealEstateSize,*/
		RoF::invtype::InvTypeViewMODPCSize,
		RoF::invtype::InvTypeViewMODBankSize,
		RoF::invtype::InvTypeViewMODSharedBankSize,
		RoF::invtype::InvTypeViewMODLimboSize,
		RoF::invtype::InvTypeAltStorageSize,
		RoF::invtype::InvTypeArchivedSize,
		RoF::invtype::InvTypeMailSize,
		RoF::invtype::InvTypeGuildTrophyTributeSize,
		RoF::Null,
		RoF::invtype::InvTypeOtherSize,

		RoF::Null, /*0x00000003FFFFFFFF,*/
		EQEmu::inventory::ContainerCount, /*RoF::invbag::ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*RoF::invaug::ItemAugSize,*/

		RoF::False, /*RoF::inventory::AllowEmptyBagInBag,*/
		RoF::inventory::AllowClickCastFromBag,
		RoF::inventory::ConcatenateInvTypeLimbo,
		RoF::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*RoF2::invtype::InvTypePossessionsSize,*/
		RoF2::invtype::InvTypeBankSize,
		RoF2::invtype::InvTypeSharedBankSize,
		RoF2::invtype::InvTypeTradeSize,
		RoF2::invtype::InvTypeWorldSize,
		RoF2::invtype::InvTypeLimboSize,
		RoF2::invtype::InvTypeTributeSize,
		RoF2::Null, /*RoF2::invtype::InvTypeTrophyTributeSize,*/
		RoF2::Null, /*RoF2::invtype::InvTypeGuildTributeSize,*/
		RoF2::invtype::InvTypeMerchantSize,
		RoF2::Null, /*RoF2::invtype::InvTypeDeletedSize,*/
		RoF2::invtype::InvTypeCorpseSize,
		EQEmu::legacy::TYPE_BAZAAR_SIZE, /*RoF2::invtype::InvTypeBazaarSize,*/
		RoF2::invtype::InvTypeInspectSize,
		RoF2::Null, /*RoF2::invtype::InvTypeRealEstateSize*/
		RoF2::invtype::InvTypeViewMODPCSize,
		RoF2::invtype::InvTypeViewMODBankSize,
		RoF2::invtype::InvTypeViewMODSharedBankSize,
		RoF2::invtype::InvTypeViewMODLimboSize,
		RoF2::invtype::InvTypeAltStorageSize,
		RoF2::invtype::InvTypeArchivedSize,
		RoF2::invtype::InvTypeMailSize,
		RoF2::invtype::InvTypeGuildTrophyTributeSize,
		RoF2::invtype::InvTypeKronoSize,
		RoF2::invtype::InvTypeOtherSize,

		RoF2::Null, /*0x00000003FFFFFFFF,*/
		EQEmu::inventory::ContainerCount, /*RoF2::invbag::ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*RoF2::invaug::ItemAugSize,*/

		RoF2::False, /*RoF2::inventory::AllowEmptyBagInBag,*/
		RoF2::inventory::AllowClickCastFromBag,
		RoF2::inventory::ConcatenateInvTypeLimbo,
		RoF2::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::InvTypeTradeSize,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null, /*InvTypeCorpseSize,*/
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,
		EntityLimits::NPC::Null,

		EntityLimits::NPC::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::NPC::False,
		EntityLimits::NPC::False,
		EntityLimits::NPC::False,
		EntityLimits::NPC::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::InvTypeTradeSize,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null, /*InvTypeCorpseSize,*/
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null,

		EntityLimits::NPCMerchant::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::NPCMerchant::False,
		EntityLimits::NPCMerchant::False,
		EntityLimits::NPCMerchant::False,
		EntityLimits::NPCMerchant::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::InvTypeTradeSize,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null, /*InvTypeCorpseSize,*/
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Merc::Null,

		EntityLimits::Merc::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::Merc::False,
		EntityLimits::Merc::False,
		EntityLimits::Merc::False,
		EntityLimits::Merc::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::InvTypeTradeSize,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null, 
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null, /*InvTypeCorpseSize,*/
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Bot::Null,

		EntityLimits::Bot::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::Bot::False,
		EntityLimits::Bot::False,
		EntityLimits::Bot::False,
		EntityLimits::Bot::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::InvTypeTradeSize,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null, /*InvTypeCorpseSize,*/
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null,

		EntityLimits::ClientPet::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::ClientPet::False,
		EntityLimits::ClientPet::False,
		EntityLimits::ClientPet::False,
		EntityLimits::ClientPet::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::InvTypeTradeSize,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null, /*InvTypeCorpseSize,*/
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null,

		EntityLimits::NPCPet::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::NPCPet::False,
		EntityLimits::NPCPet::False,
		EntityLimits::NPCPet::False,
		EntityLimits::NPCPet::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::InvTypeTradeSize,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null, /*InvTypeCorpseSize,*/
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null,

		EntityLimits::MercPet::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::MercPet::False,
		EntityLimits::MercPet::False,
		EntityLimits::MercPet::False,
		EntityLimits::MercPet::False
	},
	{
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::InvTypeTradeSize,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null, /*InvTypeCorpseSize,*/
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null,

		EntityLimits::BotPet::Null,
		EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		EntityLimits::BotPet::False,
		EntityLimits::BotPet::False,
		EntityLimits::BotPet::False,
		EntityLimits::BotPet::False
	},
	{ // OfflineTitanium
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::invtype::InvTypeTradeSize,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::invtype::InvTypeMerchantSize,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null, /*Titanium::invtype::InvTypeBazaarSize,*/
		Titanium::invtype::InvTypeInspectSize,
		Titanium::Null,
		Titanium::invtype::InvTypeViewMODPCSize,
		Titanium::invtype::InvTypeViewMODBankSize,
		Titanium::invtype::InvTypeViewMODSharedBankSize,
		Titanium::invtype::InvTypeViewMODLimboSize,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,
		Titanium::Null,

		Titanium::Null,
		EQEmu::inventory::ContainerCount, /*Titanium::Null,*/
		Titanium::Null,

		Titanium::False,
		Titanium::False,
		Titanium::False,
		Titanium::False
	},
	{ // OfflineSoF
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::invtype::InvTypeTradeSize,
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::invtype::InvTypeMerchantSize,
		SoF::Null,
		SoF::Null,
		SoF::Null, /*SoF::invtype::InvTypeBazaarSize,*/
		SoF::invtype::InvTypeInspectSize,
		SoF::Null,
		SoF::invtype::InvTypeViewMODPCSize,
		SoF::invtype::InvTypeViewMODBankSize,
		SoF::invtype::InvTypeViewMODSharedBankSize,
		SoF::invtype::InvTypeViewMODLimboSize,
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::Null,
		SoF::Null,

		SoF::Null,
		EQEmu::inventory::ContainerCount, /*SoF::Null,*/
		SoF::Null,

		SoF::False,
		SoF::False,
		SoF::False,
		SoF::False
	},
	{ // OfflineSoD
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::invtype::InvTypeTradeSize,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::invtype::InvTypeMerchantSize,
		SoD::Null,
		SoD::Null,
		SoD::Null, /*SoD::invtype::InvTypeBazaarSize,*/
		SoD::invtype::InvTypeInspectSize,
		SoD::Null,
		SoD::invtype::InvTypeViewMODPCSize,
		SoD::invtype::InvTypeViewMODBankSize,
		SoD::invtype::InvTypeViewMODSharedBankSize,
		SoD::invtype::InvTypeViewMODLimboSize,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::Null,
		SoD::Null,

		SoD::Null,
		EQEmu::inventory::ContainerCount, /*SoD::Null,*/
		SoD::Null,

		SoD::False,
		SoD::False,
		SoD::False,
		SoD::False
	},
	{ // OfflineUF
		UF::Null,
		UF::Null,
		UF::Null,
		UF::invtype::InvTypeTradeSize,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::invtype::InvTypeMerchantSize,
		UF::Null,
		UF::Null,
		UF::Null, /*UF::invtype::InvTypeBazaarSize,*/
		UF::invtype::InvTypeInspectSize,
		UF::Null,
		UF::invtype::InvTypeViewMODPCSize,
		UF::invtype::InvTypeViewMODBankSize,
		UF::invtype::InvTypeViewMODSharedBankSize,
		UF::invtype::InvTypeViewMODLimboSize,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::Null,
		UF::Null,

		UF::Null,
		EQEmu::inventory::ContainerCount, /*UF::Null,*/
		UF::Null,

		UF::False,
		UF::False,
		UF::False,
		UF::False
	},
	{ // OfflineRoF
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::invtype::InvTypeTradeSize,
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::invtype::InvTypeMerchantSize,
		RoF::Null,
		RoF::Null,
		RoF::Null, /*RoF::invtype::InvTypeBazaarSize,*/
		RoF::invtype::InvTypeInspectSize,
		RoF::Null,
		RoF::invtype::InvTypeViewMODPCSize,
		RoF::invtype::InvTypeViewMODBankSize,
		RoF::invtype::InvTypeViewMODSharedBankSize,
		RoF::invtype::InvTypeViewMODLimboSize,
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::Null,
		RoF::Null,

		RoF::Null,
		EQEmu::inventory::ContainerCount, /*RoF::Null,*/
		RoF::Null,

		RoF::False,
		RoF::False,
		RoF::False,
		RoF::False
	},
	{ // OfflineRoF2
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::invtype::InvTypeTradeSize,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::invtype::InvTypeMerchantSize,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null, /*RoF2::invtype::InvTypeBazaarSize,*/
		RoF2::invtype::InvTypeInspectSize,
		RoF2::Null,
		RoF2::invtype::InvTypeViewMODPCSize,
		RoF2::invtype::InvTypeViewMODBankSize,
		RoF2::invtype::InvTypeViewMODSharedBankSize,
		RoF2::invtype::InvTypeViewMODLimboSize,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,
		RoF2::Null,

		RoF2::Null,
		EQEmu::inventory::ContainerCount, /*RoF2::Null,*/
		RoF2::Null,

		RoF2::False,
		RoF2::False,
		RoF2::False,
		RoF2::False
	}
};

const EQEmu::inventory::LookupEntry* EQEmu::inventory::Lookup(versions::MobVersion mob_version)
{
	return &inventory_lookup_entries[static_cast<int>(versions::ValidateMobVersion(mob_version))];
}

static const EQEmu::behavior::LookupEntry behavior_lookup_entries[EQEmu::versions::MobVersionCount] =
{
	{ // Unknown
		ClientUnknown::True
	},
	{ // Client62
		Client62::True
	},
	{ // Titanium
		Titanium::behavior::CoinHasWeight
	},
	{ // SoF
		SoF::behavior::CoinHasWeight
	},
	{ // SoD
		SoD::behavior::CoinHasWeight
	},
	{ // UF
		UF::behavior::CoinHasWeight
	},
	{ // RoF
		RoF::behavior::CoinHasWeight
	},
	{ // RoF2
		RoF2::behavior::CoinHasWeight
	},
	{ // NPC
		EntityLimits::NPC::True /*CoinHasWeight*/
	},
	{ // NPCMerchant
		EntityLimits::NPC::True /*CoinHasWeight*/
	},
	{ // Merc
		EntityLimits::Merc::True /*CoinHasWeight*/
	},
	{ // Bot
		EntityLimits::Bot::True /*CoinHasWeight*/
	},
	{ // ClientPet
		EntityLimits::ClientPet::True /*CoinHasWeight*/
	},
	{ // NPCPet
		EntityLimits::NPCPet::True /*CoinHasWeight*/
	},
	{ // MercPet
		EntityLimits::MercPet::True /*CoinHasWeight*/
	},
	{ // BotPet
		EntityLimits::BotPet::True /*CoinHasWeight*/
	},
	{ // OfflineTitanium
		Titanium::False
	},
	{ // OfflineSoF
		SoF::False
	},
	{ // OfflineSoD
		SoD::False
	},
	{ // OfflineUF
		UF::False
	},
	{ // OfflineRoF
		RoF::False
	},
	{ // OfflineRoF2
		RoF2::False
	}
};

const EQEmu::behavior::LookupEntry* EQEmu::behavior::Lookup(versions::MobVersion mob_version)
{
	return &behavior_lookup_entries[static_cast<int>(versions::ValidateMobVersion(mob_version))];
}
