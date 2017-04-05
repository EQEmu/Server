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

#include "emu_legacy.h"
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
			size_t CharacterCreationLimit;
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
			size_t InventoryTypeSize[25]; // should reflect EQEmu::inventory::typeCount referenced in emu_constants.h

			uint64 PossessionsBitmask;
			size_t ItemBagSize;
			size_t ItemAugSize;

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
	enum : int { Invalid = -1, Null, Safety };

	enum : bool { False = false, True = true };

} /*ClientUnknown*/

namespace Client62
{
	enum : int { Invalid = -1, Null, Safety };

	enum : bool { False = false, True = true };

} /*Client62*/

#endif /*COMMON_EQ_LIMITS_H*/
