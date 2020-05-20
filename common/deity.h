/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_DEITY_H
#define COMMON_DEITY_H

#include "types.h"


namespace EQ
{
	namespace deity {
		enum DeityType {
			DeityUnknown = 0,
			DeityAgnostic_LB = 140,
			DeityBertoxxulous = 201,
			DeityBrellSirilis,
			DeityCazicThule,
			DeityErollisiMarr,
			DeityBristlebane,
			DeityInnoruuk,
			DeityKarana,
			DeityMithanielMarr,
			DeityPrexus,
			DeityQuellious,
			DeityRallosZek,
			DeityRodcetNife,
			DeitySolusekRo,
			DeityTheTribunal,
			DeityTunare,
			DeityVeeshan,
			DeityAgnostic = 396
		};

		enum DeityTypeBit : uint32 {
			bit_DeityNone = 0x00000000,
			bit_DeityAgnostic = 0x00000001,
			bit_DeityBertoxxulous = 0x00000002,
			bit_DeityBrellSirilis = 0x00000004,
			bit_DeityCazicThule = 0x00000008,
			bit_DeityErollisiMarr = 0x00000010,
			bit_DeityBristlebane = 0x00000020,
			bit_DeityInnoruuk = 0x00000040,
			bit_DeityKarana = 0x00000080,
			bit_DeityMithanielMarr = 0x00000100,
			bit_DeityPrexus = 0x00000200,
			bit_DeityQuellious = 0x00000400,
			bit_DeityRallosZek = 0x00000800,
			bit_DeityRodcetNife = 0x00001000,
			bit_DeitySolusekRo = 0x00002000,
			bit_DeityTheTribunal = 0x00004000,
			bit_DeityTunare = 0x00008000,
			bit_DeityVeeshan = 0x00010000,
			bit_DeityAll = 0xFFFFFFFF
		};

		extern DeityTypeBit ConvertDeityTypeToDeityTypeBit(DeityType deity_type);
		extern DeityType ConvertDeityTypeBitToDeityType(DeityTypeBit deity_type_bit);
		extern const char* DeityName(DeityType deity_type);

	} /*deity*/

} /*EQEmu*/

#endif /* COMMON_DEITY_H */
