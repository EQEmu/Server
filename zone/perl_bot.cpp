#ifdef BOTS
#include "../common/features.h"
#ifdef EMBPERL_XS_CLASSES
#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "bot.h"

#ifdef THIS	
#undef THIS
#endif

#define VALIDATE_THIS_IS_BOT \
	do { \
		if (sv_derived_from(ST(0), "Bot")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(Bot*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type Bot"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Bot_GetOwner);
XS(XS_Bot_GetOwner)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Bot::GetOwner(THIS)"); // @categories Script Utility, Bot
	{
		Bot* THIS;
		Mob* bot_owner;
        VALIDATE_THIS_IS_BOT;
		bot_owner = THIS->GetBotOwner();
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "Mob", (void*)bot_owner);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_Bot);
XS(boot_Bot)
{
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_Bot does not take any arguments.");

	char buf[128];

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "GetOwner"), XS_Bot_GetOwner, file, "$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
#endif //BOTS