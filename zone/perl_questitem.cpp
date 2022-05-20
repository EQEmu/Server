#include "../common/features.h"
#include "client.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "../common/item_instance.h"

#ifdef THIS        /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_ITEM \
	do { \
		if (sv_derived_from(ST(0), "QuestItem")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(EQ::ItemInstance*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type EQ::ItemInstance"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_QuestItem_GetName);
XS(XS_QuestItem_GetName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::GetName(THIS)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		Const_char          *RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_ITEM;
		RETVAL = THIS->GetItem()->Name;
		sv_setpv(TARG, RETVAL);
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_QuestItem_SetScale);
XS(XS_QuestItem_SetScale) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: QuestItem::SetScale(THIS, float scale_multiplier)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		float Mult;
		VALIDATE_THIS_IS_ITEM;
		Mult = (float) SvNV(ST(1));

		if (THIS->IsScaling()) {
			THIS->SetExp((int) (Mult * 10000 + .5));
		}
	}
	XSRETURN_EMPTY;
}

XS(XS_QuestItem_ItemSay);
XS(XS_QuestItem_ItemSay) {
	dXSARGS;
	if (items != 2 && items != 3)
		Perl_croak(aTHX_ "Usage: QuestItem::ItemSay(THIS, string text [int language_id])"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		Const_char          *text;
		int lang = 0;
		VALIDATE_THIS_IS_ITEM;
		text     = SvPV_nolen(ST(1));
		if (items == 3)
			lang = (int) SvUV(ST(2));

		quest_manager.GetInitiator()->ChannelMessageSend(THIS->GetItem()->Name, 0, 8, lang, 100, text);
	}
	XSRETURN_EMPTY;
}

XS(XS_QuestItem_IsType); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_IsType) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: QuestItem::IsType(THIS, type)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		bool   RETVAL;
		uint32 type = (int32) SvIV(ST(1));
		VALIDATE_THIS_IS_ITEM;
		RETVAL = THIS->IsType((EQ::item::ItemClass) type);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_QuestItem_IsAttuned); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_IsAttuned) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::IsAttuned(THIS)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		bool RETVAL;
		VALIDATE_THIS_IS_ITEM;
		RETVAL = THIS->IsAttuned();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_QuestItem_GetCharges); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_GetCharges) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::GetCharges(THIS)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		int16 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_ITEM;
		RETVAL = THIS->GetCharges();
		XSprePUSH;
		PUSHi((IV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_QuestItem_GetAugment); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_GetAugment) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: QuestItem::GetAugment(THIS, int16 slot_id)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		int16 slot_id = (int16) SvIV(ST(1));
		EQ::ItemInstance *RETVAL;
		VALIDATE_THIS_IS_ITEM;
		RETVAL = THIS->GetAugment(slot_id);
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "QuestItem", (void *) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_QuestItem_GetID); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_GetID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::GetID(THIS)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_ITEM;
		RETVAL = THIS->GetItem()->ID;
		XSprePUSH;
		PUSHi((IV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_QuestItem_ContainsAugmentByID); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_ContainsAugmentByID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: QuestItem::ContainsAugmentByID(THIS, uint32 item_id)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		uint32 item_id = (uint32) SvUV(ST(1));
		bool contains_augment = false;
		VALIDATE_THIS_IS_ITEM;
		contains_augment = THIS->ContainsAugmentByID(item_id);
		ST(0) = boolSV(contains_augment);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_QuestItem_CountAugmentByID); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_CountAugmentByID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: QuestItem::CountAugmentByID(THIS, uint32 item_id)"); // @categories Inventory and Items
	{
		EQ::ItemInstance *THIS;
		int quantity = 0;
		uint32 item_id = (uint32) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_ITEM;
		quantity = THIS->CountAugmentByID(item_id);
		XSprePUSH;
		PUSHi((IV) quantity);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_QuestItem);
XS(boot_QuestItem) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "ContainsAugmentByID"), XS_QuestItem_ContainsAugmentByID, file, "$$");
	newXSproto(strcpy(buf, "CountAugmentByID"), XS_QuestItem_CountAugmentByID, file, "$$");
	newXSproto(strcpy(buf, "GetAugment"), XS_QuestItem_GetAugment, file, "$$");
	newXSproto(strcpy(buf, "GetCharges"), XS_QuestItem_GetCharges, file, "$");
	newXSproto(strcpy(buf, "GetID"), XS_QuestItem_GetID, file, "$");
	newXSproto(strcpy(buf, "GetName"), XS_QuestItem_GetName, file, "$");
	newXSproto(strcpy(buf, "IsAttuned"), XS_QuestItem_IsAttuned, file, "$");
	newXSproto(strcpy(buf, "IsType"), XS_QuestItem_IsType, file, "$$");
	newXSproto(strcpy(buf, "ItemSay"), XS_QuestItem_ItemSay, file, "$");
	newXSproto(strcpy(buf, "SetScale"), XS_QuestItem_SetScale, file, "$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
