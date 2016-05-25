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

#include "emu_constants.h"
#include "emu_limits.h"


size_t EQEmu::constants::CharacterCreationLimit(versions::ClientVersion client_version)
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

uint16 EQEmu::inventory::InventoryTypeSize(versions::InventoryVersion inventory_version, int16 inv_type)
{
	static const uint16 local[legacy::TypeCount][versions::InventoryVersionCount] = {
		{ // local[TypePossessions]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE,
			legacy::TYPE_POSSESSIONS_SIZE
		},
		{ // local[TypeBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::invtype::InvTypeBankSize,
			legacy::TYPE_BANK_SIZE,
			legacy::TYPE_BANK_SIZE,
			legacy::TYPE_BANK_SIZE,
			legacy::TYPE_BANK_SIZE,
			legacy::TYPE_BANK_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeSharedBank]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_SHARED_BANK_SIZE,
			legacy::TYPE_SHARED_BANK_SIZE,
			legacy::TYPE_SHARED_BANK_SIZE,
			legacy::TYPE_SHARED_BANK_SIZE,
			legacy::TYPE_SHARED_BANK_SIZE,
			legacy::TYPE_SHARED_BANK_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeTrade]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_TRADE_SIZE,
			legacy::TYPE_TRADE_SIZE,
			legacy::TYPE_TRADE_SIZE,
			legacy::TYPE_TRADE_SIZE,
			legacy::TYPE_TRADE_SIZE,
			legacy::TYPE_TRADE_SIZE,
			4,
			4,
			legacy::TYPE_TRADE_SIZE, // client thinks this is another client
			4
		},
		{ // local[TypeWorld]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_WORLD_SIZE,
			legacy::TYPE_WORLD_SIZE,
			legacy::TYPE_WORLD_SIZE,
			legacy::TYPE_WORLD_SIZE,
			legacy::TYPE_WORLD_SIZE,
			legacy::TYPE_WORLD_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeLimbo]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_LIMBO_SIZE,
			legacy::TYPE_LIMBO_SIZE,
			legacy::TYPE_LIMBO_SIZE,
			legacy::TYPE_LIMBO_SIZE,
			legacy::TYPE_LIMBO_SIZE,
			legacy::TYPE_LIMBO_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeTribute]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_TRIBUTE_SIZE,
			legacy::TYPE_TRIBUTE_SIZE,
			legacy::TYPE_TRIBUTE_SIZE,
			legacy::TYPE_TRIBUTE_SIZE,
			legacy::TYPE_TRIBUTE_SIZE,
			legacy::TYPE_TRIBUTE_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeTrophyTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_TROPHY_TRIBUTE_SIZE,
			legacy::TYPE_TROPHY_TRIBUTE_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeGuildTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_GUILD_TRIBUTE_SIZE,
			legacy::TYPE_GUILD_TRIBUTE_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeMerchant]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_MERCHANT_SIZE,
			legacy::TYPE_MERCHANT_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeDeleted]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_DELETED_SIZE,
			legacy::TYPE_DELETED_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
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
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeBazaar]
			ClientUnknown::Null,
			Client62::Null,
			legacy::TYPE_BAZAAR_SIZE,
			legacy::TYPE_BAZAAR_SIZE,
			legacy::TYPE_BAZAAR_SIZE,
			legacy::TYPE_BAZAAR_SIZE,
			legacy::TYPE_BAZAAR_SIZE,
			legacy::TYPE_BAZAAR_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
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
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeRealEstate]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_REAL_ESTATE_SIZE,
			legacy::TYPE_REAL_ESTATE_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeViewMODPC]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_VIEW_MOD_PC_SIZE,
			legacy::TYPE_VIEW_MOD_PC_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeViewMODBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_VIEW_MOD_BANK_SIZE,
			legacy::TYPE_VIEW_MOD_BANK_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeViewMODSharedBank]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_VIEW_MOD_SHARED_BANK_SIZE,
			legacy::TYPE_VIEW_MOD_SHARED_BANK_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeViewMODLimbo]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_VIEW_MOD_LIMBO_SIZE,
			legacy::TYPE_VIEW_MOD_LIMBO_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeAltStorage]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_ALT_STORAGE_SIZE,
			legacy::TYPE_ALT_STORAGE_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeArchived]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_ARCHIVED_SIZE,
			legacy::TYPE_ARCHIVED_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeMail]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_MAIL_SIZE,
			legacy::TYPE_MAIL_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeGuildTrophyTribute]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_GUILD_TROPHY_TRIBUTE_SIZE,
			legacy::TYPE_GUILD_TROPHY_TRIBUTE_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeKrono]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_KRONO_SIZE,
			legacy::TYPE_KRONO_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		},
		{ // local[TypeOther]
			ClientUnknown::Null,
			Client62::Null,
			Titanium::Null,
			SoF::Null,
			SoD::Null,
			UF::Null,
			legacy::TYPE_OTHER_SIZE,
			legacy::TYPE_OTHER_SIZE,
			EntityLimits::npc::Null,
			EntityLimits::merc::Null,
			EntityLimits::bot::Null,
			EntityLimits::pet::Null
		}
	};

	if ((uint16)inv_type < legacy::TypeCount)
		return local[inv_type][static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];

	return NOT_USED;
}

