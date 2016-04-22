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
#include "inventory_version.h" // inv2 watch
#include "../common/patches/titanium_constants.h"
#include "../common/patches/sof_constants.h"
#include "../common/patches/sod_constants.h"
#include "../common/patches/uf_constants.h"
#include "../common/patches/rof_constants.h"
#include "../common/patches/rof2_constants.h"


// *** DO NOT CHANGE without a full understanding of the consequences..the server is set up to use these settings explicitly!! ***
// *** You will cause compilation failures and corrupt your database if partial or incorrect attempts to change them are made!! ***

// Hard-coded values usually indicate that further research is needed and the values given are from the old (known) system


namespace EQEmu
{
	namespace limits {
		// database
		extern size_t CharacterCreationLimit(versions::ClientVersion client_version);

		// inventory
		extern uint16 InventoryTypeSize(versions::InventoryVersion inventory_version, int16 inv_type);
		extern uint64 PossessionsBitmask(versions::InventoryVersion inventory_version);
		extern uint64 EquipmentBitmask(versions::InventoryVersion inventory_version);
		extern uint64 GeneralBitmask(versions::InventoryVersion inventory_version);
		extern uint64 CursorBitmask(versions::InventoryVersion inventory_version);

		extern bool AllowEmptyBagInBag(versions::InventoryVersion inventory_version);
		extern bool AllowClickCastFromBag(versions::InventoryVersion inventory_version);

		// items
		extern uint16 ItemCommonSize(versions::InventoryVersion inventory_version);
		extern uint16 ItemContainerSize(versions::InventoryVersion inventory_version);

		// player profile
		extern bool CoinHasWeight(versions::InventoryVersion inventory_version);
	}
}

#endif /* COMMON_EQ_LIMITS_H */
