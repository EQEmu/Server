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

#ifndef EQ_LIMITS_H
#define EQ_LIMITS_H

#include "types.h"
#include "eq_constants.h"
#include "clientversions.h"
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
	// values should default to a non-beneficial value..unless value conflicts with intended operation
	//
	// EQEmu::Constants may be used as references..but, not every reference needs to be in EQEmu::Constants (i.e., AllowsEmptyBagInBag(), CoinHasWeight(), etc...)
	class Limits {
	public:
		// client version validation (checks to avoid crashing zone server when accessing reference arrays)
		// use this inside of class Client (limits to actual clients)
		static bool IsValidPCClientVersion(ClientVersion clientVersion);
		static ClientVersion ValidatePCClientVersion(ClientVersion clientVersion);

		// basically..any non-client classes - do not invoke when setting a valid client
		static bool IsValidNPCClientVersion(ClientVersion clientVersion);
		static ClientVersion ValidateNPCClientVersion(ClientVersion clientVersion);

		// these are 'universal' - do not invoke when setting a valid client
		static bool IsValidMobClientVersion(ClientVersion clientVersion);
		static ClientVersion ValidateMobClientVersion(ClientVersion clientVersion);

		// database
		static size_t CharacterCreationLimit(ClientVersion clientVersion);

		// inventory
		static uint16 InventoryMapSize(int16 indexMap, ClientVersion clientVersion);
		static uint64 PossessionsBitmask(ClientVersion clientVersion);
		static uint64 EquipmentBitmask(ClientVersion clientVersion);
		static uint64 GeneralBitmask(ClientVersion clientVersion);
		static uint64 CursorBitmask(ClientVersion clientVersion);

		static bool AllowsEmptyBagInBag(ClientVersion clientVersion);
		static bool AllowsClickCastFromBag(ClientVersion clientVersion);

		// items
		static uint16 ItemCommonSize(ClientVersion clientVersion);
		static uint16 ItemContainerSize(ClientVersion clientVersion);

		// player profile
		static bool CoinHasWeight(ClientVersion clientVersion);
	};
}

#endif /* EQ_LIMITS_H */
