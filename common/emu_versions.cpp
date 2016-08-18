/*	EQEMu: Everquest Server Emulator
	
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

#include "emu_versions.h"


bool EQEmu::versions::IsValidClientVersion(ClientVersion client_version)
{
	if (client_version <= ClientVersion::Unknown || client_version > LastClientVersion)
		return false;

	return true;
}

EQEmu::versions::ClientVersion EQEmu::versions::ValidateClientVersion(ClientVersion client_version)
{
	if (client_version <= ClientVersion::Unknown || client_version > LastClientVersion)
		return ClientVersion::Unknown;

	return client_version;
}

const char* EQEmu::versions::ClientVersionName(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Unknown:
		return "Unknown Version";
	case ClientVersion::Client62:
		return "Client 6.2";
	case ClientVersion::Titanium:
		return "Titanium";
	case ClientVersion::SoF:
		return "SoF";
	case ClientVersion::SoD:
		return "SoD";
	case ClientVersion::UF:
		return "UF";
	case ClientVersion::RoF:
		return "RoF";
	case ClientVersion::RoF2:
		return "RoF2";
	default:
		return "Invalid Version";
	};
}

uint32 EQEmu::versions::ConvertClientVersionToClientVersionBit(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Unknown:
	case ClientVersion::Client62:
		return bit_Unknown;
	case ClientVersion::Titanium:
		return bit_Titanium;
	case ClientVersion::SoF:
		return bit_SoF;
	case ClientVersion::SoD:
		return bit_SoD;
	case ClientVersion::UF:
		return bit_UF;
	case ClientVersion::RoF:
		return bit_RoF;
	case ClientVersion::RoF2:
		return bit_RoF2;
	default:
		return bit_Unknown;
	}
}

EQEmu::versions::ClientVersion EQEmu::versions::ConvertClientVersionBitToClientVersion(uint32 client_version_bit)
{
	switch (client_version_bit) {
	case (uint32)static_cast<unsigned int>(ClientVersion::Unknown) :
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::Client62) - 1)) :
		return ClientVersion::Unknown;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::Titanium) - 1)) :
		return ClientVersion::Titanium;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::SoF) - 1)) :
		return ClientVersion::SoF;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::SoD) - 1)) :
		return ClientVersion::SoD;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::UF) - 1)) :
		return ClientVersion::UF;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::RoF) - 1)) :
		return ClientVersion::RoF;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::RoF2) - 1)) :
		return ClientVersion::RoF2;
	default:
		return ClientVersion::Unknown;
	}
}

uint32 EQEmu::versions::ConvertClientVersionToExpansion(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Unknown:
	case ClientVersion::Client62:
	case ClientVersion::Titanium:
		return 0x000007FFU;
	case ClientVersion::SoF:
		return 0x00007FFFU;
	case ClientVersion::SoD:
		return 0x0000FFFFU;
	case ClientVersion::UF:
		return 0x0001FFFFU;
	case ClientVersion::RoF:
	case ClientVersion::RoF2:
		return 0x000FFFFFU;
	default:
		return 0;
	}
}

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

bool EQEmu::versions::IsValidOfflinePCInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= LastNonPCInventoryVersion || inventory_version > LastOfflinePCInventoryVersion)
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

EQEmu::versions::InventoryVersion EQEmu::versions::ValidateOfflinePCInventoryVersion(InventoryVersion inventory_version)
{
	if (inventory_version <= LastNonPCInventoryVersion || inventory_version > LastOfflinePCInventoryVersion)
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
	case InventoryVersion::NPCMerchant:
		return "NPC Merchant";
	case InventoryVersion::Merc:
		return "Merc";
	case InventoryVersion::Bot:
		return "Bot";
	case InventoryVersion::ClientPet:
		return "Client Pet";
	case InventoryVersion::NPCPet:
		return "NPC Pet";
	case InventoryVersion::MercPet:
		return "Merc Pet";
	case InventoryVersion::BotPet:
		return "Bot Pet";
	case InventoryVersion::OfflineTitanium:
		return "Offline Titanium";
	case InventoryVersion::OfflineSoF:
		return "Offline SoF";
	case InventoryVersion::OfflineSoD:
		return "Offline SoD";
	case InventoryVersion::OfflineUF:
		return "Offline UF";
	case InventoryVersion::OfflineRoF:
		return "Offline RoF";
	case InventoryVersion::OfflineRoF2:
		return "Offline RoF2";
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

EQEmu::versions::InventoryVersion EQEmu::versions::ConvertPCInventoryVersionToOfflinePCInventoryVersion(InventoryVersion inventory_version)
{
	switch (inventory_version) {
	case InventoryVersion::Titanium:
		return InventoryVersion::OfflineTitanium;
	case InventoryVersion::SoF:
		return InventoryVersion::OfflineSoF;
	case InventoryVersion::SoD:
		return InventoryVersion::OfflineSoD;
	case InventoryVersion::UF:
		return InventoryVersion::OfflineUF;
	case InventoryVersion::RoF:
		return InventoryVersion::OfflineRoF;
	case InventoryVersion::RoF2:
		return InventoryVersion::OfflineRoF2;
	default:
		return InventoryVersion::Unknown;
	}
}

EQEmu::versions::InventoryVersion EQEmu::versions::ConvertOfflinePCInventoryVersionToPCInventoryVersion(InventoryVersion inventory_version)
{
	switch (inventory_version) {
	case InventoryVersion::OfflineTitanium:
		return InventoryVersion::Titanium;
	case InventoryVersion::OfflineSoF:
		return InventoryVersion::SoF;
	case InventoryVersion::OfflineSoD:
		return InventoryVersion::SoD;
	case InventoryVersion::OfflineUF:
		return InventoryVersion::UF;
	case InventoryVersion::OfflineRoF:
		return InventoryVersion::RoF;
	case InventoryVersion::OfflineRoF2:
		return InventoryVersion::RoF2;
	default:
		return InventoryVersion::Unknown;
	}
}

EQEmu::versions::ClientVersion EQEmu::versions::ConvertOfflinePCInventoryVersionToClientVersion(InventoryVersion inventory_version)
{
	switch (inventory_version) {
	case InventoryVersion::OfflineTitanium:
		return ClientVersion::Titanium;
	case InventoryVersion::OfflineSoF:
		return ClientVersion::SoF;
	case InventoryVersion::OfflineSoD:
		return ClientVersion::SoD;
	case InventoryVersion::OfflineUF:
		return ClientVersion::UF;
	case InventoryVersion::OfflineRoF:
		return ClientVersion::RoF;
	case InventoryVersion::OfflineRoF2:
		return ClientVersion::RoF2;
	default:
		return ClientVersion::Unknown;
	}
}

EQEmu::versions::InventoryVersion EQEmu::versions::ConvertClientVersionToOfflinePCInventoryVersion(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Titanium:
		return InventoryVersion::OfflineTitanium;
	case ClientVersion::SoF:
		return InventoryVersion::OfflineSoF;
	case ClientVersion::SoD:
		return InventoryVersion::OfflineSoD;
	case ClientVersion::UF:
		return InventoryVersion::OfflineUF;
	case ClientVersion::RoF:
		return InventoryVersion::OfflineRoF;
	case ClientVersion::RoF2:
		return InventoryVersion::OfflineRoF2;
	default:
		return InventoryVersion::Unknown;
	}
}
