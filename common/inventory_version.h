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

#ifndef COMMON_INVENTORY_VERSION_H
#define COMMON_INVENTORY_VERSION_H

#include "client_version.h"


namespace EQEmu
{
	namespace versions {
		enum class InventoryVersion {
			Unknown = 0,
			Client62,
			Titanium,
			SoF,
			SoD,
			UF,
			RoF,
			RoF2,
			NPC,
			Merc,
			Bot,
			Pet
		};

		static const InventoryVersion LastInventoryVersion = InventoryVersion::Pet;
		static const InventoryVersion LastPCInventoryVersion = InventoryVersion::RoF2;
		static const InventoryVersion LastNonPCInventoryVersion = InventoryVersion::Pet;
		static const size_t InventoryVersionCount = (static_cast<size_t>(LastInventoryVersion) + 1);

		extern bool IsValidInventoryVersion(InventoryVersion inventory_version);
		extern bool IsValidPCInventoryVersion(InventoryVersion inventory_version);
		extern bool IsValidNonPCInventoryVersion(InventoryVersion inventory_version);
		extern InventoryVersion ValidateInventoryVersion(InventoryVersion inventory_version);
		extern InventoryVersion ValidatePCInventoryVersion(InventoryVersion inventory_version);
		extern InventoryVersion ValidateNonPCInventoryVersion(InventoryVersion inventory_version);
		extern const char* InventoryVersionName(InventoryVersion inventory_version);
		extern ClientVersion ConvertInventoryVersionToClientVersion(InventoryVersion inventory_version);
		extern InventoryVersion ConvertClientVersionToInventoryVersion(ClientVersion client_version);
	}
}

#endif /* COMMON_INVENTORY_VERSION_H */
