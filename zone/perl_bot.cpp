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