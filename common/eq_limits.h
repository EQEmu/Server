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

#ifndef COMMON_EQ_LIMITS_H
#define COMMON_EQ_LIMITS_H

#include "types.h"
#include "eq_constants.h"
#include "emu_versions.h"
#include "../common/patches/titanium_limits.h"
#include "../common/patches/sof_limits.h"
#include "../common/patches/sod_limits.h"
#include "../common/patches/uf_limits.h"
#include "../common/patches/rof_limits.h"
#include "../common/patches/rof2_limits.h"


namespace EQEmu
{
	namespace constants {
		extern size_t GetCharacterCreationLimit(versions::ClientVersion client_version);

	} /*constants*/
	
	namespace inventory {
		extern size_t GetInventoryTypeSize(versions::InventoryVersion inventory_version, int inv_type);
		extern uint64 GetPossessionsBitmask(versions::InventoryVersion inventory_version);

		extern bool GetAllowEmptyBagInBag(versions::InventoryVersion inventory_version);
		extern bool GetAllowClickCastFromBag(versions::InventoryVersion inventory_version);

		extern bool GetConcatenateInvTypeLimbo(versions::InventoryVersion inventory_version);

		extern bool GetAllowOverLevelEquipment(versions::InventoryVersion inventory_version);

		extern size_t GetItemAugSize(versions::InventoryVersion inventory_version);
		extern size_t GetItemBagSize(versions::InventoryVersion inventory_version);

	} /*inventory*/
	
	namespace behavior {
		extern bool GetCoinHasWeight(versions::InventoryVersion inventory_version);

	} /*behavior*/

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
