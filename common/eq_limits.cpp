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
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL
	},
	{
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL
	},
	{
		Titanium::constants::CHARACTER_CREATION_LIMIT,
		Titanium::constants::LongBuffs,
		Titanium::constants::ShortBuffs,
		Titanium::constants::DiscBuffs,
		Titanium::constants::TotalBuffs,
		Titanium::constants::NPCBuffs,
		Titanium::constants::PetBuffs,
		Titanium::constants::MercBuffs
	},
	{
		SoF::constants::CHARACTER_CREATION_LIMIT,
		SoF::constants::LongBuffs,
		SoF::constants::ShortBuffs,
		SoF::constants::DiscBuffs,
		SoF::constants::TotalBuffs,
		SoF::constants::NPCBuffs,
		SoF::constants::PetBuffs,
		SoF::constants::MercBuffs
	},
	{
		SoD::constants::CHARACTER_CREATION_LIMIT,
		SoD::constants::LongBuffs,
		SoD::constants::ShortBuffs,
		SoD::constants::DiscBuffs,
		SoD::constants::TotalBuffs,
		SoD::constants::NPCBuffs,
		SoD::constants::PetBuffs,
		SoD::constants::MercBuffs
	},
	{
		UF::constants::CHARACTER_CREATION_LIMIT,
		UF::constants::LongBuffs,
		UF::constants::ShortBuffs,
		UF::constants::DiscBuffs,
		UF::constants::TotalBuffs,
		UF::constants::NPCBuffs,
		UF::constants::PetBuffs,
		UF::constants::MercBuffs
	},
	{
		RoF::constants::CHARACTER_CREATION_LIMIT,
		RoF::constants::LongBuffs,
		RoF::constants::ShortBuffs,
		RoF::constants::DiscBuffs,
		RoF::constants::TotalBuffs,
		RoF::constants::NPCBuffs,
		RoF::constants::PetBuffs,
		RoF::constants::MercBuffs
	},
	{
		RoF2::constants::CHARACTER_CREATION_LIMIT,
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
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,

		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,
		ClientUnknown::INULL,

		false,
		false,
		false,
		false
	},
	{
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,

		Client62::INULL,
		Client62::INULL,
		Client62::INULL,
		Client62::INULL,

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		Titanium::invtype::BANK_SIZE,
		Titanium::invtype::SHARED_BANK_SIZE,
		Titanium::invtype::TRADE_SIZE,
		Titanium::invtype::WORLD_SIZE,
		Titanium::invtype::LIMBO_SIZE,
		Titanium::invtype::TRIBUTE_SIZE,
		Titanium::INULL,
		Titanium::invtype::GUILD_TRIBUTE_SIZE,
		Titanium::invtype::MERCHANT_SIZE,
		Titanium::INULL,
		Titanium::invtype::CORPSE_SIZE,
		Titanium::invtype::BAZAAR_SIZE,
		Titanium::invtype::INSPECT_SIZE,
		Titanium::INULL,
		Titanium::invtype::VIEW_MOD_PC_SIZE,
		Titanium::invtype::VIEW_MOD_BANK_SIZE,
		Titanium::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		Titanium::invtype::VIEW_MOD_LIMBO_SIZE,
		Titanium::invtype::ALT_STORAGE_SIZE,
		Titanium::invtype::ARCHIVED_SIZE,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::invtype::OTHER_SIZE,

		Titanium::invslot::POSSESSIONS_BITMASK,
		Titanium::invslot::CORPSE_BITMASK,
		Titanium::invbag::SLOT_COUNT,
		Titanium::invaug::SOCKET_COUNT,

		Titanium::inventory::AllowEmptyBagInBag,
		Titanium::inventory::AllowClickCastFromBag,
		Titanium::inventory::ConcatenateInvTypeLimbo,
		Titanium::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		SoF::invtype::BANK_SIZE,
		SoF::invtype::SHARED_BANK_SIZE,
		SoF::invtype::TRADE_SIZE,
		SoF::invtype::WORLD_SIZE,
		SoF::invtype::LIMBO_SIZE,
		SoF::invtype::TRIBUTE_SIZE,
		SoF::INULL,
		SoF::invtype::GUILD_TRIBUTE_SIZE,
		SoF::invtype::MERCHANT_SIZE,
		SoF::INULL,
		SoF::invtype::CORPSE_SIZE,
		SoF::invtype::BAZAAR_SIZE,
		SoF::invtype::INSPECT_SIZE,
		SoF::INULL,
		SoF::invtype::VIEW_MOD_PC_SIZE,
		SoF::invtype::VIEW_MOD_BANK_SIZE,
		SoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		SoF::invtype::VIEW_MOD_LIMBO_SIZE,
		SoF::invtype::ALT_STORAGE_SIZE,
		SoF::invtype::ARCHIVED_SIZE,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::invtype::OTHER_SIZE,

		SoF::invslot::POSSESSIONS_BITMASK,
		SoF::invslot::CORPSE_BITMASK,
		SoF::invbag::SLOT_COUNT,
		SoF::invaug::SOCKET_COUNT,

		SoF::inventory::AllowEmptyBagInBag,
		SoF::inventory::AllowClickCastFromBag,
		SoF::inventory::ConcatenateInvTypeLimbo,
		SoF::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		SoD::invtype::BANK_SIZE,
		SoD::invtype::SHARED_BANK_SIZE,
		SoD::invtype::TRADE_SIZE,
		SoD::invtype::WORLD_SIZE,
		SoD::invtype::LIMBO_SIZE,
		SoD::invtype::TRIBUTE_SIZE,
		SoD::INULL,
		SoD::invtype::GUILD_TRIBUTE_SIZE,
		SoD::invtype::MERCHANT_SIZE,
		SoD::INULL,
		SoD::invtype::CORPSE_SIZE,
		SoD::invtype::BAZAAR_SIZE,
		SoD::invtype::INSPECT_SIZE,
		SoD::INULL,
		SoD::invtype::VIEW_MOD_PC_SIZE,
		SoD::invtype::VIEW_MOD_BANK_SIZE,
		SoD::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		SoD::invtype::VIEW_MOD_LIMBO_SIZE,
		SoD::invtype::ALT_STORAGE_SIZE,
		SoD::invtype::ARCHIVED_SIZE,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::invtype::OTHER_SIZE,

		SoD::invslot::POSSESSIONS_BITMASK,
		SoD::invslot::CORPSE_BITMASK,
		SoD::invbag::SLOT_COUNT,
		SoD::invaug::SOCKET_COUNT,

		SoD::inventory::AllowEmptyBagInBag,
		SoD::inventory::AllowClickCastFromBag,
		SoD::inventory::ConcatenateInvTypeLimbo,
		SoD::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		UF::invtype::BANK_SIZE,
		UF::invtype::SHARED_BANK_SIZE,
		UF::invtype::TRADE_SIZE,
		UF::invtype::WORLD_SIZE,
		UF::invtype::LIMBO_SIZE,
		UF::invtype::TRIBUTE_SIZE,
		UF::INULL,
		UF::invtype::GUILD_TRIBUTE_SIZE,
		UF::invtype::MERCHANT_SIZE,
		UF::INULL,
		UF::invtype::CORPSE_SIZE,
		UF::invtype::BAZAAR_SIZE,
		UF::invtype::INSPECT_SIZE,
		UF::INULL,
		UF::invtype::VIEW_MOD_PC_SIZE,
		UF::invtype::VIEW_MOD_BANK_SIZE,
		UF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		UF::invtype::VIEW_MOD_LIMBO_SIZE,
		UF::invtype::ALT_STORAGE_SIZE,
		UF::invtype::ARCHIVED_SIZE,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::invtype::OTHER_SIZE,

		UF::invslot::POSSESSIONS_BITMASK,
		UF::invslot::CORPSE_BITMASK,
		UF::invbag::SLOT_COUNT,
		UF::invaug::SOCKET_COUNT,

		UF::inventory::AllowEmptyBagInBag,
		UF::inventory::AllowClickCastFromBag,
		UF::inventory::ConcatenateInvTypeLimbo,
		UF::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		RoF::invtype::BANK_SIZE,
		RoF::invtype::SHARED_BANK_SIZE,
		RoF::invtype::TRADE_SIZE,
		RoF::invtype::WORLD_SIZE,
		RoF::invtype::LIMBO_SIZE,
		RoF::invtype::TRIBUTE_SIZE,
		RoF::invtype::TROPHY_TRIBUTE_SIZE,
		RoF::invtype::GUILD_TRIBUTE_SIZE,
		RoF::invtype::MERCHANT_SIZE,
		RoF::invtype::DELETED_SIZE,
		RoF::invtype::CORPSE_SIZE,
		RoF::invtype::BAZAAR_SIZE,
		RoF::invtype::INSPECT_SIZE,
		RoF::invtype::REAL_ESTATE_SIZE,
		RoF::invtype::VIEW_MOD_PC_SIZE,
		RoF::invtype::VIEW_MOD_BANK_SIZE,
		RoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		RoF::invtype::VIEW_MOD_LIMBO_SIZE,
		RoF::invtype::ALT_STORAGE_SIZE,
		RoF::invtype::ARCHIVED_SIZE,
		RoF::invtype::MAIL_SIZE,
		RoF::invtype::GUILD_TROPHY_TRIBUTE_SIZE,
		RoF::INULL,
		RoF::invtype::OTHER_SIZE,

		RoF::invslot::POSSESSIONS_BITMASK,
		RoF::invslot::CORPSE_BITMASK,
		RoF::invbag::SLOT_COUNT,
		RoF::invaug::SOCKET_COUNT,

		RoF::inventory::AllowEmptyBagInBag,
		RoF::inventory::AllowClickCastFromBag,
		RoF::inventory::ConcatenateInvTypeLimbo,
		RoF::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		RoF2::invtype::BANK_SIZE,
		RoF2::invtype::SHARED_BANK_SIZE,
		RoF2::invtype::TRADE_SIZE,
		RoF2::invtype::WORLD_SIZE,
		RoF2::invtype::LIMBO_SIZE,
		RoF2::invtype::TRIBUTE_SIZE,
		RoF2::invtype::TROPHY_TRIBUTE_SIZE,
		RoF2::invtype::GUILD_TRIBUTE_SIZE,
		RoF2::invtype::MERCHANT_SIZE,
		RoF2::invtype::DELETED_SIZE,
		RoF2::invtype::CORPSE_SIZE,
		RoF2::invtype::BAZAAR_SIZE,
		RoF2::invtype::INSPECT_SIZE,
		RoF2::invtype::REAL_ESTATE_SIZE,
		RoF2::invtype::VIEW_MOD_PC_SIZE,
		RoF2::invtype::VIEW_MOD_BANK_SIZE,
		RoF2::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		RoF2::invtype::VIEW_MOD_LIMBO_SIZE,
		RoF2::invtype::ALT_STORAGE_SIZE,
		RoF2::invtype::ARCHIVED_SIZE,
		RoF2::invtype::MAIL_SIZE,
		RoF2::invtype::GUILD_TROPHY_TRIBUTE_SIZE,
		RoF2::invtype::KRONO_SIZE,
		RoF2::invtype::OTHER_SIZE,

		RoF2::invslot::POSSESSIONS_BITMASK,
		RoF2::invslot::CORPSE_BITMASK,
		RoF2::invbag::SLOT_COUNT,
		RoF2::invaug::SOCKET_COUNT,

		RoF2::inventory::AllowEmptyBagInBag,
		RoF2::inventory::AllowClickCastFromBag,
		RoF2::inventory::ConcatenateInvTypeLimbo,
		RoF2::inventory::AllowOverLevelEquipment
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::invtype::TRADE_SIZE,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,

		EntityLimits::NPC::INULL,
		EntityLimits::NPC::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::invtype::TRADE_SIZE,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,

		EntityLimits::NPCMerchant::INULL,
		EntityLimits::NPCMerchant::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::invtype::TRADE_SIZE,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,

		EntityLimits::Merc::INULL,
		EntityLimits::Merc::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::invtype::TRADE_SIZE,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL, 
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,
		EntityLimits::Bot::INULL,

		EntityLimits::Bot::invslot::POSSESSIONS_BITMASK,
		EntityLimits::Bot::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::invaug::SOCKET_COUNT, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::invtype::TRADE_SIZE,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,

		EntityLimits::ClientPet::INULL,
		EntityLimits::ClientPet::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::invtype::TRADE_SIZE,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,

		EntityLimits::NPCPet::INULL,
		EntityLimits::NPCPet::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::invtype::TRADE_SIZE,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,

		EntityLimits::MercPet::INULL,
		EntityLimits::MercPet::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{
		EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::invtype::TRADE_SIZE,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL, /*InvTypeCorpseSize,*/
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,

		EntityLimits::BotPet::INULL,
		EntityLimits::BotPet::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		0, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	},
	{ // OfflineTitanium
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::invtype::TRADE_SIZE,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::invtype::MERCHANT_SIZE,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::invtype::BAZAAR_SIZE,
		Titanium::invtype::INSPECT_SIZE,
		Titanium::INULL,
		Titanium::invtype::VIEW_MOD_PC_SIZE,
		Titanium::invtype::VIEW_MOD_BANK_SIZE,
		Titanium::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		Titanium::invtype::VIEW_MOD_LIMBO_SIZE,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,
		Titanium::INULL,

		Titanium::INULL,
		Titanium::INULL,
		Titanium::invbag::SLOT_COUNT,
		Titanium::invaug::SOCKET_COUNT,

		false,
		false,
		false,
		false
	},
	{ // OfflineSoF
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::invtype::TRADE_SIZE,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::invtype::MERCHANT_SIZE,
		SoF::INULL,
		SoF::INULL,
		SoF::invtype::BAZAAR_SIZE,
		SoF::invtype::INSPECT_SIZE,
		SoF::INULL,
		SoF::invtype::VIEW_MOD_PC_SIZE,
		SoF::invtype::VIEW_MOD_BANK_SIZE,
		SoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		SoF::invtype::VIEW_MOD_LIMBO_SIZE,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,
		SoF::INULL,

		SoF::INULL,
		SoF::INULL,
		SoF::invbag::SLOT_COUNT,
		SoF::invaug::SOCKET_COUNT,

		false,
		false,
		false,
		false
	},
	{ // OfflineSoD
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::invtype::TRADE_SIZE,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::invtype::MERCHANT_SIZE,
		SoD::INULL,
		SoD::INULL,
		SoD::invtype::BAZAAR_SIZE,
		SoD::invtype::INSPECT_SIZE,
		SoD::INULL,
		SoD::invtype::VIEW_MOD_PC_SIZE,
		SoD::invtype::VIEW_MOD_BANK_SIZE,
		SoD::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		SoD::invtype::VIEW_MOD_LIMBO_SIZE,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,
		SoD::INULL,

		SoD::INULL,
		SoD::INULL,
		SoD::invbag::SLOT_COUNT,
		SoD::invaug::SOCKET_COUNT,

		false,
		false,
		false,
		false
	},
	{ // OfflineUF
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::invtype::TRADE_SIZE,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::invtype::MERCHANT_SIZE,
		UF::INULL,
		UF::INULL,
		UF::invtype::BAZAAR_SIZE,
		UF::invtype::INSPECT_SIZE,
		UF::INULL,
		UF::invtype::VIEW_MOD_PC_SIZE,
		UF::invtype::VIEW_MOD_BANK_SIZE,
		UF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		UF::invtype::VIEW_MOD_LIMBO_SIZE,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::INULL,
		UF::INULL,

		UF::INULL,
		UF::INULL,
		UF::invbag::SLOT_COUNT,
		UF::invaug::SOCKET_COUNT,

		false,
		false,
		false,
		false
	},
	{ // OfflineRoF
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::invtype::TRADE_SIZE,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::invtype::MERCHANT_SIZE,
		RoF::INULL,
		RoF::INULL,
		RoF::invtype::BAZAAR_SIZE,
		RoF::invtype::INSPECT_SIZE,
		RoF::INULL,
		RoF::invtype::VIEW_MOD_PC_SIZE,
		RoF::invtype::VIEW_MOD_BANK_SIZE,
		RoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		RoF::invtype::VIEW_MOD_LIMBO_SIZE,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,
		RoF::INULL,

		RoF::INULL,
		RoF::INULL,
		RoF::invbag::SLOT_COUNT,
		RoF::invaug::SOCKET_COUNT,

		false,
		false,
		false,
		false
	},
	{ // OfflineRoF2
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::invtype::TRADE_SIZE,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::invtype::MERCHANT_SIZE,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::invtype::BAZAAR_SIZE,
		RoF2::invtype::INSPECT_SIZE,
		RoF2::INULL,
		RoF2::invtype::VIEW_MOD_PC_SIZE,
		RoF2::invtype::VIEW_MOD_BANK_SIZE,
		RoF2::invtype::VIEW_MOD_SHARED_BANK_SIZE,
		RoF2::invtype::VIEW_MOD_LIMBO_SIZE,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,
		RoF2::INULL,

		RoF2::INULL,
		RoF2::INULL,
		RoF2::invbag::SLOT_COUNT,
		RoF2::invaug::SOCKET_COUNT,

		false,
		false,
		false,
		false
	}
};

