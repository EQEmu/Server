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
#include "rulesys.h"

#include <memory>


static bool global_dictionary_init = false;
void EQEmu::InitializeDynamicLookups() {
	if (global_dictionary_init == true)
		return;

	constants::InitializeDynamicLookups();
	inventory::InitializeDynamicLookups();
	behavior::InitializeDynamicLookups();

	global_dictionary_init = true;
}

static std::unique_ptr<EQEmu::constants::LookupEntry> constants_dynamic_lookup_entries[EQEmu::versions::ClientVersionCount];
static const EQEmu::constants::LookupEntry constants_static_lookup_entries[EQEmu::versions::ClientVersionCount] =
{
	/*[ClientVersion::Unknown] =*/
	EQEmu::constants::LookupEntry(
		EQEmu::expansions::Expansion::EverQuest,
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
		ClientUnknown::INULL
	),
	/*[ClientVersion::Client62] =*/
	EQEmu::constants::LookupEntry(
		EQEmu::expansions::Expansion::EverQuest,
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
		Client62::INULL
	),
	/*[ClientVersion::Titanium] =*/
	EQEmu::constants::LookupEntry(
		Titanium::constants::EXPANSION,
		Titanium::constants::EXPANSION_BIT,
		Titanium::constants::EXPANSIONS_MASK,
		Titanium::constants::CHARACTER_CREATION_LIMIT,
		Titanium::constants::SAY_LINK_BODY_SIZE,
		Titanium::constants::LongBuffs,
		Titanium::constants::ShortBuffs,
		Titanium::constants::DiscBuffs,
		Titanium::constants::TotalBuffs,
		Titanium::constants::NPCBuffs,
		Titanium::constants::PetBuffs,
		Titanium::constants::MercBuffs
	),
	/*[ClientVersion::SoF] =*/
	EQEmu::constants::LookupEntry(
		SoF::constants::EXPANSION,
		SoF::constants::EXPANSION_BIT,
		SoF::constants::EXPANSIONS_MASK,
		SoF::constants::CHARACTER_CREATION_LIMIT,
		SoF::constants::SAY_LINK_BODY_SIZE,
		SoF::constants::LongBuffs,
		SoF::constants::ShortBuffs,
		SoF::constants::DiscBuffs,
		SoF::constants::TotalBuffs,
		SoF::constants::NPCBuffs,
		SoF::constants::PetBuffs,
		SoF::constants::MercBuffs
	),
	/*[ClientVersion::SoD] =*/
	EQEmu::constants::LookupEntry(
		SoD::constants::EXPANSION,
		SoD::constants::EXPANSION_BIT,
		SoD::constants::EXPANSIONS_MASK,
		SoD::constants::CHARACTER_CREATION_LIMIT,
		SoD::constants::SAY_LINK_BODY_SIZE,
		SoD::constants::LongBuffs,
		SoD::constants::ShortBuffs,
		SoD::constants::DiscBuffs,
		SoD::constants::TotalBuffs,
		SoD::constants::NPCBuffs,
		SoD::constants::PetBuffs,
		SoD::constants::MercBuffs
	),
	/*[ClientVersion::UF] =*/
	EQEmu::constants::LookupEntry(
		UF::constants::EXPANSION,
		UF::constants::EXPANSION_BIT,
		UF::constants::EXPANSIONS_MASK,
		UF::constants::CHARACTER_CREATION_LIMIT,
		UF::constants::SAY_LINK_BODY_SIZE,
		UF::constants::LongBuffs,
		UF::constants::ShortBuffs,
		UF::constants::DiscBuffs,
		UF::constants::TotalBuffs,
		UF::constants::NPCBuffs,
		UF::constants::PetBuffs,
		UF::constants::MercBuffs
	),
	/*[ClientVersion::RoF] =*/
	EQEmu::constants::LookupEntry(
		RoF::constants::EXPANSION,
		RoF::constants::EXPANSION_BIT,
		RoF::constants::EXPANSIONS_MASK,
		RoF::constants::CHARACTER_CREATION_LIMIT,
		RoF::constants::SAY_LINK_BODY_SIZE,
		RoF::constants::LongBuffs,
		RoF::constants::ShortBuffs,
		RoF::constants::DiscBuffs,
		RoF::constants::TotalBuffs,
		RoF::constants::NPCBuffs,
		RoF::constants::PetBuffs,
		RoF::constants::MercBuffs
	),
	/*[ClientVersion::RoF2] =*/
	EQEmu::constants::LookupEntry(
		RoF2::constants::EXPANSION,
		RoF2::constants::EXPANSION_BIT,
		RoF2::constants::EXPANSIONS_MASK,
		RoF2::constants::CHARACTER_CREATION_LIMIT,
		RoF2::constants::SAY_LINK_BODY_SIZE,
		RoF2::constants::LongBuffs,
		RoF2::constants::ShortBuffs,
		RoF2::constants::DiscBuffs,
		RoF2::constants::TotalBuffs,
		RoF2::constants::NPCBuffs,
		RoF2::constants::PetBuffs,
		RoF2::constants::MercBuffs
	)
};

