#include "../common/features.h"
#include "client.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "../common/inventory_profile.h"

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_INVENTORY \
	do { \
		if (sv_derived_from(ST(0), "Inventory")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(EQ::InventoryProfile*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type EQ::InventoryProfile"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Inventory_CanItemFitInContainer);
XS(XS_Inventory_CanItemFitInContainer) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Inventory::CanItemFitInContainer(THIS, ItemInstance item_to_check, ItemInstance container_to_check)");
	{
		EQ::InventoryProfile* THIS;
		bool can_fit = false;
		EQ::ItemInstance* item_to_check = (EQ::ItemInstance*)SvIV((SV*)SvRV(ST(1)));
		EQ::ItemInstance* container_to_check = (EQ::ItemInstance*)SvIV((SV*)SvRV(ST(2)));
		const EQ::ItemData* item_data = item_to_check->GetItem();
		const EQ::ItemData* container_data = container_to_check->GetItem();
		VALIDATE_THIS_IS_INVENTORY;
		can_fit = THIS->CanItemFitInContainer(item_data, container_data);
		ST(0) = boolSV(can_fit);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_CheckNoDrop);
XS(XS_Inventory_CheckNoDrop) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::CheckNoDrop(THIS, int16 slot_id)");
	{
		EQ::InventoryProfile* THIS;
		bool no_drop = false;
		int16 slot_id = (int16)SvIV(ST(1));
		VALIDATE_THIS_IS_INVENTORY;
		no_drop = THIS->CheckNoDrop(slot_id);
		ST(0) = boolSV(no_drop);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_DeleteItem);
XS(XS_Inventory_DeleteItem) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: Inventory::DeleteItem(THIS, int16 slot_id, [uint8 quantity = 0])");
	{
		EQ::InventoryProfile* THIS;
		bool item_deleted = false;
		int16 slot_id = (int16)SvIV(ST(1));
		uint8 quantity = 0;
		VALIDATE_THIS_IS_INVENTORY;
		if (items > 2)
			quantity = (uint8)SvUV(ST(2));

		item_deleted = THIS->DeleteItem(slot_id, quantity);
		ST(0) = boolSV(item_deleted);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_FindFreeSlot);
XS(XS_Inventory_FindFreeSlot) {
	dXSARGS;
	if (items < 3 || items > 5)
		Perl_croak(aTHX_ "Usage: Inventory::FindFreeSlot(THIS, bool is_for_bag, bool try_cursor, [uint8 min_size = 0, bool is_arrow = false])");
	{
		EQ::InventoryProfile* THIS;
		int16 free_slot;
		bool is_for_bag = (bool)SvNV(ST(1));
		bool try_cursor = (bool)SvNV(ST(2));
		uint8 min_size = 0;
		bool is_arrow = false;
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (items > 3)
			min_size = (uint8)SvUV(ST(3));
		if (items > 4)
			is_arrow = (bool)SvNV(ST(4));

		free_slot = THIS->FindFreeSlot(is_for_bag, try_cursor, min_size, is_arrow);
		XSprePUSH;
		PUSHi((IV)free_slot);
	}
	XSRETURN(1);
}

XS(XS_Inventory_GetBagIndex);
XS(XS_Inventory_GetBagIndex) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::GetBagIndex(THIS, int16 slot_id)");
	{
		EQ::InventoryProfile* THIS;
		uint8 bag_index;
		int16 slot_id = (int16)SvIV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		bag_index = THIS->CalcBagIdx(slot_id);
		XSprePUSH;
		PUSHu((UV) bag_index);
	}
	XSRETURN(1);
}

XS(XS_Inventory_GetItem);
XS(XS_Inventory_GetItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::GetItem(THIS, int16 slot_id)");
	{
		EQ::InventoryProfile* THIS;
		EQ::ItemInstance* item;
		int16 slot_id = (int16)SvIV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		item = THIS->GetItem(slot_id);
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "QuestItem", (void*)item);
	}
	XSRETURN(1);
}

XS(XS_Inventory_GetMaterialFromSlot);
XS(XS_Inventory_GetMaterialFromSlot) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::GetMaterialFromSlot(THIS, int16 slot_id)");
	{
		EQ::InventoryProfile* THIS;
		uint8 material;
		int16 slot_id = (int16)SvIV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		material = THIS->CalcMaterialFromSlot(slot_id);
		XSprePUSH;
		PUSHu((UV) material);
	}
	XSRETURN(1);
}

XS(XS_Inventory_GetSlotByItemInst);
XS(XS_Inventory_GetSlotByItemInst) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::GetSlotByItemInst(THIS, ItemInstance item)");
	{
		EQ::InventoryProfile* THIS;
		int slot_id;
		EQ::ItemInstance* item = (EQ::ItemInstance*)SvIV((SV*)SvRV(ST(1)));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		slot_id = THIS->GetSlotByItemInst(item);
		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_GetSlotFromMaterial);
XS(XS_Inventory_GetSlotFromMaterial) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::GetSlotFromMaterial(THIS, uint8 material)");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id;
		uint8 material = (uint8)SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		slot_id = THIS->CalcSlotFromMaterial(material);
		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_GetSlotID);