const EQEmu::inventory::LookupEntry* EQEmu::inventory::Lookup(versions::MobVersion mob_version)
{
	return &inventory_lookup_entries[static_cast<int>(versions::ValidateMobVersion(mob_version))];
}

static const EQEmu::behavior::LookupEntry behavior_lookup_entries[EQEmu::versions::MobVersionCount] =
{
	{ // Unknown
		true
	},
	{ // Client62
		true
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
		EQEmu::behavior::CoinHasWeight
	},
	{ // NPCMerchant
		EQEmu::behavior::CoinHasWeight
	},
	{ // Merc
		EQEmu::behavior::CoinHasWeight
	},
	{ // Bot
		EQEmu::behavior::CoinHasWeight
	},
	{ // ClientPet
		EQEmu::behavior::CoinHasWeight
	},
	{ // NPCPet
		EQEmu::behavior::CoinHasWeight
	},
	{ // MercPet
		EQEmu::behavior::CoinHasWeight
	},
	{ // BotPet
		EQEmu::behavior::CoinHasWeight
	},
	{ // OfflineTitanium
		Titanium::behavior::CoinHasWeight
	},
	{ // OfflineSoF
		SoF::behavior::CoinHasWeight
	},
	{ // OfflineSoD
		SoD::behavior::CoinHasWeight
	},
	{ // OfflineUF
		UF::behavior::CoinHasWeight
	},
	{ // OfflineRoF
		RoF::behavior::CoinHasWeight
	},
	{ // OfflineRoF2
		RoF2::behavior::CoinHasWeight
	}
};

const EQEmu::behavior::LookupEntry* EQEmu::behavior::Lookup(versions::MobVersion mob_version)
{
	return &behavior_lookup_entries[static_cast<int>(versions::ValidateMobVersion(mob_version))];
}
