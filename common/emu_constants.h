/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_EMU_CONSTANTS_H
#define COMMON_EMU_CONSTANTS_H

#include "eq_limits.h"
#include "emu_legacy.h"
#include "emu_versions.h"

#include <string>


namespace EQEmu
{
	namespace inventory {
		//using namespace RoF2::invtype;
		//using namespace RoF2::invslot;
		//using namespace RoF2::invbag;
		//using namespace RoF2::invaug;

	} /*inventory*/

	namespace constants {
		const EQEmu::versions::ClientVersion CharacterCreationClient = EQEmu::versions::ClientVersion::RoF2;
		const size_t CharacterCreationMax = RoF2::constants::CharacterCreationLimit;

		const size_t SayLinkBodySize = RoF2::constants::SayLinkBodySize;

	} /*constants*/

} /*EQEmu*/

#endif /*COMMON_EMU_CONSTANTS_H*/
