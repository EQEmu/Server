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

#ifdef _WINDOWS
	#if (!defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1900))
		#define snprintf	_snprintf
		#define vsnprintf	_vsnprintf
	#endif
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#endif

#include "../common/races.h"
#include "encounter.h"
#include "entity.h"
#include "mob.h"

class Zone;

Encounter::Encounter(const char* enc_name)
	:Mob
	(
	nullptr, nullptr, 0, 0, 0, INVISIBLE_MAN, 0, BT_NoTarget, 0, 0, 0, 0, 0, glm::vec4(0,0,0,0), 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EQEmu::TintProfile(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	)
{
	encounter_name[0] = 0;
	strn0cpy(encounter_name, enc_name, 64);
	remove_me = false;
}

Encounter::~Encounter()
{

}

bool Encounter::Process() {
	if (remove_me) return false;
	return true;
}
