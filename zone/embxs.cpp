/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#ifdef EMBPERL

#include "../common/debug.h"
#include "masterentity.h"
#include "command.h"

#include "embperl.h"
#include "embxs.h"



const char *getItemName(unsigned itemid)
{
	const Item_Struct* item = nullptr;
	item = database.GetItem(itemid);

	if (item)
		return item->Name;
	else
		return nullptr;
}

XS(XS_qc_getItemName); /* prototype to pass -Wmissing-prototypes */
XS(XS_qc_getItemName)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getItemName(itemid)");
	{
		unsigned		itemid = (unsigned)SvUV(ST(0));
		const char *	RETVAL;
		dXSTARG;
	RETVAL = getItemName(itemid);
		sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	}
	XSRETURN(1);
}


EXTERN_C XS(boot_qc); /* prototype to pass -Wmissing-prototypes */
EXTERN_C XS(boot_qc)
{
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = '\0';

	if(items != 1)
		LogFile->write(EQEMuLog::Error, "boot_qc does not take any arguments.");

	char buf[128];	//shouldent have any function names longer than this.

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK ;

	newXS(strcpy(buf, "quest::getItemName"), XS_qc_getItemName, file);

	XSRETURN_YES;
}

#ifdef EMBPERL_IO_CAPTURE

XS(XS_EQEmuIO_PRINT); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQEmuIO_PRINT)
{
	dXSARGS;
	if (items < 2)
		return;
	//Perl_croak(aTHX_ "Usage: EQEmuIO::PRINT(@strings)");

	int r;
	for(r = 1; r < items; r++) {
		char *str = SvPV_nolen(ST(r));
		char *cur = str;

		int i;
		int pos = 0;
		int len = 0;
		for(i = 0; *cur != '\0'; i++, cur++) {
			if(*cur == '\n') {
				LogFile->writebuf(EQEMuLog::Quest, str + pos, 1, len);
				len = 0;
				pos = i+1;
			} else {
				len++;
			}
		}
		if(len > 0) {
			LogFile->writebuf(EQEMuLog::Quest, str + pos, 1, len);
		}
	}

	XSRETURN_EMPTY;
}
#endif //EMBPERL_IO_CAPTURE


#ifdef EMBPERL_COMMANDS

XS(XS_command_add); /* prototype to pass -Wmissing-prototypes */
XS(XS_command_add)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: commands::command_add(name, desc, access)");

	char *name = SvPV_nolen(ST(0));
	char *desc = SvPV_nolen(ST(1));
	int	access = (int)SvIV(ST(2));

	command_add_perl(name, desc, access);

	XSRETURN_EMPTY;
}

#endif //EMBPERL_COMMANDS

#endif // EMBPERL
