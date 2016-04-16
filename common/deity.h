/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#ifndef DEITY_H
#define DEITY_H

#include "types.h"


namespace EQEmu
{
	class Deity {
	public:
		enum Types {
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

		enum TypeBits : uint32 {
			BIT_DeityAll = 0x00000000,
			BIT_DeityAgnostic = 0x00000001,
			BIT_DeityBertoxxulous = 0x00000002,
			BIT_DeityBrellSirilis = 0x00000004,
			BIT_DeityCazicThule = 0x00000008,
			BIT_DeityErollisiMarr = 0x00000010,
			BIT_DeityBristlebane = 0x00000020,
			BIT_DeityInnoruuk = 0x00000040,
			BIT_DeityKarana = 0x00000080,
			BIT_DeityMithanielMarr = 0x00000100,
			BIT_DeityPrexus = 0x00000200,
			BIT_DeityQuellious = 0x00000400,
			BIT_DeityRallosZek = 0x00000800,
			BIT_DeityRodcetNife = 0x00001000,
			BIT_DeitySolusekRo = 0x00002000,
			BIT_DeityTheTribunal = 0x00004000,
			BIT_DeityTunare = 0x00008000,
			BIT_DeityVeeshan = 0x00010000
		};

		static TypeBits ConvertDeityToDeityBit(Types deity);
		static Types ConvertDeityBitToDeity(TypeBits deity_bit);
		static const char* GetDeityName(Types deity);
	};
}

#endif /* DEITY_H */
