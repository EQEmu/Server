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

#ifndef COMMON_EMU_CONSTANTS_H
#define COMMON_EMU_CONSTANTS_H

#include "eq_limits.h"
// (future use)
//using namespace RoF2::maps;	// server inventory maps enumeration (code and database sync'd to reference)
//using namespace RoF::slots;	// server possessions slots enumeration (code and database sync'd to reference)

#include "emu_legacy.h"
#include "inventory_version.h"
#include "light_source.h"
#include "deity.h"
#include "say_link.h"

#include <string>


// *** DO NOT CHANGE without a full understanding of the consequences..the server is set up to use these settings explicitly!! ***
// *** You will cause compilation failures and corrupt your database if partial or incorrect attempts to change them are made!! ***

// Hard-coded values usually indicate that further research is needed and the values given are from the old (known) system


namespace EQEmu
{
	// an immutable value is required to initialize arrays, etc... use this class as a repository for those
	namespace constants {
		// database
		static const EQEmu::versions::ClientVersion CharacterCreationClient = EQEmu::versions::ClientVersion::RoF2;
		static const size_t CharacterCreationLimit = RoF2::consts::CHARACTER_CREATION_LIMIT;

		// inventory
		extern uint16 InventoryTypeSize(int16 type_index);
		//extern const char* InventoryLocationName(Location_Struct location);
		extern const char* InventoryTypeName(int16 type_index);
		extern const char* InventorySlotName(int16 slot_index);
		extern const char* InventorySubName(int16 sub_index);
		extern const char* InventoryAugName(int16 aug_index);
	}
}

#endif /* COMMON_EMU_CONSTANTS_H */
