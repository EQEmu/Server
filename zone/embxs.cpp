/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "embxs.h"
#include "embperl.h"
#include "masterentity.h"
#include "command.h"
#include "bot_command.h"

const char *getItemName(unsigned itemid)
{
	const EQ::ItemData* item = nullptr;
  item = database.GetItem(itemid);

  if (item)
    return item->Name;
  else
    return nullptr;
}

const char* Perl__qc_getItemName(unsigned itemid)
{
	return getItemName(itemid); // possible nullptr return
}

#ifdef EMBPERL_IO_CAPTURE

XS(XS_EQEmuIO_PRINT); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQEmuIO_PRINT)
{
    dXSARGS;
    if (items < 2)
    	return;
//        Perl_croak(aTHX_ "Usage: EQEmuIO::PRINT(@strings)");

	int r;
	for (r = 1; r < items; r++) {
		char *str = SvPV_nolen(ST(r));
		char *cur = str;

		/* Strip newlines from log message 'str' */
		*std::remove(str, str + strlen(str), '\n') = '\0';

		std::string log_string = str;

		if (log_string.find("did not return a true") != std::string::npos) {
			return;
		}

		if (log_string.find("is experimental") != std::string::npos) {
			return;
		}

		int i;
		int pos = 0;
		int len = 0;

		for (i = 0; *cur != '\0'; i++, cur++) {
			if (*cur == '\n') {
				LogQuests("{}", str);
				len = 0;
				pos = i + 1;
			}
			else {
				len++;
			}
		}
		if (!log_string.empty()) {
			LogQuests("{}", log_string);
		}
	}

	XSRETURN_EMPTY;
}
#endif //EMBPERL_IO_CAPTURE

#endif // EMBPERL
