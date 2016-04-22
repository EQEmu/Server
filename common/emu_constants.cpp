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
#include "string_util.h"


uint16 EQEmu::constants::InventoryTypeSize(int16 type_index)
{
	switch (type_index) {
	case legacy::TypePossessions:
		return legacy::TYPE_POSSESSIONS_SIZE;
	case legacy::TypeBank:
		return legacy::TYPE_BANK_SIZE;
	case legacy::TypeSharedBank:
		return legacy::TYPE_SHARED_BANK_SIZE;
	case legacy::TypeTrade:
		return legacy::TYPE_TRADE_SIZE;
	case legacy::TypeWorld:
		return legacy::TYPE_WORLD_SIZE;
	case legacy::TypeLimbo:
		return legacy::TYPE_LIMBO_SIZE;
	case legacy::TypeTribute:
		return legacy::TYPE_TRIBUTE_SIZE;
	case legacy::TypeTrophyTribute:
		return legacy::TYPE_TROPHY_TRIBUTE_SIZE;
	case legacy::TypeGuildTribute:
		return legacy::TYPE_GUILD_TRIBUTE_SIZE;
	case legacy::TypeMerchant:
		return legacy::TYPE_MERCHANT_SIZE;
	case legacy::TypeDeleted:
		return legacy::TYPE_DELETED_SIZE;
	case legacy::TypeCorpse:
		return legacy::TYPE_CORPSE_SIZE;
	case legacy::TypeBazaar:
		return legacy::TYPE_BAZAAR_SIZE;
	case legacy::TypeInspect:
		return legacy::TYPE_INSPECT_SIZE;
	case legacy::TypeRealEstate:
		return legacy::TYPE_REAL_ESTATE_SIZE;
	case legacy::TypeViewMODPC:
		return legacy::TYPE_VIEW_MOD_PC_SIZE;
	case legacy::TypeViewMODBank:
		return legacy::TYPE_VIEW_MOD_BANK_SIZE;
	case legacy::TypeViewMODSharedBank:
		return legacy::TYPE_VIEW_MOD_SHARED_BANK_SIZE;
	case legacy::TypeViewMODLimbo:
		return legacy::TYPE_VIEW_MOD_LIMBO_SIZE;
	case legacy::TypeAltStorage:
		return legacy::TYPE_ALT_STORAGE_SIZE;
	case legacy::TypeArchived:
		return legacy::TYPE_ARCHIVED_SIZE;
	case legacy::TypeMail:
		return legacy::TYPE_MAIL_SIZE;
	case legacy::TypeGuildTrophyTribute:
		return legacy::TYPE_GUILD_TROPHY_TRIBUTE_SIZE;
	case legacy::TypeKrono:
		return legacy::TYPE_KRONO_SIZE;
	case legacy::TypeOther:
		return legacy::TYPE_OTHER_SIZE;
	default:
		return NOT_USED;
	}
}

/*
const char* EQEmu::constants::InventoryLocationName(Location_Struct location)
{
	// not ready for implementation...
	std::string ret_str;
	StringFormat(ret_str, "%s, %s, %s, %s", InventoryMapName(location.map), InventoryMainName(location.main), InventorySubName(location.sub), InventoryAugName(location.aug));
	return  ret_str;
}
*/

