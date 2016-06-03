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


size_t EQEmu::constants::GetCharacterCreationLimit(versions::ClientVersion client_version)
{
	static const size_t local[versions::ClientVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		Titanium::constants::CharacterCreationLimit,
		SoF::constants::CharacterCreationLimit,
		SoD::constants::CharacterCreationLimit,
		UF::constants::CharacterCreationLimit,
		RoF::constants::CharacterCreationLimit,
		RoF2::constants::CharacterCreationLimit
	};

	return local[static_cast<size_t>(versions::ValidateClientVersion(client_version))];
}

size_t EQEmu::inventory::GetInventoryTypeSize(versions::InventoryVersion inventory_version, int inv_type)
{
	static const size_t local[legacy::TypeCount][versions::InventoryVersionCount] = {
		{ // local[TypePossessions]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_POSSESSIONS_SIZE, //Titanium::invtype::InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //SoF::invtype::InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //SoD::invtype::InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //UF::invtype::InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //RoF::invtype::InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //RoF2::invtype::InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //InvTypePossessionsSize,
			legacy::TYPE_POSSESSIONS_SIZE, //InvTypePossessionsSize,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeBankSize,
			SoF::invtype::InvTypeBankSize,
			SoD::invtype::InvTypeBankSize,
			UF::invtype::InvTypeBankSize,
			RoF::invtype::InvTypeBankSize,
			RoF2::invtype::InvTypeBankSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeSharedBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeSharedBankSize,
			SoF::invtype::InvTypeSharedBankSize,
			SoD::invtype::InvTypeSharedBankSize,
			UF::invtype::InvTypeSharedBankSize,
			RoF::invtype::InvTypeSharedBankSize,
			RoF2::invtype::InvTypeSharedBankSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeTrade]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeTradeSize,
			SoF::invtype::InvTypeTradeSize,
			SoD::invtype::InvTypeTradeSize,
			UF::invtype::InvTypeTradeSize,
			RoF::invtype::InvTypeTradeSize,
			RoF2::invtype::InvTypeTradeSize,
			EntityLimits::NPC::InvTypeTradeSize,
			EntityLimits::Merc::InvTypeTradeSize,
			EntityLimits::Bot::InvTypeTradeSize, // client thinks this is another client
			EntityLimits::Pet::InvTypeTradeSize,
			Titanium::invtype::InvTypeTradeSize,
			SoF::invtype::InvTypeTradeSize,
			SoD::invtype::InvTypeTradeSize,
			UF::invtype::InvTypeTradeSize,
			RoF::invtype::InvTypeTradeSize,
			RoF2::invtype::InvTypeTradeSize
		},
		{ // local[TypeWorld]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeWorldSize,
			SoF::invtype::InvTypeWorldSize,
			SoD::invtype::InvTypeWorldSize,
			UF::invtype::InvTypeWorldSize,
			RoF::invtype::InvTypeWorldSize,
			RoF2::invtype::InvTypeWorldSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeLimbo]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeLimboSize,
			SoF::invtype::InvTypeLimboSize,
			SoD::invtype::InvTypeLimboSize,
			UF::invtype::InvTypeLimboSize,
			RoF::invtype::InvTypeLimboSize,
			RoF2::invtype::InvTypeLimboSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeTributeSize,
			SoF::invtype::InvTypeTributeSize,
			SoD::invtype::InvTypeTributeSize,
			UF::invtype::InvTypeTributeSize,
			RoF::invtype::InvTypeTributeSize,
			RoF2::invtype::InvTypeTributeSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeTrophyTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null, //RoF::invtype::InvTypeTrophyTributeSize,
			RoF2::Null, //RoF2::invtype::InvTypeTrophyTributeSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeGuildTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null, //Titanium::invtype::InvTypeGuildTributeSize,
			SoF::Null, //SoF::invtype::InvTypeGuildTributeSize,
			SoD::Null, //SoD::invtype::InvTypeGuildTributeSize,
			UF::Null, //UF::invtype::InvTypeGuildTributeSize,
			RoF::Null, //RoF::invtype::InvTypeGuildTributeSize,
			RoF2::Null, //RoF2::invtype::InvTypeGuildTributeSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeMerchant]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeMerchantSize,
			SoF::invtype::InvTypeMerchantSize,
			SoD::invtype::InvTypeMerchantSize,
			UF::invtype::InvTypeMerchantSize,
			RoF::invtype::InvTypeMerchantSize,
			RoF2::invtype::InvTypeMerchantSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::invtype::InvTypeMerchantSize,
			SoF::invtype::InvTypeMerchantSize,
			SoD::invtype::InvTypeMerchantSize,
			UF::invtype::InvTypeMerchantSize,
			RoF::invtype::InvTypeMerchantSize,
			RoF2::invtype::InvTypeMerchantSize
		},
		{ // local[TypeDeleted]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null, //RoF::invtype::InvTypeDeletedSize,
			RoF2::Null, //RoF2::invtype::InvTypeDeletedSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeCorpse]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeCorpseSize,
			SoF::invtype::InvTypeCorpseSize,
			SoD::invtype::InvTypeCorpseSize,
			UF::invtype::InvTypeCorpseSize,
			RoF::invtype::InvTypeCorpseSize,
			RoF2::invtype::InvTypeCorpseSize,
			EntityLimits::NPC::Null, //InvTypeCorpseSize,
			EntityLimits::Merc::Null, //InvTypeCorpseSize,
			EntityLimits::Bot::Null, //InvTypeCorpseSize,
			EntityLimits::Pet::Null, //InvTypeCorpseSize,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeBazaar]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_BAZAAR_SIZE, //Titanium::invtype::InvTypeBazaarSize,
			legacy::TYPE_BAZAAR_SIZE, //SoF::invtype::InvTypeBazaarSize,
			legacy::TYPE_BAZAAR_SIZE, //SoD::invtype::InvTypeBazaarSize,
			legacy::TYPE_BAZAAR_SIZE, //UF::invtype::InvTypeBazaarSize,
			legacy::TYPE_BAZAAR_SIZE, //RoF::invtype::InvTypeBazaarSize,
			legacy::TYPE_BAZAAR_SIZE, //RoF2::invtype::InvTypeBazaarSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null, //Titanium::invtype::InvTypeBazaarSize,
			SoF::Null, //SoF::invtype::InvTypeBazaarSize,
			SoD::Null, //SoD::invtype::InvTypeBazaarSize,
			UF::Null, //UF::invtype::InvTypeBazaarSize,
			RoF::Null, //RoF::invtype::InvTypeBazaarSize,
			RoF2::Null //RoF2::invtype::InvTypeBazaarSize,
		},
		{ // local[TypeInspect]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeInspectSize,
			SoF::invtype::InvTypeInspectSize,
			SoD::invtype::InvTypeInspectSize,
			UF::invtype::InvTypeInspectSize,
			RoF::invtype::InvTypeInspectSize,
			RoF2::invtype::InvTypeInspectSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::invtype::InvTypeInspectSize,
			SoF::invtype::InvTypeInspectSize,
			SoD::invtype::InvTypeInspectSize,
			UF::invtype::InvTypeInspectSize,
			RoF::invtype::InvTypeInspectSize,
			RoF2::invtype::InvTypeInspectSize
		},
		{ // local[TypeRealEstate]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null, //RoF::invtype::InvTypeRealEstateSize,
			RoF2::Null, //RoF2::invtype::InvTypeRealEstateSize
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeViewMODPC]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeViewMODPCSize,
			SoF::invtype::InvTypeViewMODPCSize,
			SoD::invtype::InvTypeViewMODPCSize,
			UF::invtype::InvTypeViewMODPCSize,
			RoF::invtype::InvTypeViewMODPCSize,
			RoF2::invtype::InvTypeViewMODPCSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::invtype::InvTypeViewMODPCSize,
			SoF::invtype::InvTypeViewMODPCSize,
			SoD::invtype::InvTypeViewMODPCSize,
			UF::invtype::InvTypeViewMODPCSize,
			RoF::invtype::InvTypeViewMODPCSize,
			RoF2::invtype::InvTypeViewMODPCSize
		},
		{ // local[TypeViewMODBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeViewMODBankSize,
			SoF::invtype::InvTypeViewMODBankSize,
			SoD::invtype::InvTypeViewMODBankSize,
			UF::invtype::InvTypeViewMODBankSize,
			RoF::invtype::InvTypeViewMODBankSize,
			RoF2::invtype::InvTypeViewMODBankSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::invtype::InvTypeViewMODBankSize,
			SoF::invtype::InvTypeViewMODBankSize,
			SoD::invtype::InvTypeViewMODBankSize,
			UF::invtype::InvTypeViewMODBankSize,
			RoF::invtype::InvTypeViewMODBankSize,
			RoF2::invtype::InvTypeViewMODBankSize
		},
		{ // local[TypeViewMODSharedBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeViewMODSharedBankSize,
			SoF::invtype::InvTypeViewMODSharedBankSize,
			SoD::invtype::InvTypeViewMODSharedBankSize,
			UF::invtype::InvTypeViewMODSharedBankSize,
			RoF::invtype::InvTypeViewMODSharedBankSize,
			RoF2::invtype::InvTypeViewMODSharedBankSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::invtype::InvTypeViewMODSharedBankSize,
			SoF::invtype::InvTypeViewMODSharedBankSize,
			SoD::invtype::InvTypeViewMODSharedBankSize,
			UF::invtype::InvTypeViewMODSharedBankSize,
			RoF::invtype::InvTypeViewMODSharedBankSize,
			RoF2::invtype::InvTypeViewMODSharedBankSize
		},
		{ // local[TypeViewMODLimbo]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeViewMODLimboSize,
			SoF::invtype::InvTypeViewMODLimboSize,
			SoD::invtype::InvTypeViewMODLimboSize,
			UF::invtype::InvTypeViewMODLimboSize,
			RoF::invtype::InvTypeViewMODLimboSize,
			RoF2::invtype::InvTypeViewMODLimboSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::invtype::InvTypeViewMODLimboSize,
			SoF::invtype::InvTypeViewMODLimboSize,
			SoD::invtype::InvTypeViewMODLimboSize,
			UF::invtype::InvTypeViewMODLimboSize,
			RoF::invtype::InvTypeViewMODLimboSize,
			RoF2::invtype::InvTypeViewMODLimboSize
		},
		{ // local[TypeAltStorage]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeAltStorageSize,
			SoF::invtype::InvTypeAltStorageSize,
			SoD::invtype::InvTypeAltStorageSize,
			UF::invtype::InvTypeAltStorageSize,
			RoF::invtype::InvTypeAltStorageSize,
			RoF2::invtype::InvTypeAltStorageSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeArchived]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeArchivedSize,
			SoF::invtype::InvTypeArchivedSize,
			SoD::invtype::InvTypeArchivedSize,
			UF::invtype::InvTypeArchivedSize,
			RoF::invtype::InvTypeArchivedSize,
			RoF2::invtype::InvTypeArchivedSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeMail]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::invtype::InvTypeMailSize,
			RoF2::invtype::InvTypeMailSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeGuildTrophyTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::invtype::InvTypeGuildTrophyTributeSize,
			RoF2::invtype::InvTypeGuildTrophyTributeSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeKrono]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::invtype::InvTypeKronoSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		},
		{ // local[TypeOther]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeOtherSize,
			SoF::invtype::InvTypeOtherSize,
			SoD::invtype::InvTypeOtherSize,
			UF::invtype::InvTypeOtherSize,
			RoF::invtype::InvTypeOtherSize,
			RoF2::invtype::InvTypeOtherSize,
			EntityLimits::NPC::Null,
			EntityLimits::Merc::Null,
			EntityLimits::Bot::Null,
			EntityLimits::Pet::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			RoF::Null,
			RoF2::Null
		}
	};

	if (inv_type < 0 || inv_type >= legacy::TypeCount)
		return 0;

	return local[inv_type][static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

uint64 EQEmu::inventory::GetPossessionsBitmask(versions::InventoryVersion inventory_version)
{
	static const uint64 local[versions::InventoryVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		Titanium::Null, //0x000000027FDFFFFF,
		SoF::Null, //0x000000027FFFFFFF,
		SoD::Null, //0x000000027FFFFFFF,
		UF::Null, //0x000000027FFFFFFF,
		RoF::Null, //0x00000003FFFFFFFF,
		RoF2::Null, //0x00000003FFFFFFFF,
		EntityLimits::NPC::Null,
		EntityLimits::Merc::Null,
		EntityLimits::Bot::Null,
		EntityLimits::Pet::Null,
		Titanium::Null,
		SoF::Null,
		SoD::Null,
		UF::Null,
		RoF::Null,
		RoF2::Null
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::inventory::GetAllowEmptyBagInBag(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::False,
		Client62::False,
		Titanium::inventory::AllowEmptyBagInBag,
		SoF::inventory::AllowEmptyBagInBag,
		SoD::inventory::AllowEmptyBagInBag,
		UF::inventory::AllowEmptyBagInBag,
		RoF::False, //RoF::inventory::AllowEmptyBagInBag,
		RoF2::False, //RoF2::inventory::AllowEmptyBagInBag,
		EntityLimits::NPC::False,
		EntityLimits::Merc::False,
		EntityLimits::Bot::False,
		EntityLimits::Pet::False,
		Titanium::False,
		SoF::False,
		SoD::False,
		UF::False,
		RoF::False,
		RoF2::False
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::inventory::GetAllowClickCastFromBag(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::False,
		Client62::False,
		Titanium::inventory::AllowClickCastFromBag,
		SoF::inventory::AllowClickCastFromBag,
		SoD::inventory::AllowClickCastFromBag,
		UF::inventory::AllowClickCastFromBag,
		RoF::inventory::AllowClickCastFromBag,
		RoF2::inventory::AllowClickCastFromBag,
		EntityLimits::NPC::False,
		EntityLimits::Merc::False,
		EntityLimits::Bot::False,
		EntityLimits::Pet::False,
		Titanium::False,
		SoF::False,
		SoD::False,
		UF::False,
		RoF::False,
		RoF2::False
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::inventory::GetConcatenateInvTypeLimbo(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::False,
		Client62::False,
		Titanium::inventory::ConcatenateInvTypeLimbo,
		SoF::inventory::ConcatenateInvTypeLimbo,
		SoD::inventory::ConcatenateInvTypeLimbo,
		UF::inventory::ConcatenateInvTypeLimbo,
		RoF::inventory::ConcatenateInvTypeLimbo,
		RoF2::inventory::ConcatenateInvTypeLimbo,
		EntityLimits::NPC::False,
		EntityLimits::Merc::False,
		EntityLimits::Bot::False,
		EntityLimits::Pet::False,
		Titanium::False,
		SoF::False,
		SoD::False,
		UF::False,
		RoF::False,
		RoF2::False
	};
	
	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::inventory::GetAllowOverLevelEquipment(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::False,
		Client62::False,
		Titanium::inventory::AllowOverLevelEquipment,
		SoF::inventory::AllowOverLevelEquipment,
		SoD::inventory::AllowOverLevelEquipment,
		UF::inventory::AllowOverLevelEquipment,
		RoF::inventory::AllowOverLevelEquipment,
		RoF2::inventory::AllowOverLevelEquipment,
		EntityLimits::NPC::False,
		EntityLimits::Merc::False,
		EntityLimits::Bot::False,
		EntityLimits::Pet::False,
		Titanium::False,
		SoF::False,
		SoD::False,
		UF::False,
		RoF::False,
		RoF2::False
	};
	
	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

size_t EQEmu::inventory::GetItemAugSize(versions::InventoryVersion inventory_version)
{
	static const size_t local[versions::InventoryVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		legacy::ITEM_COMMON_SIZE, //Titanium::invaug::ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //SoF::invaug::ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //SoD::invaug::ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //UF::invaug::ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //RoF::invaug::ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //RoF2::invaug::ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //ItemAugSize,
		legacy::ITEM_COMMON_SIZE, //ItemAugSize,
		Titanium::Null,
		SoF::Null,
		SoD::Null,
		UF::Null,
		RoF::Null,
		RoF2::Null
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

size_t EQEmu::inventory::GetItemBagSize(versions::InventoryVersion inventory_version)
{
	static const size_t local[versions::InventoryVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		legacy::ITEM_CONTAINER_SIZE, //Titanium::invbag::ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //SoF::invbag::ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //SoD::invbag::ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //UF::invbag::ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //RoF::invbag::ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //RoF2::invbag::ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //ItemBagSize,
		legacy::ITEM_CONTAINER_SIZE, //Titanium::Null,
		legacy::ITEM_CONTAINER_SIZE, //SoF::Null,
		legacy::ITEM_CONTAINER_SIZE, //SoD::Null,
		legacy::ITEM_CONTAINER_SIZE, //UF::Null,
		legacy::ITEM_CONTAINER_SIZE, //RoF::Null,
		legacy::ITEM_CONTAINER_SIZE, //RoF2::Null
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::behavior::GetCoinHasWeight(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::True,
		Client62::True,
		Titanium::behavior::CoinHasWeight,
		SoF::behavior::CoinHasWeight,
		SoD::behavior::CoinHasWeight,
		UF::behavior::CoinHasWeight,
		RoF::behavior::CoinHasWeight,
		RoF::behavior::CoinHasWeight,
		EntityLimits::NPC::True, //CoinHasWeight,
		EntityLimits::Merc::True, //CoinHasWeight,
		EntityLimits::Bot::True, //CoinHasWeight,
		EntityLimits::Pet::True, //CoinHasWeight,
		Titanium::False,
		SoF::False,
		SoD::False,
		UF::False,
		RoF::False,
		RoF2::False
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}
