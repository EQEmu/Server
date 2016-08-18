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

#include "sod_limits.h"

#include "../string_util.h"


size_t SoD::invtype::GetInvTypeSize(int inv_type)
{
	switch (inv_type) {
	case invtype::InvTypePossessions:
		return invtype::InvTypePossessionsSize;
	case invtype::InvTypeBank:
		return invtype::InvTypeBankSize;
	case invtype::InvTypeSharedBank:
		return invtype::InvTypeSharedBankSize;
	case invtype::InvTypeTrade:
		return invtype::InvTypeTradeSize;
	case invtype::InvTypeWorld:
		return invtype::InvTypeWorldSize;
	case invtype::InvTypeLimbo:
		return invtype::InvTypeLimboSize;
	case invtype::InvTypeTribute:
		return invtype::InvTypeTributeSize;
	case invtype::InvTypeGuildTribute:
		return invtype::InvTypeGuildTributeSize;
	case invtype::InvTypeMerchant:
		return invtype::InvTypeMerchantSize;
	case invtype::InvTypeCorpse:
		return invtype::InvTypeCorpseSize;
	case invtype::InvTypeBazaar:
		return invtype::InvTypeBazaarSize;
	case invtype::InvTypeInspect:
		return invtype::InvTypeInspectSize;
	case invtype::InvTypeViewMODPC:
		return invtype::InvTypeViewMODPCSize;
	case invtype::InvTypeViewMODBank:
		return invtype::InvTypeViewMODBankSize;
	case invtype::InvTypeViewMODSharedBank:
		return invtype::InvTypeViewMODSharedBankSize;
	case invtype::InvTypeViewMODLimbo:
		return invtype::InvTypeViewMODLimboSize;
	case invtype::InvTypeAltStorage:
		return invtype::InvTypeAltStorageSize;
	case invtype::InvTypeArchived:
		return invtype::InvTypeArchivedSize;
	case invtype::InvTypeOther:
		return invtype::InvTypeOtherSize;
	default:
		return 0;
	}
}

const char* SoD::invtype::GetInvTypeName(int inv_type)
{
	switch (inv_type) {
	case invtype::InvTypeInvalid:
		return "Invalid Type";
	case invtype::InvTypePossessions:
		return "Possessions";
	case invtype::InvTypeBank:
		return "Bank";
	case invtype::InvTypeSharedBank:
		return "Shared Bank";
	case invtype::InvTypeTrade:
		return "Trade";
	case invtype::InvTypeWorld:
		return "World";
	case invtype::InvTypeLimbo:
		return "Limbo";
	case invtype::InvTypeTribute:
		return "Tribute";
	case invtype::InvTypeGuildTribute:
		return "Guild Tribute";
	case invtype::InvTypeMerchant:
		return "Merchant";
	case invtype::InvTypeCorpse:
		return "Corpse";
	case invtype::InvTypeBazaar:
		return "Bazaar";
	case invtype::InvTypeInspect:
		return "Inspect";
	case invtype::InvTypeViewMODPC:
		return "View MOD PC";
	case invtype::InvTypeViewMODBank:
		return "View MOD Bank";
	case invtype::InvTypeViewMODSharedBank:
		return "View MOD Shared Bank";
	case invtype::InvTypeViewMODLimbo:
		return "View MOD Limbo";
	case invtype::InvTypeAltStorage:
		return "Alt Storage";
	case invtype::InvTypeArchived:
		return "Archived";
	case invtype::InvTypeOther:
		return "Other";
	default:
		return "Unknown Type";
	}
}

bool SoD::invtype::IsInvTypePersistent(int inv_type)
{
	switch (inv_type) {
	case invtype::InvTypePossessions:
	case invtype::InvTypeBank:
	case invtype::InvTypeSharedBank:
	case invtype::InvTypeTrade:
	case invtype::InvTypeWorld:
	case invtype::InvTypeLimbo:
	case invtype::InvTypeTribute:
	case invtype::InvTypeGuildTribute:
		return true;
	default:
		return false;
	}
}

