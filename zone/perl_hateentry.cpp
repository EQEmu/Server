/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2009 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/linked_list.h"
#include "hate_list.h"

#ifdef THIS        /* this macro seems to leak out on some systems */
#undef THIS
#endif

XS(XS_HateEntry_GetEnt); /* prototype to pass -Wmissing-prototypes */
XS(XS_HateEntry_GetEnt) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: HateEntry::GetData(THIS)");
	{
		struct_HateList *THIS;
		Mob             *RETVAL;

		if (sv_derived_from(ST(0), "HateEntry")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(struct_HateList *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type tHateEntry");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

		RETVAL = THIS->entity_on_hatelist;
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "Mob", (void *) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_HateEntry_GetHate); /* prototype to pass -Wmissing-prototypes */
XS(XS_HateEntry_GetHate) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: HateEntry::GetHate(THIS)");
	{
		struct_HateList *THIS;
		int32 RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "HateEntry")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(struct_HateList *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type tHateEntry");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

		RETVAL = THIS->stored_hate_amount;
		XSprePUSH;
		PUSHi((IV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_HateEntry_GetDamage); /* prototype to pass -Wmissing-prototypes */
XS(XS_HateEntry_GetDamage) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: HateEntry::GetDamage(THIS)");
	{
		struct_HateList *THIS;
		int32 RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "HateEntry")) {
			IV tmp = SvIV((SV *) SvRV(ST(0)));
			THIS = INT2PTR(struct_HateList *, tmp);
		} else
			Perl_croak(aTHX_ "THIS is not of type tHateEntry");
		if (THIS == nullptr)
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash.");

		RETVAL = THIS->hatelist_damage;
		XSprePUSH;
		PUSHi((IV) RETVAL);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_HateEntry);
XS(boot_HateEntry) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK;

	newXSproto(strcpy(buf, "GetEnt"), XS_HateEntry_GetEnt, file, "$");
	newXSproto(strcpy(buf, "GetDamage"), XS_HateEntry_GetDamage, file, "$");
	newXSproto(strcpy(buf, "GetHate"), XS_HateEntry_GetHate, file, "$");

	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

