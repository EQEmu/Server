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

bool EQEmu::versions::IsValidMobVersion(MobVersion mob_version)
{
	if (mob_version <= MobVersion::Unknown || mob_version > LastMobVersion)
		return false;

	return true;
}

bool EQEmu::versions::IsValidPCMobVersion(MobVersion mob_version)
{
	if (mob_version <= MobVersion::Unknown || mob_version > LastPCMobVersion)
		return false;

	return true;
}

bool EQEmu::versions::IsValidNonPCMobVersion(MobVersion mob_version)
{
	if (mob_version <= LastPCMobVersion || mob_version > LastNonPCMobVersion)
		return false;

	return true;
}

bool EQEmu::versions::IsValidOfflinePCMobVersion(MobVersion mob_version)
{
	if (mob_version <= LastNonPCMobVersion || mob_version > LastOfflinePCMobVersion)
		return false;

	return true;
}

EQEmu::versions::MobVersion EQEmu::versions::ValidateMobVersion(MobVersion mob_version)
{
	if (mob_version <= MobVersion::Unknown || mob_version > LastMobVersion)
		return MobVersion::Unknown;

	return mob_version;
}

EQEmu::versions::MobVersion EQEmu::versions::ValidatePCMobVersion(MobVersion mob_version)
{
	if (mob_version <= MobVersion::Unknown || mob_version > LastPCMobVersion)
		return MobVersion::Unknown;

	return mob_version;
}

EQEmu::versions::MobVersion EQEmu::versions::ValidateNonPCMobVersion(MobVersion mob_version)
{
	if (mob_version <= LastPCMobVersion || mob_version > LastNonPCMobVersion)
		return MobVersion::Unknown;

	return mob_version;
}

EQEmu::versions::MobVersion EQEmu::versions::ValidateOfflinePCMobVersion(MobVersion mob_version)
{
	if (mob_version <= LastNonPCMobVersion || mob_version > LastOfflinePCMobVersion)
		return MobVersion::Unknown;

	return mob_version;
}

const char* EQEmu::versions::MobVersionName(MobVersion mob_version)
{
	switch (mob_version) {
	case MobVersion::Unknown:
		return "Unknown Version";
	case MobVersion::Client62:
		return "Client 6.2";
	case MobVersion::Titanium:
		return "Titanium";
	case MobVersion::SoF:
		return "SoF";
	case MobVersion::SoD:
		return "SoD";
	case MobVersion::UF:
		return "UF";
	case MobVersion::RoF:
		return "RoF";
	case MobVersion::RoF2:
		return "RoF2";
	case MobVersion::NPC:
		return "NPC";
	case MobVersion::NPCMerchant:
		return "NPC Merchant";
	case MobVersion::Merc:
		return "Merc";
	case MobVersion::Bot:
		return "Bot";
	case MobVersion::ClientPet:
		return "Client Pet";
	case MobVersion::NPCPet:
		return "NPC Pet";
	case MobVersion::MercPet:
		return "Merc Pet";
	case MobVersion::BotPet:
		return "Bot Pet";
	case MobVersion::OfflineTitanium:
		return "Offline Titanium";
	case MobVersion::OfflineSoF:
		return "Offline SoF";
	case MobVersion::OfflineSoD:
		return "Offline SoD";
	case MobVersion::OfflineUF:
		return "Offline UF";
	case MobVersion::OfflineRoF:
		return "Offline RoF";
	case MobVersion::OfflineRoF2:
		return "Offline RoF2";
	default:
		return "Invalid Version";
	};
}

EQEmu::versions::ClientVersion EQEmu::versions::ConvertMobVersionToClientVersion(MobVersion mob_version)
{
	switch (mob_version) {
	case MobVersion::Unknown:
	case MobVersion::Client62:
		return ClientVersion::Unknown;
	case MobVersion::Titanium:
		return ClientVersion::Titanium;
	case MobVersion::SoF:
		return ClientVersion::SoF;
	case MobVersion::SoD:
		return ClientVersion::SoD;
	case MobVersion::UF:
		return ClientVersion::UF;
	case MobVersion::RoF:
		return ClientVersion::RoF;
	case MobVersion::RoF2:
		return ClientVersion::RoF2;
	default:
		return ClientVersion::Unknown;
	}
}

EQEmu::versions::MobVersion EQEmu::versions::ConvertClientVersionToMobVersion(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Unknown:
	case ClientVersion::Client62:
		return MobVersion::Unknown;
	case ClientVersion::Titanium:
		return MobVersion::Titanium;
	case ClientVersion::SoF:
		return MobVersion::SoF;
	case ClientVersion::SoD:
		return MobVersion::SoD;
	case ClientVersion::UF:
		return MobVersion::UF;
	case ClientVersion::RoF:
		return MobVersion::RoF;
	case ClientVersion::RoF2:
		return MobVersion::RoF2;
	default:
		return MobVersion::Unknown;
	}
}

EQEmu::versions::MobVersion EQEmu::versions::ConvertPCMobVersionToOfflinePCMobVersion(MobVersion mob_version)
{
	switch (mob_version) {
	case MobVersion::Titanium:
		return MobVersion::OfflineTitanium;
	case MobVersion::SoF:
		return MobVersion::OfflineSoF;
	case MobVersion::SoD:
		return MobVersion::OfflineSoD;
	case MobVersion::UF:
		return MobVersion::OfflineUF;
	case MobVersion::RoF:
		return MobVersion::OfflineRoF;
	case MobVersion::RoF2:
		return MobVersion::OfflineRoF2;
	default:
		return MobVersion::Unknown;
	}
}

EQEmu::versions::MobVersion EQEmu::versions::ConvertOfflinePCMobVersionToPCMobVersion(MobVersion mob_version)
{
	switch (mob_version) {
	case MobVersion::OfflineTitanium:
		return MobVersion::Titanium;
	case MobVersion::OfflineSoF:
		return MobVersion::SoF;
	case MobVersion::OfflineSoD:
		return MobVersion::SoD;
	case MobVersion::OfflineUF:
		return MobVersion::UF;
	case MobVersion::OfflineRoF:
		return MobVersion::RoF;
	case MobVersion::OfflineRoF2:
		return MobVersion::RoF2;
	default:
		return MobVersion::Unknown;
	}
}

EQEmu::versions::ClientVersion EQEmu::versions::ConvertOfflinePCMobVersionToClientVersion(MobVersion mob_version)
{
	switch (mob_version) {
	case MobVersion::OfflineTitanium:
		return ClientVersion::Titanium;
	case MobVersion::OfflineSoF:
		return ClientVersion::SoF;
	case MobVersion::OfflineSoD:
		return ClientVersion::SoD;
	case MobVersion::OfflineUF:
		return ClientVersion::UF;
	case MobVersion::OfflineRoF:
		return ClientVersion::RoF;
	case MobVersion::OfflineRoF2:
		return ClientVersion::RoF2;
	default:
		return ClientVersion::Unknown;
	}
}

EQEmu::versions::MobVersion EQEmu::versions::ConvertClientVersionToOfflinePCMobVersion(ClientVersion client_version)
{
	switch (client_version) {
	case ClientVersion::Titanium:
		return MobVersion::OfflineTitanium;
	case ClientVersion::SoF:
		return MobVersion::OfflineSoF;
	case ClientVersion::SoD:
		return MobVersion::OfflineSoD;
	case ClientVersion::UF:
		return MobVersion::OfflineUF;
	case ClientVersion::RoF:
		return MobVersion::OfflineRoF;
	case ClientVersion::RoF2:
		return MobVersion::OfflineRoF2;
	default:
		return MobVersion::Unknown;
	}
}