static bool constants_dictionary_init = false;
void EQEmu::constants::InitializeDynamicLookups() {
	if (constants_dictionary_init == true)
		return;
	constants_dictionary_init = true;

	if (RuleB(World, UseClientBasedExpansionSettings))
		return;

	// use static references for now
}

const EQEmu::constants::LookupEntry* EQEmu::constants::DynamicLookup(versions::ClientVersion client_version)
{
	client_version = versions::ValidateClientVersion(client_version);
	if (constants_dynamic_lookup_entries[static_cast<int>(client_version)])
		return constants_dynamic_lookup_entries[static_cast<int>(client_version)].get();

	return &constants_static_lookup_entries[static_cast<int>(client_version)];
}

const EQEmu::constants::LookupEntry* EQEmu::constants::StaticLookup(versions::ClientVersion client_version)
{
	return &constants_static_lookup_entries[static_cast<int>(versions::ValidateClientVersion(client_version))];
}

static std::unique_ptr<EQEmu::inventory::LookupEntry> inventory_dynamic_lookup_entries[EQEmu::versions::MobVersionCount];
static const EQEmu::inventory::LookupEntry inventory_static_lookup_entries[EQEmu::versions::MobVersionCount] =
{
	/*[MobVersion::Unknown] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL, ClientUnknown::INULL, ClientUnknown::INULL,
			ClientUnknown::INULL
		),
		
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
	),
	/*[MobVersion::Client62] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL, Client62::INULL, Client62::INULL,
			Client62::INULL
		),
		
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
	),
	/*[MobVersion::Titanium] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,		Titanium::invtype::BANK_SIZE,			Titanium::invtype::SHARED_BANK_SIZE,
			Titanium::invtype::TRADE_SIZE,			Titanium::invtype::WORLD_SIZE,			Titanium::invtype::LIMBO_SIZE,
			Titanium::invtype::TRIBUTE_SIZE,		Titanium::INULL,						Titanium::invtype::GUILD_TRIBUTE_SIZE,
			Titanium::invtype::MERCHANT_SIZE,		Titanium::INULL,						Titanium::invtype::CORPSE_SIZE,
			Titanium::invtype::BAZAAR_SIZE,			Titanium::invtype::INSPECT_SIZE,		Titanium::INULL,
			Titanium::invtype::VIEW_MOD_PC_SIZE,	Titanium::invtype::VIEW_MOD_BANK_SIZE,	Titanium::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			Titanium::invtype::VIEW_MOD_LIMBO_SIZE,	Titanium::invtype::ALT_STORAGE_SIZE,	Titanium::invtype::ARCHIVED_SIZE,
			Titanium::INULL,						Titanium::INULL,						Titanium::INULL,
			Titanium::invtype::OTHER_SIZE
		),
		
		Titanium::invslot::EQUIPMENT_BITMASK,
		Titanium::invslot::GENERAL_BITMASK,
		Titanium::invslot::CURSOR_BITMASK,
		Titanium::invslot::POSSESSIONS_BITMASK,
		Titanium::invslot::CORPSE_BITMASK,
		Titanium::invbag::SLOT_COUNT,
		Titanium::invaug::SOCKET_COUNT,
		
		Titanium::inventory::AllowEmptyBagInBag,
		Titanium::inventory::AllowClickCastFromBag,
		Titanium::inventory::ConcatenateInvTypeLimbo,
		Titanium::inventory::AllowOverLevelEquipment
	),
	/*[MobVersion::SoF] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,	SoF::invtype::BANK_SIZE,			SoF::invtype::SHARED_BANK_SIZE,
			SoF::invtype::TRADE_SIZE,			SoF::invtype::WORLD_SIZE,			SoF::invtype::LIMBO_SIZE,
			SoF::invtype::TRIBUTE_SIZE,			SoF::INULL,							SoF::invtype::GUILD_TRIBUTE_SIZE,
			SoF::invtype::MERCHANT_SIZE,		SoF::INULL,							SoF::invtype::CORPSE_SIZE,
			SoF::invtype::BAZAAR_SIZE,			SoF::invtype::INSPECT_SIZE,			SoF::INULL,
			SoF::invtype::VIEW_MOD_PC_SIZE,		SoF::invtype::VIEW_MOD_BANK_SIZE,	SoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			SoF::invtype::VIEW_MOD_LIMBO_SIZE,	SoF::invtype::ALT_STORAGE_SIZE,		SoF::invtype::ARCHIVED_SIZE,
			SoF::INULL,							SoF::INULL,							SoF::INULL,
			SoF::invtype::OTHER_SIZE
		),
		
		SoF::invslot::EQUIPMENT_BITMASK,
		SoF::invslot::GENERAL_BITMASK,
		SoF::invslot::CURSOR_BITMASK,
		SoF::invslot::POSSESSIONS_BITMASK,
		SoF::invslot::CORPSE_BITMASK,
		SoF::invbag::SLOT_COUNT,
		SoF::invaug::SOCKET_COUNT,
		
		SoF::inventory::AllowEmptyBagInBag,
		SoF::inventory::AllowClickCastFromBag,
		SoF::inventory::ConcatenateInvTypeLimbo,
		SoF::inventory::AllowOverLevelEquipment
	),
	/*[MobVersion::SoD] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,	SoD::invtype::BANK_SIZE,			SoD::invtype::SHARED_BANK_SIZE,
			SoD::invtype::TRADE_SIZE,			SoD::invtype::WORLD_SIZE,			SoD::invtype::LIMBO_SIZE,
			SoD::invtype::TRIBUTE_SIZE,			SoD::INULL,							SoD::invtype::GUILD_TRIBUTE_SIZE,
			SoD::invtype::MERCHANT_SIZE,		SoD::INULL,							SoD::invtype::CORPSE_SIZE,
			SoD::invtype::BAZAAR_SIZE,			SoD::invtype::INSPECT_SIZE,			SoD::INULL,
			SoD::invtype::VIEW_MOD_PC_SIZE,		SoD::invtype::VIEW_MOD_BANK_SIZE,	SoD::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			SoD::invtype::VIEW_MOD_LIMBO_SIZE,	SoD::invtype::ALT_STORAGE_SIZE,		SoD::invtype::ARCHIVED_SIZE,
			SoD::INULL,							SoD::INULL,							SoD::INULL,
			SoD::invtype::OTHER_SIZE
		),

		SoD::invslot::EQUIPMENT_BITMASK,
		SoD::invslot::GENERAL_BITMASK,
		SoD::invslot::CURSOR_BITMASK,
		SoD::invslot::POSSESSIONS_BITMASK,
		SoD::invslot::CORPSE_BITMASK,
		SoD::invbag::SLOT_COUNT,
		SoD::invaug::SOCKET_COUNT,

		SoD::inventory::AllowEmptyBagInBag,
		SoD::inventory::AllowClickCastFromBag,
		SoD::inventory::ConcatenateInvTypeLimbo,
		SoD::inventory::AllowOverLevelEquipment
	),
	/*[MobVersion::UF] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,	UF::invtype::BANK_SIZE,				UF::invtype::SHARED_BANK_SIZE,
			UF::invtype::TRADE_SIZE,			UF::invtype::WORLD_SIZE,			UF::invtype::LIMBO_SIZE,
			UF::invtype::TRIBUTE_SIZE,			UF::INULL,							UF::invtype::GUILD_TRIBUTE_SIZE,
			UF::invtype::MERCHANT_SIZE,			UF::INULL,							UF::invtype::CORPSE_SIZE,
			UF::invtype::BAZAAR_SIZE,			UF::invtype::INSPECT_SIZE,			UF::INULL,
			UF::invtype::VIEW_MOD_PC_SIZE,		UF::invtype::VIEW_MOD_BANK_SIZE,	UF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			UF::invtype::VIEW_MOD_LIMBO_SIZE,	UF::invtype::ALT_STORAGE_SIZE,		UF::invtype::ARCHIVED_SIZE,
			UF::INULL,							UF::INULL,							UF::INULL,
			UF::invtype::OTHER_SIZE
		),

		UF::invslot::EQUIPMENT_BITMASK,
		UF::invslot::GENERAL_BITMASK,
		UF::invslot::CURSOR_BITMASK,
		UF::invslot::POSSESSIONS_BITMASK,
		UF::invslot::CORPSE_BITMASK,
		UF::invbag::SLOT_COUNT,
		UF::invaug::SOCKET_COUNT,

		UF::inventory::AllowEmptyBagInBag,
		UF::inventory::AllowClickCastFromBag,
		UF::inventory::ConcatenateInvTypeLimbo,
		UF::inventory::AllowOverLevelEquipment
	),
	/*[MobVersion::RoF] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,	RoF::invtype::BANK_SIZE,					RoF::invtype::SHARED_BANK_SIZE,
			RoF::invtype::TRADE_SIZE,			RoF::invtype::WORLD_SIZE,					RoF::invtype::LIMBO_SIZE,
			RoF::invtype::TRIBUTE_SIZE,			RoF::invtype::TROPHY_TRIBUTE_SIZE,			RoF::invtype::GUILD_TRIBUTE_SIZE,
			RoF::invtype::MERCHANT_SIZE,		RoF::invtype::DELETED_SIZE,					RoF::invtype::CORPSE_SIZE,
			RoF::invtype::BAZAAR_SIZE,			RoF::invtype::INSPECT_SIZE,					RoF::invtype::REAL_ESTATE_SIZE,
			RoF::invtype::VIEW_MOD_PC_SIZE,		RoF::invtype::VIEW_MOD_BANK_SIZE,			RoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			RoF::invtype::VIEW_MOD_LIMBO_SIZE,	RoF::invtype::ALT_STORAGE_SIZE,				RoF::invtype::ARCHIVED_SIZE,
			RoF::invtype::MAIL_SIZE,			RoF::invtype::GUILD_TROPHY_TRIBUTE_SIZE,	RoF::INULL,
			RoF::invtype::OTHER_SIZE
		),

		RoF::invslot::EQUIPMENT_BITMASK,
		RoF::invslot::GENERAL_BITMASK,
		RoF::invslot::CURSOR_BITMASK,
		RoF::invslot::POSSESSIONS_BITMASK,
		RoF::invslot::CORPSE_BITMASK,
		RoF::invbag::SLOT_COUNT,
		RoF::invaug::SOCKET_COUNT,

		RoF::inventory::AllowEmptyBagInBag,
		RoF::inventory::AllowClickCastFromBag,
		RoF::inventory::ConcatenateInvTypeLimbo,
		RoF::inventory::AllowOverLevelEquipment
	),
	/*[MobVersion::RoF2] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,	RoF2::invtype::BANK_SIZE,					RoF2::invtype::SHARED_BANK_SIZE,
			RoF2::invtype::TRADE_SIZE,			RoF2::invtype::WORLD_SIZE,					RoF2::invtype::LIMBO_SIZE,
			RoF2::invtype::TRIBUTE_SIZE,		RoF2::invtype::TROPHY_TRIBUTE_SIZE,			RoF2::invtype::GUILD_TRIBUTE_SIZE,
			RoF2::invtype::MERCHANT_SIZE,		RoF2::invtype::DELETED_SIZE,				RoF2::invtype::CORPSE_SIZE,
			RoF2::invtype::BAZAAR_SIZE,			RoF2::invtype::INSPECT_SIZE,				RoF2::invtype::REAL_ESTATE_SIZE,
			RoF2::invtype::VIEW_MOD_PC_SIZE,	RoF2::invtype::VIEW_MOD_BANK_SIZE,			RoF2::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			RoF2::invtype::VIEW_MOD_LIMBO_SIZE,	RoF2::invtype::ALT_STORAGE_SIZE,			RoF2::invtype::ARCHIVED_SIZE,
			RoF2::invtype::MAIL_SIZE,			RoF2::invtype::GUILD_TROPHY_TRIBUTE_SIZE,	RoF2::invtype::KRONO_SIZE,
			RoF2::invtype::OTHER_SIZE
		),

		RoF2::invslot::EQUIPMENT_BITMASK,
		RoF2::invslot::GENERAL_BITMASK,
		RoF2::invslot::CURSOR_BITMASK,
		RoF2::invslot::POSSESSIONS_BITMASK,
		RoF2::invslot::CORPSE_BITMASK,
		RoF2::invbag::SLOT_COUNT,
		RoF2::invaug::SOCKET_COUNT,

		RoF2::inventory::AllowEmptyBagInBag,
		RoF2::inventory::AllowClickCastFromBag,
		RoF2::inventory::ConcatenateInvTypeLimbo,
		RoF2::inventory::AllowOverLevelEquipment
	),
	/*[MobVersion::NPC] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE,		EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::invtype::TRADE_SIZE,	EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::INULL,				EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::INULL,				EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::NPC::INULL,				EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::INULL,				EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::INULL,				EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::INULL,				EntityLimits::NPC::INULL,	EntityLimits::NPC::INULL,
			EntityLimits::NPC::INULL
		),

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
	),
	/*[MobVersion::NPCMerchant] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::invtype::TRADE_SIZE,					EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::INULL,								EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::INULL,								EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::NPCMerchant::INULL,								EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::INULL,								EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::INULL,								EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::INULL,								EntityLimits::NPCMerchant::INULL,	EntityLimits::NPCMerchant::INULL,
			EntityLimits::NPCMerchant::INULL
		),

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
	),
	/*[MobVersion::Merc] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::invtype::TRADE_SIZE,						EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::INULL,										EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::INULL,										EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::Merc::INULL,										EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::INULL,										EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::INULL,										EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::INULL,										EntityLimits::Merc::INULL,	EntityLimits::Merc::INULL,
			EntityLimits::Merc::INULL
		),

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
	),
	/*[MobVersion::Bot] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::invtype::TRADE_SIZE,							EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::INULL,										EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::INULL,										EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::Bot::INULL,										EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::INULL,										EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::INULL,										EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::INULL,										EntityLimits::Bot::INULL,	EntityLimits::Bot::INULL,
			EntityLimits::Bot::INULL
		),

		EntityLimits::Bot::invslot::EQUIPMENT_BITMASK,
		EntityLimits::Bot::invslot::GENERAL_BITMASK,
		EntityLimits::Bot::invslot::CURSOR_BITMASK,
		EntityLimits::Bot::invslot::POSSESSIONS_BITMASK,
		EntityLimits::Bot::INULL,
		0, //EQEmu::inventory::ContainerCount, /*ItemBagSize,*/
		EQEmu::invaug::SOCKET_COUNT, //EQEmu::inventory::SocketCount, /*ItemAugSize,*/

		false,
		false,
		false,
		false
	),
	/*[MobVersion::ClientPet] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::invtype::TRADE_SIZE,					EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::INULL,									EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::INULL,									EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::ClientPet::INULL,									EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::INULL,									EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::INULL,									EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::INULL,									EntityLimits::ClientPet::INULL,	EntityLimits::ClientPet::INULL,
			EntityLimits::ClientPet::INULL
		),

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
	),
	/*[MobVersion::NPCPet] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::invtype::TRADE_SIZE,						EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::INULL,									EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::INULL,									EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::NPCPet::INULL,									EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::INULL,									EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::INULL,									EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::INULL,									EntityLimits::NPCPet::INULL,	EntityLimits::NPCPet::INULL,
			EntityLimits::NPCPet::INULL
		),

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
	),
	/*[MobVersion::MercPet] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::invtype::TRADE_SIZE,						EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::INULL,									EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::INULL,									EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::MercPet::INULL,									EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::INULL,									EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::INULL,									EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::INULL,									EntityLimits::MercPet::INULL,	EntityLimits::MercPet::INULL,
			EntityLimits::MercPet::INULL
		),

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
	),
	/*[MobVersion::BotPet] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			EQEmu::invtype::POSSESSIONS_SIZE, /*InvTypePossessionsSize,*/	EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::invtype::TRADE_SIZE,						EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::INULL,									EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::INULL,									EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL, /*InvTypeCorpseSize,*/
			EntityLimits::BotPet::INULL,									EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::INULL,									EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::INULL,									EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::INULL,									EntityLimits::BotPet::INULL,	EntityLimits::BotPet::INULL,
			EntityLimits::BotPet::INULL
		),

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
	),
	/*[MobVersion::OfflineTitanium] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			Titanium::INULL,						Titanium::INULL,						Titanium::INULL,
			Titanium::invtype::TRADE_SIZE,			Titanium::INULL,						Titanium::INULL,
			Titanium::INULL,						Titanium::INULL,						Titanium::INULL,
			Titanium::invtype::MERCHANT_SIZE,		Titanium::INULL,						Titanium::INULL,
			Titanium::invtype::BAZAAR_SIZE,			Titanium::invtype::INSPECT_SIZE,		Titanium::INULL,
			Titanium::invtype::VIEW_MOD_PC_SIZE,	Titanium::invtype::VIEW_MOD_BANK_SIZE,	Titanium::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			Titanium::invtype::VIEW_MOD_LIMBO_SIZE,	Titanium::INULL,						Titanium::INULL,
			Titanium::INULL,						Titanium::INULL,						Titanium::INULL,
			Titanium::INULL
		),

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
	),
	/*[MobVersion::OfflineSoF] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			SoF::INULL,							SoF::INULL,							SoF::INULL,
			SoF::invtype::TRADE_SIZE,			SoF::INULL,							SoF::INULL,
			SoF::INULL,							SoF::INULL,							SoF::INULL,
			SoF::invtype::MERCHANT_SIZE,		SoF::INULL,							SoF::INULL,
			SoF::invtype::BAZAAR_SIZE,			SoF::invtype::INSPECT_SIZE,			SoF::INULL,
			SoF::invtype::VIEW_MOD_PC_SIZE,		SoF::invtype::VIEW_MOD_BANK_SIZE,	SoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			SoF::invtype::VIEW_MOD_LIMBO_SIZE,	SoF::INULL,							SoF::INULL,
			SoF::INULL,							SoF::INULL,							SoF::INULL,
			SoF::INULL
		),

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
	),
	/*[MobVersion::OfflineSoD] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			SoD::INULL,							SoD::INULL,							SoD::INULL,
			SoD::invtype::TRADE_SIZE,			SoD::INULL,							SoD::INULL,
			SoD::INULL,							SoD::INULL,							SoD::INULL,
			SoD::invtype::MERCHANT_SIZE,		SoD::INULL,							SoD::INULL,
			SoD::invtype::BAZAAR_SIZE,			SoD::invtype::INSPECT_SIZE,			SoD::INULL,
			SoD::invtype::VIEW_MOD_PC_SIZE,		SoD::invtype::VIEW_MOD_BANK_SIZE,	SoD::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			SoD::invtype::VIEW_MOD_LIMBO_SIZE,	SoD::INULL,							SoD::INULL,
			SoD::INULL,							SoD::INULL,							SoD::INULL,
			SoD::INULL
		),

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
	),
	/*[MobVersion::OfflineUF] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			UF::INULL,							UF::INULL,							UF::INULL,
			UF::invtype::TRADE_SIZE,			UF::INULL,							UF::INULL,
			UF::INULL,							UF::INULL,							UF::INULL,
			UF::invtype::MERCHANT_SIZE,			UF::INULL,							UF::INULL,
			UF::invtype::BAZAAR_SIZE,			UF::invtype::INSPECT_SIZE,			UF::INULL,
			UF::invtype::VIEW_MOD_PC_SIZE,		UF::invtype::VIEW_MOD_BANK_SIZE,	UF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			UF::invtype::VIEW_MOD_LIMBO_SIZE,	UF::INULL,							UF::INULL,
			UF::INULL,							UF::INULL,							UF::INULL,
			UF::INULL
		),

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
	),
	/*[MobVersion::OfflineRoF] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			RoF::INULL,							RoF::INULL,							RoF::INULL,
			RoF::invtype::TRADE_SIZE,			RoF::INULL,							RoF::INULL,
			RoF::INULL,							RoF::INULL,							RoF::INULL,
			RoF::invtype::MERCHANT_SIZE,		RoF::INULL,							RoF::INULL,
			RoF::invtype::BAZAAR_SIZE,			RoF::invtype::INSPECT_SIZE,			RoF::INULL,
			RoF::invtype::VIEW_MOD_PC_SIZE,		RoF::invtype::VIEW_MOD_BANK_SIZE,	RoF::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			RoF::invtype::VIEW_MOD_LIMBO_SIZE,	RoF::INULL,							RoF::INULL,
			RoF::INULL,							RoF::INULL,							RoF::INULL,
			RoF::INULL
		),

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
	),
	/*[MobVersion::OfflineRoF2] =*/
	EQEmu::inventory::LookupEntry(
		EQEmu::inventory::LookupEntry::InventoryTypeSize_Struct(
			RoF2::INULL,						RoF2::INULL,						RoF2::INULL,
			RoF2::invtype::TRADE_SIZE,			RoF2::INULL,						RoF2::INULL,
			RoF2::INULL,						RoF2::INULL,						RoF2::INULL,
			RoF2::invtype::MERCHANT_SIZE,		RoF2::INULL,						RoF2::INULL,
			RoF2::invtype::BAZAAR_SIZE,			RoF2::invtype::INSPECT_SIZE,		RoF2::INULL,
			RoF2::invtype::VIEW_MOD_PC_SIZE,	RoF2::invtype::VIEW_MOD_BANK_SIZE,	RoF2::invtype::VIEW_MOD_SHARED_BANK_SIZE,
			RoF2::invtype::VIEW_MOD_LIMBO_SIZE,	RoF2::INULL,						RoF2::INULL,
			RoF2::INULL,						RoF2::INULL,						RoF2::INULL,
			RoF2::INULL
		),

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
	)
};

