/*	EQEMu:  Everquest Server Emulator
	
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

#ifndef COMMON_CLIENT_VERSION_H
#define COMMON_CLIENT_VERSION_H

#include "types.h"

#include <stdlib.h>


namespace EQEmu
{
	namespace versions {
		enum class ClientVersion {
			Unknown = 0,
			Client62,	// Build: 'Aug  4 2005 15:40:59'
			Titanium,	// Build: 'Oct 31 2005 10:33:37'
			SoF,		// Build: 'Sep  7 2007 09:11:49'
			SoD,		// Build: 'Dec 19 2008 15:22:49'
			UF,			// Build: 'Jun  8 2010 16:44:32'
			RoF,		// Build: 'Dec 10 2012 17:35:44'
			RoF2		// Build: 'May 10 2013 23:30:08'
		};

		enum ClientVersionBit : uint32 {
			bit_Unknown = 0,
			bit_Client62 = 0x00000001, // unsupported (placeholder for scripts)
			bit_Titanium = 0x00000002,
			bit_SoF = 0x00000004,
			bit_SoD = 0x00000008,
			bit_UF = 0x00000010,
			bit_RoF = 0x00000020,
			bit_RoF2 = 0x00000040,
			bit_TitaniumAndEarlier = 0x00000003,
			bit_SoFAndEarlier = 0x00000007,
			bit_SoDAndEarlier = 0x0000000F,
			bit_UFAndEarlier = 0x0000001F,
			bit_RoFAndEarlier = 0x0000003F,
			bit_SoFAndLater = 0xFFFFFFFC,
			bit_SoDAndLater = 0xFFFFFFF8,
			bit_UFAndLater = 0xFFFFFFF0,
			bit_RoFAndLater = 0xFFFFFFE0,
			bit_RoF2AndLater = 0xFFFFFFC0,
			bit_AllClients = 0xFFFFFFFF
		};

		static const ClientVersion LastClientVersion = ClientVersion::RoF2;
		static const size_t ClientVersionCount = (static_cast<size_t>(LastClientVersion) + 1);

		extern bool IsValidClientVersion(ClientVersion client_version);
		extern ClientVersion ValidateClientVersion(ClientVersion client_version);
		extern const char* ClientVersionName(ClientVersion client_version);
		extern uint32 ConvertClientVersionToClientVersionBit(ClientVersion client_version);
		extern ClientVersion ConvertClientVersionBitToClientVersion(uint32 client_version_bit);
		extern uint32 ConvertClientVersionToExpansion(ClientVersion client_version);
	}
}

#endif /* COMMON_CLIENT_VERSION_H */