const char* SoD::invslot::GetInvPossessionsSlotName(int inv_slot)
{
	switch (inv_slot) {
	case invslot::InvSlotInvalid:
		return "Invalid Slot";
	case invslot::PossessionsCharm:
		return "Charm";
	case invslot::PossessionsEar1:
		return "Ear 1";
	case invslot::PossessionsHead:
		return "Head";
	case invslot::PossessionsFace:
		return "Face";
	case invslot::PossessionsEar2:
		return "Ear 2";
	case invslot::PossessionsNeck:
		return "Neck";
	case invslot::PossessionsShoulders:
		return "Shoulders";
	case invslot::PossessionsArms:
		return "Arms";
	case invslot::PossessionsBack:
		return "Back";
	case invslot::PossessionsWrist1:
		return "Wrist 1";
	case invslot::PossessionsWrist2:
		return "Wrist 2";
	case invslot::PossessionsRange:
		return "Range";
	case invslot::PossessionsHands:
		return "Hands";
	case invslot::PossessionsPrimary:
		return "Primary";
	case invslot::PossessionsSecondary:
		return "Secondary";
	case invslot::PossessionsFinger1:
		return "Finger 1";
	case invslot::PossessionsFinger2:
		return "Finger 2";
	case invslot::PossessionsChest:
		return "Chest";
	case invslot::PossessionsLegs:
		return "Legs";
	case invslot::PossessionsFeet:
		return "Feet";
	case invslot::PossessionsWaist:
		return "Waist";
	case invslot::PossessionsPowerSource:
		return "Power Source";
	case invslot::PossessionsAmmo:
		return "Ammo";
	case invslot::PossessionsGeneral1:
		return "General 1";
	case invslot::PossessionsGeneral2:
		return "General 2";
	case invslot::PossessionsGeneral3:
		return "General 3";
	case invslot::PossessionsGeneral4:
		return "General 4";
	case invslot::PossessionsGeneral5:
		return "General 5";
	case invslot::PossessionsGeneral6:
		return "General 6";
	case invslot::PossessionsGeneral7:
		return "General 7";
	case invslot::PossessionsGeneral8:
		return "General 8";
	case invslot::PossessionsCursor:
		return "Cursor";
	default:
		return "Unknown Slot";
	}
}

const char* SoD::invslot::GetInvCorpseSlotName(int inv_slot)
{
	if (!invtype::GetInvTypeSize(invtype::InvTypeCorpse) || inv_slot == invslot::InvSlotInvalid)
		return "Invalid Slot";

	// needs work
	if ((size_t)(inv_slot + 1) < invslot::CorpseBegin || (size_t)(inv_slot + 1) >= invslot::CorpseEnd)
		return "Unknown Slot";

	static std::string ret_str;
	ret_str = StringFormat("Slot %i", (inv_slot + 1));

	return ret_str.c_str();
}

const char* SoD::invslot::GetInvSlotName(int inv_type, int inv_slot)
{
	if (inv_type == invtype::InvTypePossessions)
		return invslot::GetInvPossessionsSlotName(inv_slot);
	else if (inv_type == invtype::InvTypeCorpse)
		return invslot::GetInvCorpseSlotName(inv_slot);

	size_t type_size = invtype::GetInvTypeSize(inv_type);

	if (!type_size || inv_slot == invslot::InvSlotInvalid)
		return "Invalid Slot";

	if ((size_t)(inv_slot + 1) >= type_size)
		return "Unknown Slot";

	static std::string ret_str;
	ret_str = StringFormat("Slot %i", (inv_slot + 1));

	return ret_str.c_str();
}

const char* SoD::invbag::GetInvBagIndexName(int bag_index)
{
	if (bag_index == invbag::InvBagInvalid)
		return "Invalid Bag";

	if ((size_t)bag_index >= invbag::ItemBagSize)
		return "Unknown Bag";

	static std::string ret_str;
	ret_str = StringFormat("Bag %i", (bag_index + 1));

	return ret_str.c_str();
}

const char* SoD::invaug::GetInvAugIndexName(int aug_index)
{
	if (aug_index == invaug::InvAugInvalid)
		return "Invalid Augment";

	if ((size_t)aug_index >= invaug::ItemAugSize)
		return "Unknown Augment";

	static std::string ret_str;
	ret_str = StringFormat("Augment %i", (aug_index + 1));

	return ret_str.c_str();
}
