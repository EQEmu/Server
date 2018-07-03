/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

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

XS(XS_QuestItem_GetName);
XS(XS_QuestItem_GetName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::GetName(THIS)");
	{
		EQEmu::ItemInstance *THIS;
		Const_char          *RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

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
		Perl_croak(aTHX_ "Usage: QuestItem::SetScale(THIS, float scale_multiplier)");
	{
		EQEmu::ItemInstance *THIS;
		float Mult;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

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
		Perl_croak(aTHX_ "Usage: QuestItem::ItemSay(THIS, string text [int language_id])");
	{
		EQEmu::ItemInstance *THIS;
		Const_char          *text;
		int lang = 0;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

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
		Perl_croak(aTHX_ "Usage: QuestItem::IsType(THIS, type)");
	{
		EQEmu::ItemInstance *THIS;
		bool   RETVAL;
		uint32 type = (int32) SvIV(ST(1));

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

		RETVAL = THIS->IsType((EQEmu::item::ItemClass) type);
		ST(0)       = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_QuestItem_IsAttuned); /* prototype to pass -Wmissing-prototypes */
XS(XS_QuestItem_IsAttuned) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::IsAttuned(THIS)");
	{
		EQEmu::ItemInstance *THIS;
		bool RETVAL;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

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
		Perl_croak(aTHX_ "Usage: QuestItem::GetCharges(THIS)");
	{
		EQEmu::ItemInstance *THIS;
		int16 RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

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
		Perl_croak(aTHX_ "Usage: QuestItem::GetAugment(THIS, int16 slot_id)");
	{
		EQEmu::ItemInstance *THIS;
		int16 slot_id = (int16) SvIV(ST(1));
		EQEmu::ItemInstance *RETVAL;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

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
		Perl_croak(aTHX_ "Usage: QuestItem::GetID(THIS)");
	{
		EQEmu::ItemInstance *THIS;
		uint32 RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "QuestItem")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(EQEmu::ItemInstance *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type EQEmu::ItemInstance");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

		RETVAL = THIS->GetItem()->ID;
		XSprePUSH;
		PUSHi((IV) RETVAL);
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

	newXSproto(strcpy(buf, "GetName"), XS_QuestItem_GetName, file, "$");
	newXSproto(strcpy(buf, "SetScale"), XS_QuestItem_SetScale, file, "$");
	newXSproto(strcpy(buf, "ItemSay"), XS_QuestItem_ItemSay, file, "$");
	newXSproto(strcpy(buf, "IsType"), XS_QuestItem_IsType, file, "$$");
	newXSproto(strcpy(buf, "IsAttuned"), XS_QuestItem_IsAttuned, file, "$");
	newXSproto(strcpy(buf, "GetCharges"), XS_QuestItem_GetCharges, file, "$");
	newXSproto(strcpy(buf, "GetAugment"), XS_QuestItem_GetAugment, file, "$$");
	newXSproto(strcpy(buf, "GetID"), XS_QuestItem_GetID, file, "$");

	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
