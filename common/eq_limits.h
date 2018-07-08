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
		class LookupEntry {
		public:
			int16 CharacterCreationLimit;
			int LongBuffs;
			int ShortBuffs;
			int DiscBuffs;
			int TotalBuffs;
			int NPCBuffs;
			int PetBuffs;
			int MercBuffs;
		};

		const LookupEntry* Lookup(versions::ClientVersion client_version);

	} /*constants*/
	
	namespace inventory {
		class LookupEntry {
		public:
			// note: 'PossessionsBitmask' needs to be attuned to the client version with the highest number
			// of possessions slots and 'InventoryTypeSize[typePossessions]' should reflect the same count
			// with translators adjusting for valid slot indices. Server-side validations will be performed
			// against 'PossessionsBitmask' (note: the same applies to Corpse type size and bitmask)

			int16 InventoryTypeSize[25]; // should reflect EQEmu::invtype::TYPE_COUNT referenced in emu_constants.h

			uint64 PossessionsBitmask;
			uint64 CorpseBitmask;
			int16 BagSlotCount;
			int16 AugSocketCount;

			bool AllowEmptyBagInBag;
			bool AllowClickCastFromBag;
			bool ConcatenateInvTypeLimbo;
			bool AllowOverLevelEquipment;
		};

		const LookupEntry* Lookup(versions::MobVersion mob_version);

	} /*inventory*/
	
	namespace behavior {
		class LookupEntry {
		public:
			bool CoinHasWeight;
		};

		const LookupEntry* Lookup(versions::MobVersion mob_version);

	} /*behavior*/

} /*EQEmu*/

namespace ClientUnknown
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

} /*ClientUnknown*/

namespace Client62
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

} /*Client62*/

#endif /*COMMON_EQ_LIMITS_H*/