const char* EQEmu::constants::InventoryTypeName(int16 type_index)
{
	switch (type_index) {
	case INVALID_INDEX:
		return "Invalid Type";
	case legacy::TypePossessions:
		return "Possessions";
	case legacy::TypeBank:
		return "Bank";
	case legacy::TypeSharedBank:
		return "SharedBank";
	case legacy::TypeTrade:
		return "Trade";
	case legacy::TypeWorld:
		return "World";
	case legacy::TypeLimbo:
		return "Limbo";
	case legacy::TypeTribute:
		return "Tribute";
	case legacy::TypeTrophyTribute:
		return "TrophyTribute";
	case legacy::TypeGuildTribute:
		return "GuildTribute";
	case legacy::TypeMerchant:
		return "Merchant";
	case legacy::TypeDeleted:
		return "Deleted";
	case legacy::TypeCorpse:
		return "Corpse";
	case legacy::TypeBazaar:
		return "Bazaar";
	case legacy::TypeInspect:
		return "Inspect";
	case legacy::TypeRealEstate:
		return "RealEstate";
	case legacy::TypeViewMODPC:
		return "ViewMODPC";
	case legacy::TypeViewMODBank:
		return "ViewMODBank";
	case legacy::TypeViewMODSharedBank:
		return "ViewMODSharedBank";
	case legacy::TypeViewMODLimbo:
		return "ViewMODLimbo";
	case legacy::TypeAltStorage:
		return "AltStorage";
	case legacy::TypeArchived:
		return "Archived";
	case legacy::TypeMail:
		return "Mail";
	case legacy::TypeGuildTrophyTribute:
		return "GuildTrophyTribute";
	case legacy::TypeKrono:
		return "Krono";
	case legacy::TypeOther:
		return "Other";
	default:
		return "Unknown Type";
	}
}

const char* EQEmu::constants::InventorySlotName(int16 slot_index)
{
	switch (slot_index) {
	case INVALID_INDEX:
		return "Invalid Slot";
	case legacy::SlotCharm:
		return "Charm";
	case legacy::SlotEar1:
		return "Ear1";
	case legacy::SlotHead:
		return "Head";
	case legacy::SlotFace:
		return "Face";
	case legacy::SlotEar2:
		return "Ear2";
	case legacy::SlotNeck:
		return "Neck";
	case legacy::SlotShoulders:
		return "Shoulders";
	case legacy::SlotArms:
		return "Arms";
	case legacy::SlotBack:
		return "Back";
	case legacy::SlotWrist1:
		return "Wrist1";
	case legacy::SlotWrist2:
		return "Wrist2";
	case legacy::SlotRange:
		return "Range";
	case legacy::SlotHands:
		return "Hands";
	case legacy::SlotPrimary:
		return "Primary";
	case legacy::SlotSecondary:
		return "Secondary";
	case legacy::SlotFinger1:
		return "Finger1";
	case legacy::SlotFinger2:
		return "Finger2";
	case legacy::SlotChest:
		return "Chest";
	case legacy::SlotLegs:
		return "Legs";
	case legacy::SlotFeet:
		return "Feet";
	case legacy::SlotWaist:
		return "Waist";
	case legacy::SlotPowerSource:
		return "PowerSource";
	case legacy::SlotAmmo:
		return "Ammo";
	case legacy::SlotGeneral1:
		return "General1";
	case legacy::SlotGeneral2:
		return "General2";
	case legacy::SlotGeneral3:
		return "General3";
	case legacy::SlotGeneral4:
		return "General4";
	case legacy::SlotGeneral5:
		return "General5";
	case legacy::SlotGeneral6:
		return "General6";
	case legacy::SlotGeneral7:
		return "General7";
	case legacy::SlotGeneral8:
		return "General8";
	/*
	case legacy::SlotGeneral9:
		return "General9";
	case legacy::SlotGeneral10:
		return "General10";
	*/
	case legacy::SlotCursor:
		return "Cursor";
	default:
		return "Unknown Slot";
	}
}

const char* EQEmu::constants::InventorySubName(int16 sub_index)
{
	if (sub_index == INVALID_INDEX)
		return "Invalid Sub";

	if ((uint16)sub_index >= legacy::ITEM_CONTAINER_SIZE)
		return "Unknown Sub";

	static std::string ret_str;
	ret_str = StringFormat("Container%i", (sub_index + 1)); // zero-based index..but, count starts at one

	return ret_str.c_str();
}

const char* EQEmu::constants::InventoryAugName(int16 aug_index)
{
	if (aug_index == INVALID_INDEX)
		return "Invalid Aug";

	if ((uint16)aug_index >= legacy::ITEM_COMMON_SIZE)
		return "Unknown Aug";

	static std::string ret_str;
	ret_str = StringFormat("Augment%i", (aug_index + 1)); // zero-based index..but, count starts at one

	return ret_str.c_str();
}
