/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#ifndef COMMON_EMU_LIMITS_H
#define COMMON_EMU_LIMITS_H

#include "types.h"

#include <stdlib.h>


namespace EntityLimits
{
	namespace NPC {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*NPC*/

	namespace NPCMerchant {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*NPCMerchant*/

	namespace Merc {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*Merc*/

	namespace Bot {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 8;

	} /*Bot*/

	namespace ClientPet {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*Pet*/

	namespace NPCPet {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*Pet*/

	namespace MercPet {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*Pet*/

	namespace BotPet {
		enum : int { Invalid = -1, Null, Safety };

		enum : bool { False = false, True = true };

		const size_t InvTypeTradeSize = 4;

	} /*Pet*/

}; /*EntityLimits*/

#endif /*COMMON_EMU_LIMITS_H*/
