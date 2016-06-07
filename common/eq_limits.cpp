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
	{ // Unknown
		ClientUnknown::Null
	},
	{ // Client62
		Client62::Null
	},
	{ // Titanium
		Titanium::constants::CharacterCreationLimit
	},
	{ // SoF
		SoF::constants::CharacterCreationLimit
	},
	{ // SoD
		SoD::constants::CharacterCreationLimit
	},
	{ // UF
		UF::constants::CharacterCreationLimit
	},
	{ // RoF
		RoF::constants::CharacterCreationLimit
	},
	{ // RoF2
		RoF2::constants::CharacterCreationLimit
	}
};

const EQEmu::constants::LookupEntry* EQEmu::constants::Lookup(versions::ClientVersion client_version)
{
	return &constants_lookup_entries[static_cast<int>(versions::ValidateClientVersion(client_version))];
}

static const EQEmu::inventory::LookupEntry inventory_lookup_entries[EQEmu::versions::InventoryVersionCount] =
{
	{ // Unknown
		ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null,
		ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null,
		ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null,
		ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null,
		ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null,

		ClientUnknown::Null, ClientUnknown::Null, ClientUnknown::Null,

		ClientUnknown::False, ClientUnknown::False, ClientUnknown::False, ClientUnknown::False
	},
	{ // Client62
		Client62::Null, Client62::Null, Client62::Null, Client62::Null, Client62::Null,
		Client62::Null, Client62::Null, Client62::Null, Client62::Null, Client62::Null,
		Client62::Null, Client62::Null, Client62::Null, Client62::Null, Client62::Null,
		Client62::Null, Client62::Null, Client62::Null, Client62::Null, Client62::Null,
		Client62::Null, Client62::Null, Client62::Null, Client62::Null, Client62::Null,

		Client62::Null, Client62::Null, Client62::Null,

		Client62::False, Client62::False, Client62::False, Client62::False
	},
	{ // Titanium
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*Titanium::invtype::InvTypePossessionsSize,*/ Titanium::invtype::InvTypeBankSize, Titanium::invtype::InvTypeSharedBankSize, Titanium::invtype::InvTypeTradeSize, Titanium::invtype::InvTypeWorldSize,
		Titanium::invtype::InvTypeLimboSize, Titanium::invtype::InvTypeTributeSize, Titanium::Null, Titanium::Null, /*Titanium::invtype::InvTypeGuildTributeSize,*/ Titanium::invtype::InvTypeMerchantSize,
		Titanium::Null, Titanium::invtype::InvTypeCorpseSize, EQEmu::legacy::TYPE_BAZAAR_SIZE, /*Titanium::invtype::InvTypeBazaarSize,*/ Titanium::invtype::InvTypeInspectSize, Titanium::Null,
		Titanium::invtype::InvTypeViewMODPCSize, Titanium::invtype::InvTypeViewMODBankSize, Titanium::invtype::InvTypeViewMODSharedBankSize, Titanium::invtype::InvTypeViewMODLimboSize, Titanium::invtype::InvTypeAltStorageSize,
		Titanium::invtype::InvTypeArchivedSize, Titanium::Null, Titanium::Null, Titanium::Null, Titanium::invtype::InvTypeOtherSize,

		Titanium::Null, /*0x000000027FDFFFFF,*/ EQEmu::legacy::ITEM_CONTAINER_SIZE, /*Titanium::invbag::ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*Titanium::invaug::ItemAugSize,*/

		Titanium::inventory::AllowEmptyBagInBag, Titanium::inventory::AllowClickCastFromBag, Titanium::inventory::ConcatenateInvTypeLimbo, Titanium::inventory::AllowOverLevelEquipment
	},
	{ // SoF
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*SoF::invtype::InvTypePossessionsSize,*/ SoF::invtype::InvTypeBankSize, SoF::invtype::InvTypeSharedBankSize, SoF::invtype::InvTypeTradeSize, SoF::invtype::InvTypeWorldSize,
		SoF::invtype::InvTypeLimboSize, SoF::invtype::InvTypeTributeSize, SoF::Null, SoF::Null, /*SoF::invtype::InvTypeGuildTributeSize,*/ SoF::invtype::InvTypeMerchantSize,
		SoF::Null, SoF::invtype::InvTypeCorpseSize, EQEmu::legacy::TYPE_BAZAAR_SIZE, /*SoF::invtype::InvTypeBazaarSize,*/ SoF::invtype::InvTypeInspectSize, SoF::Null,
		SoF::invtype::InvTypeViewMODPCSize, SoF::invtype::InvTypeViewMODBankSize, SoF::invtype::InvTypeViewMODSharedBankSize, SoF::invtype::InvTypeViewMODLimboSize, SoF::invtype::InvTypeAltStorageSize,
		SoF::invtype::InvTypeArchivedSize, SoF::Null, SoF::Null, SoF::Null, SoF::invtype::InvTypeOtherSize,

		SoF::Null, /*0x000000027FFFFFFF,*/ EQEmu::legacy::ITEM_CONTAINER_SIZE, /*SoF::invbag::ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*SoF::invaug::ItemAugSize,*/

		SoF::inventory::AllowEmptyBagInBag, SoF::inventory::AllowClickCastFromBag, SoF::inventory::ConcatenateInvTypeLimbo, SoF::inventory::AllowOverLevelEquipment
	},
	{ // SoD
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*SoD::invtype::InvTypePossessionsSize,*/ SoD::invtype::InvTypeBankSize, SoD::invtype::InvTypeSharedBankSize, SoD::invtype::InvTypeTradeSize, SoD::invtype::InvTypeWorldSize,
		SoD::invtype::InvTypeLimboSize, SoD::invtype::InvTypeTributeSize, SoD::Null, SoD::Null, /*SoD::invtype::InvTypeGuildTributeSize,*/ SoD::invtype::InvTypeMerchantSize,
		SoD::Null, SoD::invtype::InvTypeCorpseSize, EQEmu::legacy::TYPE_BAZAAR_SIZE, /*SoD::invtype::InvTypeBazaarSize,*/ SoD::invtype::InvTypeInspectSize, SoD::Null,
		SoD::invtype::InvTypeViewMODPCSize, SoD::invtype::InvTypeViewMODBankSize, SoD::invtype::InvTypeViewMODSharedBankSize, SoD::invtype::InvTypeViewMODLimboSize, SoD::invtype::InvTypeAltStorageSize,
		SoD::invtype::InvTypeArchivedSize, SoD::Null, SoD::Null, SoD::Null, SoD::invtype::InvTypeOtherSize,

		SoD::Null, /*0x000000027FFFFFFF,*/ EQEmu::legacy::ITEM_CONTAINER_SIZE, /*SoD::invbag::ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*SoD::invaug::ItemAugSize,*/

		SoD::inventory::AllowEmptyBagInBag, SoD::inventory::AllowClickCastFromBag, SoD::inventory::ConcatenateInvTypeLimbo, SoD::inventory::AllowOverLevelEquipment
	},
	{ // UF
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*UF::invtype::InvTypePossessionsSize,*/ UF::invtype::InvTypeBankSize, UF::invtype::InvTypeSharedBankSize, UF::invtype::InvTypeTradeSize, UF::invtype::InvTypeWorldSize,
		UF::invtype::InvTypeLimboSize, UF::invtype::InvTypeTributeSize, UF::Null, UF::Null, /*UF::invtype::InvTypeGuildTributeSize,*/ UF::invtype::InvTypeMerchantSize,
		UF::Null, UF::invtype::InvTypeCorpseSize, EQEmu::legacy::TYPE_BAZAAR_SIZE, /*UF::invtype::InvTypeBazaarSize,*/ UF::invtype::InvTypeInspectSize, UF::Null,
		UF::invtype::InvTypeViewMODPCSize, UF::invtype::InvTypeViewMODBankSize, UF::invtype::InvTypeViewMODSharedBankSize, UF::invtype::InvTypeViewMODLimboSize, UF::invtype::InvTypeAltStorageSize,
		UF::invtype::InvTypeArchivedSize, UF::Null, UF::Null, UF::Null, UF::invtype::InvTypeOtherSize,

		UF::Null, /*0x000000027FFFFFFF,*/ EQEmu::legacy::ITEM_CONTAINER_SIZE, /*UF::invbag::ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*UF::invaug::ItemAugSize,*/

		UF::inventory::AllowEmptyBagInBag, UF::inventory::AllowClickCastFromBag, UF::inventory::ConcatenateInvTypeLimbo, UF::inventory::AllowOverLevelEquipment
	},
	{ // RoF
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*RoF::invtype::InvTypePossessionsSize,*/ RoF::invtype::InvTypeBankSize, RoF::invtype::InvTypeSharedBankSize, RoF::invtype::InvTypeTradeSize, RoF::invtype::InvTypeWorldSize,
		RoF::invtype::InvTypeLimboSize, RoF::invtype::InvTypeTributeSize, RoF::Null, /*RoF::invtype::InvTypeTrophyTributeSize,*/ RoF::Null, /*RoF::invtype::InvTypeGuildTributeSize,*/ RoF::invtype::InvTypeMerchantSize,
		RoF::Null, /*RoF::invtype::InvTypeDeletedSize,*/ RoF::invtype::InvTypeCorpseSize, EQEmu::legacy::TYPE_BAZAAR_SIZE, /*RoF::invtype::InvTypeBazaarSize,*/ RoF::invtype::InvTypeInspectSize, RoF::Null, /*RoF::invtype::InvTypeRealEstateSize,*/
		RoF::invtype::InvTypeViewMODPCSize, RoF::invtype::InvTypeViewMODBankSize, RoF::invtype::InvTypeViewMODSharedBankSize, RoF::invtype::InvTypeViewMODLimboSize, RoF::invtype::InvTypeAltStorageSize,
		RoF::invtype::InvTypeArchivedSize, RoF::invtype::InvTypeMailSize, RoF::invtype::InvTypeGuildTrophyTributeSize, RoF::Null, RoF::invtype::InvTypeOtherSize,

		RoF::Null, /*0x00000003FFFFFFFF,*/ EQEmu::legacy::ITEM_CONTAINER_SIZE, /*RoF::invbag::ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*RoF::invaug::ItemAugSize,*/

		RoF::False, /*RoF::inventory::AllowEmptyBagInBag,*/ RoF::inventory::AllowClickCastFromBag, RoF::inventory::ConcatenateInvTypeLimbo, RoF::inventory::AllowOverLevelEquipment
	},
	{ // RoF2
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*RoF2::invtype::InvTypePossessionsSize,*/ RoF2::invtype::InvTypeBankSize, RoF2::invtype::InvTypeSharedBankSize, RoF2::invtype::InvTypeTradeSize, RoF2::invtype::InvTypeWorldSize,
		RoF2::invtype::InvTypeLimboSize, RoF2::invtype::InvTypeTributeSize, RoF2::Null, /*RoF2::invtype::InvTypeTrophyTributeSize,*/ RoF2::Null, /*RoF2::invtype::InvTypeGuildTributeSize,*/ RoF2::invtype::InvTypeMerchantSize,
		RoF2::Null, /*RoF2::invtype::InvTypeDeletedSize,*/ RoF2::invtype::InvTypeCorpseSize, EQEmu::legacy::TYPE_BAZAAR_SIZE, /*RoF2::invtype::InvTypeBazaarSize,*/ RoF2::invtype::InvTypeInspectSize, RoF2::Null, /*RoF2::invtype::InvTypeRealEstateSize*/
		RoF2::invtype::InvTypeViewMODPCSize, RoF2::invtype::InvTypeViewMODBankSize, RoF2::invtype::InvTypeViewMODSharedBankSize, RoF2::invtype::InvTypeViewMODLimboSize, RoF2::invtype::InvTypeAltStorageSize,
		RoF2::invtype::InvTypeArchivedSize, RoF2::invtype::InvTypeMailSize, RoF2::invtype::InvTypeGuildTrophyTributeSize, RoF2::invtype::InvTypeKronoSize, RoF2::invtype::InvTypeOtherSize,

		RoF2::Null, /*0x00000003FFFFFFFF,*/ EQEmu::legacy::ITEM_CONTAINER_SIZE, /*RoF2::invbag::ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*RoF2::invaug::ItemAugSize,*/

		RoF2::False, /*RoF2::inventory::AllowEmptyBagInBag,*/ RoF2::inventory::AllowClickCastFromBag, RoF2::inventory::ConcatenateInvTypeLimbo, RoF2::inventory::AllowOverLevelEquipment
	},
	{ // NPC
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::InvTypeTradeSize, EntityLimits::NPC::Null,
		EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null,
		EntityLimits::NPC::Null, EntityLimits::NPC::Null, /*InvTypeCorpseSize,*/ EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null,
		EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null,
		EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null, EntityLimits::NPC::Null,

		EntityLimits::NPC::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::NPC::False, EntityLimits::NPC::False, EntityLimits::NPC::False, EntityLimits::NPC::False
	},
	{ // NPCMerchant
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::InvTypeTradeSize, EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, /*InvTypeCorpseSize,*/ EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null,
		EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null, EntityLimits::NPCMerchant::Null,

		EntityLimits::NPCMerchant::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::NPCMerchant::False, EntityLimits::NPCMerchant::False, EntityLimits::NPCMerchant::False, EntityLimits::NPCMerchant::False
	},
	{ // Merc
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::InvTypeTradeSize, EntityLimits::Merc::Null,
		EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null,
		EntityLimits::Merc::Null, EntityLimits::Merc::Null, /*InvTypeCorpseSize,*/ EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null,
		EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null,
		EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null, EntityLimits::Merc::Null,

		EntityLimits::Merc::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::Merc::False, EntityLimits::Merc::False, EntityLimits::Merc::False, EntityLimits::Merc::False
	},
	{ // Bot
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::InvTypeTradeSize, EntityLimits::Bot::Null,
		EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, 
		EntityLimits::Bot::Null, EntityLimits::Bot::Null, /*InvTypeCorpseSize,*/ EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null,
		EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null,
		EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null, EntityLimits::Bot::Null,

		EntityLimits::Bot::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::Bot::False, EntityLimits::Bot::False, EntityLimits::Bot::False, EntityLimits::Bot::False
	},
	{ // ClientPet
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::InvTypeTradeSize, EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, /*InvTypeCorpseSize,*/ EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null,
		EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null, EntityLimits::ClientPet::Null,

		EntityLimits::ClientPet::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::ClientPet::False, EntityLimits::ClientPet::False, EntityLimits::ClientPet::False, EntityLimits::ClientPet::False
	},
	{ // NPCPet
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::InvTypeTradeSize, EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, /*InvTypeCorpseSize,*/ EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null,
		EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null, EntityLimits::NPCPet::Null,

		EntityLimits::NPCPet::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::NPCPet::False, EntityLimits::NPCPet::False, EntityLimits::NPCPet::False, EntityLimits::NPCPet::False
	},
	{ // MercPet
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::InvTypeTradeSize, EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, /*InvTypeCorpseSize,*/ EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null,
		EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null, EntityLimits::MercPet::Null,

		EntityLimits::MercPet::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::MercPet::False, EntityLimits::MercPet::False, EntityLimits::MercPet::False, EntityLimits::MercPet::False
	},
	{ // BotPet
		EQEmu::legacy::TYPE_POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/ EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::InvTypeTradeSize, EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, /*InvTypeCorpseSize,*/ EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null,
		EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null, EntityLimits::BotPet::Null,

		EntityLimits::BotPet::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*ItemBagSize,*/ EQEmu::legacy::ITEM_COMMON_SIZE, /*ItemAugSize,*/

		EntityLimits::BotPet::False, EntityLimits::BotPet::False, EntityLimits::BotPet::False, EntityLimits::BotPet::False
	},
	{ // OfflineTitanium
		Titanium::Null, Titanium::Null, Titanium::Null, Titanium::invtype::InvTypeTradeSize, Titanium::Null,
		Titanium::Null, Titanium::Null, Titanium::Null, Titanium::Null, Titanium::invtype::InvTypeMerchantSize,
		Titanium::Null, Titanium::Null, Titanium::Null, /*Titanium::invtype::InvTypeBazaarSize,*/ Titanium::invtype::InvTypeInspectSize, Titanium::Null,
		Titanium::invtype::InvTypeViewMODPCSize, Titanium::invtype::InvTypeViewMODBankSize, Titanium::invtype::InvTypeViewMODSharedBankSize, Titanium::invtype::InvTypeViewMODLimboSize, Titanium::Null,
		Titanium::Null, Titanium::Null, Titanium::Null, Titanium::Null, Titanium::Null,

		Titanium::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*Titanium::Null,*/ Titanium::Null,

		Titanium::False, Titanium::False, Titanium::False, Titanium::False
	},
	{ // OfflineSoF
		SoF::Null, SoF::Null, SoF::Null, SoF::invtype::InvTypeTradeSize, SoF::Null,
		SoF::Null, SoF::Null, SoF::Null, SoF::Null, SoF::invtype::InvTypeMerchantSize,
		SoF::Null, SoF::Null, SoF::Null, /*SoF::invtype::InvTypeBazaarSize,*/ SoF::invtype::InvTypeInspectSize, SoF::Null,
		SoF::invtype::InvTypeViewMODPCSize, SoF::invtype::InvTypeViewMODBankSize, SoF::invtype::InvTypeViewMODSharedBankSize, SoF::invtype::InvTypeViewMODLimboSize, SoF::Null,
		SoF::Null, SoF::Null, SoF::Null, SoF::Null, SoF::Null,

		SoF::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*SoF::Null,*/ SoF::Null,

		SoF::False, SoF::False, SoF::False, SoF::False
	},
	{ // OfflineSoD
		SoD::Null, SoD::Null, SoD::Null, SoD::invtype::InvTypeTradeSize, SoD::Null,
		SoD::Null, SoD::Null, SoD::Null, SoD::Null, SoD::invtype::InvTypeMerchantSize,
		SoD::Null, SoD::Null, SoD::Null, /*SoD::invtype::InvTypeBazaarSize,*/ SoD::invtype::InvTypeInspectSize, SoD::Null,
		SoD::invtype::InvTypeViewMODPCSize, SoD::invtype::InvTypeViewMODBankSize, SoD::invtype::InvTypeViewMODSharedBankSize, SoD::invtype::InvTypeViewMODLimboSize, SoD::Null,
		SoD::Null, SoD::Null, SoD::Null, SoD::Null, SoD::Null,

		SoD::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*SoD::Null,*/ SoD::Null,

		SoD::False, SoD::False, SoD::False, SoD::False
	},
	{ // OfflineUF
		UF::Null, UF::Null, UF::Null, UF::invtype::InvTypeTradeSize, UF::Null,
		UF::Null, UF::Null, UF::Null, UF::Null, UF::invtype::InvTypeMerchantSize,
		UF::Null, UF::Null, UF::Null, /*UF::invtype::InvTypeBazaarSize,*/ UF::invtype::InvTypeInspectSize, UF::Null,
		UF::invtype::InvTypeViewMODPCSize, UF::invtype::InvTypeViewMODBankSize, UF::invtype::InvTypeViewMODSharedBankSize, UF::invtype::InvTypeViewMODLimboSize, UF::Null,
		UF::Null, UF::Null, UF::Null, UF::Null, UF::Null,

		UF::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*UF::Null,*/ UF::Null,

		UF::False, UF::False, UF::False, UF::False
	},
	{ // OfflineRoF
		RoF::Null, RoF::Null, RoF::Null, RoF::invtype::InvTypeTradeSize, RoF::Null,
		RoF::Null, RoF::Null, RoF::Null, RoF::Null, RoF::invtype::InvTypeMerchantSize,
		RoF::Null, RoF::Null, RoF::Null, /*RoF::invtype::InvTypeBazaarSize,*/ RoF::invtype::InvTypeInspectSize, RoF::Null,
		RoF::invtype::InvTypeViewMODPCSize, RoF::invtype::InvTypeViewMODBankSize, RoF::invtype::InvTypeViewMODSharedBankSize, RoF::invtype::InvTypeViewMODLimboSize, RoF::Null,
		RoF::Null, RoF::Null, RoF::Null, RoF::Null, RoF::Null,

		RoF::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*RoF::Null,*/ RoF::Null,

		RoF::False, RoF::False, RoF::False, RoF::False
	},
	{ // OfflineRoF2
		RoF2::Null, RoF2::Null, RoF2::Null, RoF2::invtype::InvTypeTradeSize, RoF2::Null,
		RoF2::Null, RoF2::Null, RoF2::Null, RoF2::Null, RoF2::invtype::InvTypeMerchantSize,
		RoF2::Null, RoF2::Null, RoF2::Null, /*RoF2::invtype::InvTypeBazaarSize,*/ RoF2::invtype::InvTypeInspectSize, RoF2::Null,
		RoF2::invtype::InvTypeViewMODPCSize, RoF2::invtype::InvTypeViewMODBankSize, RoF2::invtype::InvTypeViewMODSharedBankSize, RoF2::invtype::InvTypeViewMODLimboSize, RoF2::Null,
		RoF2::Null, RoF2::Null, RoF2::Null, RoF2::Null, RoF2::Null,

		RoF2::Null, EQEmu::legacy::ITEM_CONTAINER_SIZE, /*RoF2::Null,*/ RoF2::Null,

		RoF2::False, RoF2::False, RoF2::False, RoF2::False
	}
};

const EQEmu::inventory::LookupEntry* EQEmu::inventory::Lookup(versions::InventoryVersion inventory_version)
{
	return &inventory_lookup_entries[static_cast<int>(versions::ValidateInventoryVersion(inventory_version))];
}

static const EQEmu::behavior::LookupEntry behavior_lookup_entries[EQEmu::versions::InventoryVersionCount] =
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

const EQEmu::behavior::LookupEntry* EQEmu::behavior::Lookup(versions::InventoryVersion inventory_version)
{
	return &behavior_lookup_entries[static_cast<int>(versions::ValidateInventoryVersion(inventory_version))];
}
