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

#define VALIDATE_THIS_IS_HATE \
	do { \
		if (sv_derived_from(ST(0), "HateEntry")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(struct_HateList*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type HateEntry"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_HateEntry_GetEnt); /* prototype to pass -Wmissing-prototypes */
XS(XS_HateEntry_GetEnt) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: HateEntry::GetEnt(THIS)"); // @categories Script Utility, Hate and Aggro
	{
		struct_HateList *THIS;
		Mob             *RETVAL;
		VALIDATE_THIS_IS_HATE;
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
		Perl_croak(aTHX_ "Usage: HateEntry::GetHate(THIS)"); // @categories Script Utility, Hate and Aggro
	{
		struct_HateList *THIS;
		int64 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_HATE;
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
		Perl_croak(aTHX_ "Usage: HateEntry::GetDamage(THIS)"); // @categories Script Utility, Hate and Aggro
	{
		struct_HateList *THIS;
		int64 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_HATE;
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
	newXSproto(strcpy(buf, "GetDamage"), XS_HateEntry_GetDamage, file, "$");
	newXSproto(strcpy(buf, "GetEnt"), XS_HateEntry_GetEnt, file, "$");
	newXSproto(strcpy(buf, "GetHate"), XS_HateEntry_GetHate, file, "$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