static bool inventory_dictionary_init = false;
void EQEmu::inventory::InitializeDynamicLookups() {
	if (inventory_dictionary_init == true)
		return;
	inventory_dictionary_init = true;

	// server is configured for static definitions
	if (RuleB(World, UseClientBasedExpansionSettings))
		return;

	// Notes:
	// Currently, there are only 3 known expansions that affect inventory-related settings in the clients..
	//   - Expansion::PoR "Prophecy of Ro" - toggles between 24 (set) and 16 (clear) bank slots
	//   - Expansion::TBS "The Buried Sea" - toggles slotPowerSource enabled (set) and disabled (clear) 
	//   - Expansion::HoT "House of Thule" - toggles slotGeneral9/slotGeneral10 enabled (set) and disabled (clear)
	// Obviously, the client must support the expansion to allow any (set) condition

	const uint32 current_expansions = RuleI(World, ExpansionSettings);
	const uint32 dynamic_check_mask = (EQEmu::expansions::bitPoR | EQEmu::expansions::bitTBS | EQEmu::expansions::bitHoT); // the only known expansions that affect inventory

	// if all of the above expansion bits are present, then static references will suffice
	if ((current_expansions & dynamic_check_mask) == dynamic_check_mask)
		return;

	for (uint32 iter = static_cast<uint32>(EQEmu::versions::ClientVersion::Unknown); iter <= static_cast<uint32>(EQEmu::versions::LastClientVersion); ++iter) {
		// no need to dynamic this condition since it is the lowest compatibility standard at this time
		if (iter <= static_cast<uint32>(EQEmu::versions::ClientVersion::Titanium))
			continue;

		// direct manipulation of lookup indices is safe so long as (int)ClientVersion::<client> == (int)MobVersion::<client>
		inventory_dynamic_lookup_entries[iter] = std::unique_ptr<LookupEntry>(new LookupEntry(inventory_static_lookup_entries[iter]));

		// clamp affected fields to the lowest standard
		inventory_dynamic_lookup_entries[iter]->InventoryTypeSize.Bank = Titanium::invtype::BANK_SIZE; // bank size
		inventory_dynamic_lookup_entries[iter]->EquipmentBitmask = Titanium::invslot::EQUIPMENT_BITMASK; // power source
		inventory_dynamic_lookup_entries[iter]->GeneralBitmask = Titanium::invslot::GENERAL_BITMASK; // general size
		inventory_dynamic_lookup_entries[iter]->PossessionsBitmask = 0; // we'll fix later
		inventory_dynamic_lookup_entries[iter]->CorpseBitmask = 0; // we'll fix later

		if (current_expansions & EQEmu::expansions::bitPoR) {
			// update bank size
			if (constants_static_lookup_entries[iter].ExpansionsMask & EQEmu::expansions::bitPoR)
				inventory_dynamic_lookup_entries[iter]->InventoryTypeSize.Bank = SoF::invtype::BANK_SIZE;
		}

		if (current_expansions & EQEmu::expansions::bitTBS) {
			// update power source
			if (constants_static_lookup_entries[iter].ExpansionsMask & EQEmu::expansions::bitTBS)
				inventory_dynamic_lookup_entries[iter]->EquipmentBitmask = SoF::invslot::EQUIPMENT_BITMASK;
		}

		if (current_expansions & EQEmu::expansions::bitHoT) {
			// update general size
			if (constants_static_lookup_entries[iter].ExpansionsMask & EQEmu::expansions::bitHoT)
				inventory_dynamic_lookup_entries[iter]->GeneralBitmask = RoF::invslot::GENERAL_BITMASK;
		}

		// fixup possessions bitmask
		inventory_dynamic_lookup_entries[iter]->PossessionsBitmask =
			(
				inventory_dynamic_lookup_entries[iter]->EquipmentBitmask |
				inventory_dynamic_lookup_entries[iter]->GeneralBitmask |
				inventory_dynamic_lookup_entries[iter]->CursorBitmask
			);

		// fixup corpse bitmask
		inventory_dynamic_lookup_entries[iter]->CorpseBitmask =
			(
				inventory_dynamic_lookup_entries[iter]->GeneralBitmask |
				inventory_dynamic_lookup_entries[iter]->CursorBitmask |
				(inventory_dynamic_lookup_entries[iter]->EquipmentBitmask << 34)
			);

		// expansion-related fields are now updated and all other fields reflect the static entry values
	}

	// only client versions that require a change from their static definitions have been given a dynamic lookup entry
}

