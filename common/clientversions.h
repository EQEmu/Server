/*
EQEMu:  Everquest Server Emulator

Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#ifndef CLIENTVERSIONS_H
#define CLIENTVERSIONS_H

#include "types.h"

static const uint32 BIT_Client62			= 0x00000001; // 1 (unsupported - placeholder for scripts)

static const uint32 BIT_Titanium			= 0x00000002; // 2
static const uint32 BIT_SoF					= 0x00000004; // 4
static const uint32 BIT_SoD					= 0x00000008; // 8
static const uint32 BIT_UF					= 0x00000010; // 16
static const uint32 BIT_RoF					= 0x00000020; // 32
static const uint32 BIT_RoF2				= 0x00000040; // 64

static const uint32 BIT_TitaniumAndEarlier	= 0x00000003; // 3
static const uint32 BIT_SoFAndEarlier		= 0x00000007; // 7
static const uint32 BIT_SoDAndEarlier		= 0x0000000F; // 15
static const uint32 BIT_UFAndEarlier		= 0x0000001F; // 31
static const uint32 BIT_RoFAndEarlier		= 0x0000003F; // 63

static const uint32 BIT_SoFAndLater			= 0xFFFFFFFC; // 4294967292
static const uint32 BIT_SoDAndLater			= 0xFFFFFFF8; // 4294967288
static const uint32 BIT_UFAndLater			= 0xFFFFFFF0; // 4294967280
static const uint32 BIT_RoFAndLater			= 0xFFFFFFE0; // 4294967264
static const uint32 BIT_RoF2AndLater		= 0xFFFFFFC0; // 4294967232

static const uint32 BIT_AllClients			= 0xFFFFFFFF;

enum class ClientVersion
{
	Unknown = 0,
	Client62,	// Build: 'Aug  4 2005 15:40:59'
	Titanium,	// Build: 'Oct 31 2005 10:33:37'
	SoF,		// Build: 'Sep  7 2007 09:11:49'
	SoD,		// Build: 'Dec 19 2008 15:22:49'
	UF,			// Build: 'Jun  8 2010 16:44:32'
	RoF,		// Build: 'Dec 10 2012 17:35:44'
	RoF2,		// Build: 'May 10 2013 23:30:08'

	MobNPC,
	MobMerc,
	MobBot,
	MobPet,
};

#define CLIENT_VERSION_COUNT 12
#define LAST_PC_CLIENT ClientVersion::RoF2
#define LAST_NPC_CLIENT ClientVersion::MobPet


static const char* ClientVersionName(ClientVersion version)
{
	switch (version)
	{
	case ClientVersion::Unknown:
		return "Unknown";
	case ClientVersion::Client62:
		return "Client62";
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
	case ClientVersion::MobNPC:
		return "MobNPC";
	case ClientVersion::MobMerc:
		return "MobMerc";
	case ClientVersion::MobBot:
		return "MobBot";
	case ClientVersion::MobPet:
		return "MobPet";
	default:
		return "<ERROR> Invalid ClientVersion";
	};
}

static uint32 ClientBitFromVersion(ClientVersion clientVersion)
{
	switch (clientVersion)
	{
	case ClientVersion::Unknown:
	case ClientVersion::Client62:
		return 0;
	case ClientVersion::Titanium:
	case ClientVersion::SoF:
	case ClientVersion::SoD:
	case ClientVersion::UF:
	case ClientVersion::RoF:
	case ClientVersion::RoF2:
	case ClientVersion::MobNPC:
	case ClientVersion::MobMerc:
	case ClientVersion::MobBot:
	case ClientVersion::MobPet:
		return ((uint32)1 << (static_cast<unsigned int>(clientVersion) - 1));
	default:
		return 0;
	}
}

static ClientVersion ClientVersionFromBit(uint32 clientVersionBit)
{
	switch (clientVersionBit)
	{
	case (uint32)static_cast<unsigned int>(ClientVersion::Unknown):
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::Client62) - 1)):
		return ClientVersion::Unknown;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::Titanium) - 1)):
		return ClientVersion::Titanium;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::SoF) - 1)):
		return ClientVersion::SoF;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::SoD) - 1)):
		return ClientVersion::SoD;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::UF) - 1)):
		return ClientVersion::UF;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::RoF) - 1)):
		return ClientVersion::RoF;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::RoF2) - 1)):
		return ClientVersion::RoF2;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::MobNPC) - 1)):
		return ClientVersion::MobNPC;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::MobMerc) - 1)):
		return ClientVersion::MobMerc;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::MobBot) - 1)):
		return ClientVersion::MobBot;
	case ((uint32)1 << (static_cast<unsigned int>(ClientVersion::MobPet) - 1)):
		return ClientVersion::MobPet;
	default:
		return ClientVersion::Unknown;
	}
}

static uint32 ExpansionFromClientVersion(ClientVersion clientVersion)
{
	switch(clientVersion)
	{
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

#endif /* CLIENTVERSIONS_H */