XS(XS_Inventory_GetSlotID) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: Inventory::GetSlotID(THIS, int16 slot_id, [uint8 bag_index])");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id = (int16)SvIV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (items == 2)
			slot_id = THIS->CalcSlotId(slot_id);

		if (items == 3) {
			uint8 bag_index = (uint8)SvUV(ST(2));
			slot_id = THIS->CalcSlotId(slot_id, bag_index);
		}

		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_HasItem);
XS(XS_Inventory_HasItem) {
	dXSARGS;
	if (items < 2 || items > 4)
		Perl_croak(aTHX_ "Usage: Inventory::HasItem(THIS, uint32 item_id, [uint8 quantity, uint8 where])");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id;
		uint32 item_id = (uint32)SvUV(ST(1));
		uint8 quantity = 0;
		uint8 where_to_look = 0xFF;
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (items > 2)
			quantity = (uint8)SvUV(ST(2));
		if (items > 3)
			where_to_look = (uint8)SvUV(ST(3));

		slot_id = THIS->HasItem(item_id, quantity, where_to_look);
		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_HasItemByLoreGroup);
XS(XS_Inventory_HasItemByLoreGroup) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: Inventory::HasItemByLoreGroup(THIS, uint32 loregroup, [uint8 where])");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id;
		uint32 loregroup = (uint32)SvUV(ST(1));
		uint8 where_to_look = 0xFF;
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (items > 2)
			where_to_look = (uint8)SvUV(ST(2));

		slot_id = THIS->HasItemByLoreGroup(loregroup, where_to_look);
		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_HasItemByUse);
XS(XS_Inventory_HasItemByUse) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: Inventory::HasItemByUse(THIS, uint8 use, uint8 quantity, [uint8 where])");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id;
		uint8 item_use = (uint8)SvUV(ST(1));
		uint8 quantity = (uint8)SvUV(ST(2));
		uint8 where_to_look = 0xFF;
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (items > 3)
			where_to_look = (uint8)SvUV(ST(3));

		slot_id = THIS->HasItemByUse(item_use, quantity, where_to_look);
		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_HasSpaceForItem);
XS(XS_Inventory_HasSpaceForItem) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Inventory::HasSpaceForItem(THIS, ItemInstance item_to_check, uint8 quantity)");
	{
		EQ::InventoryProfile* THIS;
		bool has_space = false;
		EQ::ItemInstance* item_to_check = (EQ::ItemInstance*)SvIV((SV*)SvRV(ST(1)));
		uint8 quantity = (uint8)SvUV(ST(2));
		const EQ::ItemData* item_data = item_to_check->GetItem();
		VALIDATE_THIS_IS_INVENTORY;
		has_space = THIS->HasSpaceForItem(item_data, quantity);
		ST(0) = boolSV(has_space);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_PopItem);
XS(XS_Inventory_PopItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::PopItem(THIS, int16 slot_id)");
	{
		EQ::InventoryProfile* THIS;
		EQ::ItemInstance* item;
		int16 slot_id = (int16)SvIV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		item = THIS->PopItem(slot_id);
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "QuestItem", (void*)item);
	}
	XSRETURN(1);
}

XS(XS_Inventory_SupportsContainers);
XS(XS_Inventory_SupportsContainers) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::SupportsContainers(THIS, int16 slot_id)");
	{
		EQ::InventoryProfile* THIS;
		bool supports_containers = false;
		int16 slot_id = (int16)SvIV(ST(1));
		VALIDATE_THIS_IS_INVENTORY;
		supports_containers = THIS->SupportsContainers(slot_id);
		ST(0) = boolSV(supports_containers);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_SwapItem);
XS(XS_Inventory_SwapItem) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Inventory::SwapItem(THIS, int16 source_slot_id, int16 destination_slot_id)");
	{
		EQ::InventoryProfile* THIS;
		bool item_swapped = false;
		int16 source_slot_id = (int16)SvIV(ST(1));
		int16 destination_slot_id = (int16)SvIV(ST(2));
		EQ::InventoryProfile::SwapItemFailState fail_state = EQ::InventoryProfile::swapInvalid;
		VALIDATE_THIS_IS_INVENTORY;
		item_swapped = THIS->SwapItem(source_slot_id, destination_slot_id, fail_state);
		ST(0) = boolSV(item_swapped);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_PushCursor);
XS(XS_Inventory_PushCursor) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::PushCursor(THIS, ItemInstance item)");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id;
		EQ::ItemInstance* item = (EQ::ItemInstance*)SvIV((SV*)SvRV(ST(1)));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (item)
			slot_id = THIS->PushCursor(*item);
		else
			slot_id = 0;

		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_PutItem);