uint64 EQEmu::inventory::PossessionsBitmask(versions::InventoryVersion inventory_version)
{
	static const uint64 local[versions::InventoryVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		0x000000027FDFFFFF,
		0x000000027FFFFFFF,
		0x000000027FFFFFFF,
		0x000000027FFFFFFF,
		0x00000003FFFFFFFF,
		0x00000003FFFFFFFF,
		EntityLimits::npc::Null,
		EntityLimits::merc::Null,
		EntityLimits::bot::Null,
		EntityLimits::pet::Null
	};

	return NOT_USED;
	//return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::inventory::AllowEmptyBagInBag(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::False,
		Client62::False,
		Titanium::behavior::AllowEmptyBagInBag,
		SoF::behavior::AllowEmptyBagInBag,
		SoD::behavior::AllowEmptyBagInBag,
		UF::behavior::AllowEmptyBagInBag,
		RoF::behavior::AllowEmptyBagInBag,
		RoF2::behavior::AllowEmptyBagInBag,
		EntityLimits::npc::False,
		EntityLimits::merc::False,
		EntityLimits::bot::False,
		EntityLimits::pet::False
	};

	return false;
	//return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::inventory::AllowClickCastFromBag(versions::InventoryVersion inventory_version)
{
	static const bool local[versions::InventoryVersionCount] = {
		ClientUnknown::False,
		Client62::False,
		Titanium::behavior::AllowClickCastFromBag,
		SoF::behavior::AllowClickCastFromBag,
		SoD::behavior::AllowClickCastFromBag,
		UF::behavior::AllowClickCastFromBag,
		RoF::behavior::AllowClickCastFromBag,
		RoF2::behavior::AllowClickCastFromBag,
		EntityLimits::npc::False,
		EntityLimits::merc::False,
		EntityLimits::bot::False,
		EntityLimits::pet::False
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

uint16 EQEmu::inventory::ItemAugSize(versions::InventoryVersion inventory_version)
{
	static const uint16 local[versions::InventoryVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE,
		legacy::ITEM_COMMON_SIZE
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

uint16 EQEmu::inventory::ItemBagSize(versions::InventoryVersion inventory_version)
{
	static const uint16 local[versions::InventoryVersionCount] = {
		ClientUnknown::Null,
		Client62::Null,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE,
		legacy::ITEM_CONTAINER_SIZE
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}

bool EQEmu::profile::CoinHasWeight(versions::InventoryVersion inventory_version)
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
		EntityLimits::npc::True,
		EntityLimits::merc::True,
		EntityLimits::bot::True,
		EntityLimits::pet::True
	};

	return local[static_cast<size_t>(versions::ValidateInventoryVersion(inventory_version))];
}
