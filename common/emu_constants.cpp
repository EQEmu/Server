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


uint16 EQEmu::Constants::InventoryTypeSize(int16 type_index)
{
	switch (type_index) {
	case TypePossessions:
		return TYPE_POSSESSIONS_SIZE;
	case TypeBank:
		return TYPE_BANK_SIZE;
	case TypeSharedBank:
		return TYPE_SHARED_BANK_SIZE;
	case TypeTrade:
		return TYPE_TRADE_SIZE;
	case TypeWorld:
		return TYPE_WORLD_SIZE;
	case TypeLimbo:
		return TYPE_LIMBO_SIZE;
	case TypeTribute:
		return TYPE_TRIBUTE_SIZE;
	case TypeTrophyTribute:
		return TYPE_TROPHY_TRIBUTE_SIZE;
	case TypeGuildTribute:
		return TYPE_GUILD_TRIBUTE_SIZE;
	case TypeMerchant:
		return TYPE_MERCHANT_SIZE;
	case TypeDeleted:
		return TYPE_DELETED_SIZE;
	case TypeCorpse:
		return TYPE_CORPSE_SIZE;
	case TypeBazaar:
		return TYPE_BAZAAR_SIZE;
	case TypeInspect:
		return TYPE_INSPECT_SIZE;
	case TypeRealEstate:
		return TYPE_REAL_ESTATE_SIZE;
	case TypeViewMODPC:
		return TYPE_VIEW_MOD_PC_SIZE;
	case TypeViewMODBank:
		return TYPE_VIEW_MOD_BANK_SIZE;
	case TypeViewMODSharedBank:
		return TYPE_VIEW_MOD_SHARED_BANK_SIZE;
	case TypeViewMODLimbo:
		return TYPE_VIEW_MOD_LIMBO_SIZE;
	case TypeAltStorage:
		return TYPE_ALT_STORAGE_SIZE;
	case TypeArchived:
		return TYPE_ARCHIVED_SIZE;
	case TypeMail:
		return TYPE_MAIL_SIZE;
	case TypeGuildTrophyTribute:
		return TYPE_GUILD_TROPHY_TRIBUTE_SIZE;
	case TypeKrono:
		return TYPE_KRONO_SIZE;
	case TypeOther:
		return TYPE_OTHER_SIZE;
	default:
		return NOT_USED;
	}
}

/*
const char* EQEmu::Constants::InventoryLocationName(Location_Struct location)
{
	// not ready for implementation...
	std::string ret_str;
	StringFormat(ret_str, "%s, %s, %s, %s", InventoryMapName(location.map), InventoryMainName(location.main), InventorySubName(location.sub), InventoryAugName(location.aug));
	return  ret_str;
}
*/

const char* EQEmu::Constants::InventoryTypeName(int16 type_index)
{
	switch (type_index) {
	case INVALID_INDEX:
		return "Invalid Type";
	case TypePossessions:
		return "Possessions";
	case TypeBank:
		return "Bank";
	case TypeSharedBank:
		return "SharedBank";
	case TypeTrade:
		return "Trade";
	case TypeWorld:
		return "World";
	case TypeLimbo:
		return "Limbo";
	case TypeTribute:
		return "Tribute";
	case TypeTrophyTribute:
		return "TrophyTribute";
	case TypeGuildTribute:
		return "GuildTribute";
	case TypeMerchant:
		return "Merchant";
	case TypeDeleted:
		return "Deleted";
	case TypeCorpse:
		return "Corpse";
	case TypeBazaar:
		return "Bazaar";
	case TypeInspect:
		return "Inspect";
	case TypeRealEstate:
		return "RealEstate";
	case TypeViewMODPC:
		return "ViewMODPC";
	case TypeViewMODBank:
		return "ViewMODBank";
	case TypeViewMODSharedBank:
		return "ViewMODSharedBank";
	case TypeViewMODLimbo:
		return "ViewMODLimbo";
	case TypeAltStorage:
		return "AltStorage";
	case TypeArchived:
		return "Archived";
	case TypeMail:
		return "Mail";
	case TypeGuildTrophyTribute:
		return "GuildTrophyTribute";
	case TypeKrono:
		return "Krono";
	case TypeOther:
		return "Other";
	default:
		return "Unknown Type";
	}
}

const char* EQEmu::Constants::InventorySlotName(int16 slot_index)
{
	switch (slot_index) {
	case INVALID_INDEX:
		return "Invalid Slot";
	case SlotCharm:
		return "Charm";
	case SlotEar1:
		return "Ear1";
	case SlotHead:
		return "Head";
	case SlotFace:
		return "Face";
	case SlotEar2:
		return "Ear2";
	case SlotNeck:
		return "Neck";
	case SlotShoulders:
		return "Shoulders";
	case SlotArms:
		return "Arms";
	case SlotBack:
		return "Back";
	case SlotWrist1:
		return "Wrist1";
	case SlotWrist2:
		return "Wrist2";
	case SlotRange:
		return "Range";
	case SlotHands:
		return "Hands";
	case SlotPrimary:
		return "Primary";
	case SlotSecondary:
		return "Secondary";
	case SlotFinger1:
		return "Finger1";
	case SlotFinger2:
		return "Finger2";
	case SlotChest:
		return "Chest";
	case SlotLegs:
		return "Legs";
	case SlotFeet:
		return "Feet";
	case SlotWaist:
		return "Waist";
	case SlotPowerSource:
		return "PowerSource";
	case SlotAmmo:
		return "Ammo";
	case SlotGeneral1:
		return "General1";
	case SlotGeneral2:
		return "General2";
	case SlotGeneral3:
		return "General3";
	case SlotGeneral4:
		return "General4";
	case SlotGeneral5:
		return "General5";
	case SlotGeneral6:
		return "General6";
	case SlotGeneral7:
		return "General7";
	case SlotGeneral8:
		return "General8";
	/*
	case SlotGeneral9:
		return "General9";
	case SlotGeneral10:
		return "General10";
	*/
	case SlotCursor:
		return "Cursor";
	default:
		return "Unknown Slot";
	}
}

const char* EQEmu::Constants::InventorySubName(int16 sub_index)
{
	if (sub_index == INVALID_INDEX)
		return "Invalid Sub";

	if ((uint16)sub_index >= ITEM_CONTAINER_SIZE)
		return "Unknown Sub";

	static std::string ret_str;
	ret_str = StringFormat("Container%i", (sub_index + 1)); // zero-based index..but, count starts at one

	return ret_str.c_str();
}

const char* EQEmu::Constants::InventoryAugName(int16 aug_index)
{
	if (aug_index == INVALID_INDEX)
		return "Invalid Aug";

	if ((uint16)aug_index >= ITEM_COMMON_SIZE)
		return "Unknown Aug";

	static std::string ret_str;
	ret_str = StringFormat("Augment%i", (aug_index + 1)); // zero-based index..but, count starts at one

	return ret_str.c_str();
}