XS(XS_Inventory_PutItem) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Inventory::PutItem(THIS, int16 slot_id, ItemInstance item)");
	{
		EQ::InventoryProfile* THIS;
		int16 slot_id = (int16)SvUV(ST(1));
		EQ::ItemInstance* item = (EQ::ItemInstance*)SvIV((SV*)SvRV(ST(2)));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		if (item)
			slot_id = THIS->PutItem(slot_id, *item);
		else
			slot_id = 0;
			
		XSprePUSH;
		PUSHi((IV) slot_id);
	}
	XSRETURN(1);
}

XS(XS_Inventory_HasAugmentEquippedByID);
XS(XS_Inventory_HasAugmentEquippedByID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::HasAugmentEquippedByID(THIS, uint32 item_id)");
	{
		EQ::InventoryProfile* THIS;
		bool has_equipped = false;
		uint32 item_id = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_INVENTORY;
		has_equipped = THIS->HasAugmentEquippedByID(item_id);
		ST(0) = boolSV(has_equipped);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_CountAugmentEquippedByID);
XS(XS_Inventory_CountAugmentEquippedByID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::CountAugmentEquippedByID(THIS, uint32 item_id)");
	{
		EQ::InventoryProfile* THIS;
		int quantity = 0;
		uint32 item_id = (uint32) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		quantity = THIS->CountAugmentEquippedByID(item_id);
		XSprePUSH;
		PUSHi((IV) quantity);
	}
	XSRETURN(1);
}

XS(XS_Inventory_HasItemEquippedByID);
XS(XS_Inventory_HasItemEquippedByID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::HasItemEquippedByID(THIS, uint32 item_id)");
	{
		EQ::InventoryProfile* THIS;
		bool has_equipped = false;
		uint32 item_id = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_INVENTORY;
		has_equipped = THIS->HasItemEquippedByID(item_id);
		ST(0) = boolSV(has_equipped);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Inventory_CountItemEquippedByID);
XS(XS_Inventory_CountItemEquippedByID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Inventory::CountItemEquippedByID(THIS, uint32 item_id)");
	{
		EQ::InventoryProfile* THIS;
		int quantity = 0;
		uint32 item_id = (uint32) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_INVENTORY;
		quantity = THIS->CountItemEquippedByID(item_id);
		XSprePUSH;
		PUSHi((IV) quantity);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_Inventory);
XS(boot_Inventory) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;
	if (items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");

	char buf[128];
	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "CanItemFitInContainer"), XS_Inventory_CanItemFitInContainer, file, "$$$");
	newXSproto(strcpy(buf, "CountAugmentEquippedByID"), XS_Inventory_CountAugmentEquippedByID, file, "$$");
	newXSproto(strcpy(buf, "CountItemEquippedByID"), XS_Inventory_CountItemEquippedByID, file, "$$");
	newXSproto(strcpy(buf, "CheckNoDrop"), XS_Inventory_CheckNoDrop, file, "$$");
	newXSproto(strcpy(buf, "DeleteItem"), XS_Inventory_DeleteItem, file, "$$;$");
	newXSproto(strcpy(buf, "FindFreeSlot"), XS_Inventory_FindFreeSlot, file, "$$$;$$");
	newXSproto(strcpy(buf, "GetBagIndex"), XS_Inventory_GetBagIndex, file, "$$");
	newXSproto(strcpy(buf, "GetItem"), XS_Inventory_GetItem, file, "$$;$");
	newXSproto(strcpy(buf, "GetMaterialFromSlot"), XS_Inventory_GetMaterialFromSlot, file, "$$");
	newXSproto(strcpy(buf, "GetSlotByItemInst"), XS_Inventory_GetSlotByItemInst, file, "$$");
	newXSproto(strcpy(buf, "GetSlotFromMaterial"), XS_Inventory_GetSlotFromMaterial, file, "$$");
	newXSproto(strcpy(buf, "GetSlotID"), XS_Inventory_GetSlotID, file, "$$;$");
	newXSproto(strcpy(buf, "HasAugmentEquippedByID"), XS_Inventory_HasAugmentEquippedByID, file, "$$");
	newXSproto(strcpy(buf, "HasItem"), XS_Inventory_HasItem, file, "$$;$$");
	newXSproto(strcpy(buf, "HasItemByLoreGroup"), XS_Inventory_HasItemByLoreGroup, file, "$$;$");
	newXSproto(strcpy(buf, "HasItemByUse"), XS_Inventory_HasItemByUse, file, "$$;$$");
	newXSproto(strcpy(buf, "HasItemEquippedByID"), XS_Inventory_HasItemEquippedByID, file, "$$");
	newXSproto(strcpy(buf, "HasSpaceForItem"), XS_Inventory_HasSpaceForItem, file, "$$$");
	newXSproto(strcpy(buf, "PopItem"), XS_Inventory_PopItem, file, "$$");
	newXSproto(strcpy(buf, "PushCursor"), XS_Inventory_PushCursor, file, "$$");
	newXSproto(strcpy(buf, "PutItem"), XS_Inventory_PutItem, file, "$$$");
	newXSproto(strcpy(buf, "SupportsContainers"), XS_Inventory_SupportsContainers, file, "$$");
	newXSproto(strcpy(buf, "SwapItem"), XS_Inventory_SwapItem, file, "$$$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