const EQEmu::inventory::LookupEntry* EQEmu::inventory::DynamicLookup(versions::MobVersion mob_version)
{
	mob_version = versions::ValidateMobVersion(mob_version);
	if (inventory_dynamic_lookup_entries[static_cast<int>(mob_version)])
		return inventory_dynamic_lookup_entries[static_cast<int>(mob_version)].get();

	return &inventory_static_lookup_entries[static_cast<int>(mob_version)];
}

const EQEmu::inventory::LookupEntry* EQEmu::inventory::StaticLookup(versions::MobVersion mob_version)
{
	return &inventory_static_lookup_entries[static_cast<int>(versions::ValidateMobVersion(mob_version))];
}

static std::unique_ptr<EQEmu::behavior::LookupEntry> behavior_dynamic_lookup_entries[EQEmu::versions::MobVersionCount];
static const EQEmu::behavior::LookupEntry behavior_static_lookup_entries[EQEmu::versions::MobVersionCount] =
{
	/*[MobVersion::Unknown] =*/
	EQEmu::behavior::LookupEntry(
		true
	),
	/*[MobVersion::Client62] =*/
	EQEmu::behavior::LookupEntry(
		true
	),
	/*[MobVersion::Titanium] =*/
	EQEmu::behavior::LookupEntry(
		Titanium::behavior::CoinHasWeight
	),
	/*[MobVersion::SoF] =*/
	EQEmu::behavior::LookupEntry(
		SoF::behavior::CoinHasWeight
	),
	/*[MobVersion::SoD] =*/
	EQEmu::behavior::LookupEntry(
		SoD::behavior::CoinHasWeight
	),
	/*[MobVersion::UF] =*/
	EQEmu::behavior::LookupEntry(
		UF::behavior::CoinHasWeight
	),
	/*[MobVersion::RoF] =*/
	EQEmu::behavior::LookupEntry(
		RoF::behavior::CoinHasWeight
	),
	/*[MobVersion::RoF2] =*/
	EQEmu::behavior::LookupEntry(
		RoF2::behavior::CoinHasWeight
	),
	/*[MobVersion::NPC] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::NPCMerchant] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::Merc] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::Bot] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::ClientPet] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::NPCPet] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::MercPet] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::BotPet] =*/
	EQEmu::behavior::LookupEntry(
		EQEmu::behavior::CoinHasWeight
	),
	/*[MobVersion::OfflineTitanium] =*/
	EQEmu::behavior::LookupEntry(
		Titanium::behavior::CoinHasWeight
	),
	/*[MobVersion::OfflineSoF] =*/
	EQEmu::behavior::LookupEntry(
		SoF::behavior::CoinHasWeight
	),
	/*[MobVersion::OfflineSoD] =*/
	EQEmu::behavior::LookupEntry(
		SoD::behavior::CoinHasWeight
	),
	/*[MobVersion::OfflineUF] =*/
	EQEmu::behavior::LookupEntry(
		UF::behavior::CoinHasWeight
	),
	/*[MobVersion::OfflineRoF] =*/
	EQEmu::behavior::LookupEntry(
		RoF::behavior::CoinHasWeight
	),
	/*[MobVersion::OfflineRoF2] =*/
	EQEmu::behavior::LookupEntry(
		RoF2::behavior::CoinHasWeight
	)
};

static bool behavior_dictionary_init = false;
void EQEmu::behavior::InitializeDynamicLookups() {
	if (behavior_dictionary_init == true)
		return;
	behavior_dictionary_init = true;

	if (RuleB(World, UseClientBasedExpansionSettings))
		return;

	// use static references for now
}

const EQEmu::behavior::LookupEntry* EQEmu::behavior::DynamicLookup(versions::MobVersion mob_version)
{
	mob_version = versions::ValidateMobVersion(mob_version);
	if (behavior_dynamic_lookup_entries[static_cast<int>(mob_version)])
		return behavior_dynamic_lookup_entries[static_cast<int>(mob_version)].get();

	return &behavior_static_lookup_entries[static_cast<int>(mob_version)];
}

const EQEmu::behavior::LookupEntry* EQEmu::behavior::StaticLookup(versions::MobVersion mob_version)
{
	return &behavior_static_lookup_entries[static_cast<int>(versions::ValidateMobVersion(mob_version))];
}
