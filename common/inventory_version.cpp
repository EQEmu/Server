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

#include "inventory_version.h"


bool EQEmu::versions::IsValidInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= InventoryVersion::Unknown || inventory_version > LastInventoryVersion)
		return false;

	return true;
}

bool EQEmu::versions::IsValidPCInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= InventoryVersion::Unknown || inventory_version > LastPCInventoryVersion)
		return false;

	return true;
}

bool EQEmu::versions::IsValidNonPCInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= LastPCInventoryVersion || inventory_version > LastNonPCInventoryVersion)
		return false;

	return true;
}

EQEmu::versions::InventoryVersion EQEmu::versions::ValidateInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= InventoryVersion::Unknown || inventory_version > LastInventoryVersion)
		return InventoryVersion::Unknown;

	return inventory_version;
}

EQEmu::versions::InventoryVersion EQEmu::versions::ValidatePCInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= InventoryVersion::Unknown || inventory_version > LastPCInventoryVersion)
		return InventoryVersion::Unknown;

	return inventory_version;
}

EQEmu::versions::InventoryVersion EQEmu::versions::ValidateNonPCInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= LastPCInventoryVersion || inventory_version > LastNonPCInventoryVersion)
		return InventoryVersion::Unknown;

	return inventory_version;
}

const char* EQEmu::versions::InventoryVersionName(InventoryVersion inventory_version)
{
	switch (inventory_version) {
	case InventoryVersion::Unknown:
		return "Unknown Version";
	case InventoryVersion::Client62:
		return "Client 6.2";
	case InventoryVersion::Titanium:
		return "Titanium";
	case InventoryVersion::SoF:
		return "SoF";
	case InventoryVersion::SoD:
		return "SoD";
	case InventoryVersion::UF:
		return "UF";
	case InventoryVersion::RoF:
		return "RoF";
	case InventoryVersion::RoF2:
		return "RoF2";
	case InventoryVersion::NPC:
		return "NPC";
	case InventoryVersion::Merc:
		return "Merc";
	case InventoryVersion::Bot:
		return "Bot";
	case InventoryVersion::Pet:
		return "Pet";
	default:
		return "Invalid Version";
	};
}

EQEmu::versions::ClientVersion EQEmu::versions::ConvertInventoryVersionToClientVersion(InventoryVersion inventory_version)
{
	switch (inventory_version) {
	case InventoryVersion::Unknown:
	case InventoryVersion::Client62:
		return ClientVersion::Unknown;
	case InventoryVersion::Titanium:
		return ClientVersion::Titanium;
	case InventoryVersion::SoF:
		return ClientVersion::SoF;
	case InventoryVersion::SoD:
		return ClientVersion::SoD;
	case InventoryVersion::UF:
		return ClientVersion::UF;
	case InventoryVersion::RoF:
		return ClientVersion::RoF;
	case InventoryVersion::RoF2:
		return ClientVersion::RoF2;
	default:
		return ClientVersion::Unknown;
	}
}

EQEmu::versions::InventoryVersion EQEmu::versions::ConvertClientVersionToInventoryVersion(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Unknown:
	case ClientVersion::Client62:
		return InventoryVersion::Unknown;
	case ClientVersion::Titanium:
		return InventoryVersion::Titanium;
	case ClientVersion::SoF:
		return InventoryVersion::SoF;
	case ClientVersion::SoD:
		return InventoryVersion::SoD;
	case ClientVersion::UF:
		return InventoryVersion::UF;
	case ClientVersion::RoF:
		return InventoryVersion::RoF;
	case ClientVersion::RoF2:
		return InventoryVersion::RoF2;
	default:
		return InventoryVersion::Unknown;
	}
}
