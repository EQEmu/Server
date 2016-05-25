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

#ifndef COMMON_EQ_LIMITS_H
#define COMMON_EQ_LIMITS_H

#include "types.h"
#include "eq_constants.h"
#include "inventory_version.h"
#include "../common/patches/titanium_limits.h"
#include "../common/patches/sof_limits.h"
#include "../common/patches/sod_limits.h"
#include "../common/patches/uf_limits.h"
#include "../common/patches/rof_limits.h"
#include "../common/patches/rof2_limits.h"


namespace EQEmu
{
	namespace constants {
		extern size_t CharacterCreationLimit(versions::ClientVersion client_version);

	} /*constants*/
	
	namespace inventory {
		extern uint16 InventoryTypeSize(versions::InventoryVersion inventory_version, int16 inv_type);
		extern uint64 PossessionsBitmask(versions::InventoryVersion inventory_version);

		extern bool AllowEmptyBagInBag(versions::InventoryVersion inventory_version);
		extern bool AllowClickCastFromBag(versions::InventoryVersion inventory_version);

		extern uint16 ItemAugSize(versions::InventoryVersion inventory_version);
		extern uint16 ItemBagSize(versions::InventoryVersion inventory_version);

		extern bool ConcatenateInvTypeLimbo(versions::InventoryVersion inventory_version);

		extern bool AllowOverLevelEquipment(versions::InventoryVersion inventory_version);

	} /*inventory*/
	
	namespace profile {
		extern bool CoinHasWeight(versions::InventoryVersion inventory_version);

	} /*profile*/

} /*EQEmu*/

namespace ClientUnknown
{
	enum : int { Invalid = -1, Null, Safety };

	enum : bool { False = false, True = true };

} /*ClientUnknown*/

namespace Client62
{
	enum : int { Invalid = -1, Null, Safety };

	enum : bool { False = false, True = true };

} /*Client62*/

#endif /*COMMON_EQ_LIMITS_H*/
