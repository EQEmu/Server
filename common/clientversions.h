#ifndef CLIENTVERSIONS_H
#define CLIENTVERSIONS_H

#include "types.h"

static const uint32 BIT_Client62	= 1;
static const uint32 BIT_Titanium	= 2;
static const uint32 BIT_SoF			= 4;
static const uint32 BIT_SoD			= 8;
static const uint32 BIT_UF			= 16;
static const uint32 BIT_RoF			= 32;
static const uint32 BIT_RoF2		= 64;

static const uint32 BIT_TitaniumAndEarlier	= 0x00000003;
static const uint32 BIT_SoFAndLater			= 0xFFFFFFFC;
static const uint32 BIT_SoDAndLater			= 0xFFFFFFF8;
static const uint32 BIT_UFAndLater			= 0xFFFFFFF0;
static const uint32 BIT_RoFAndLater			= 0xFFFFFFE0;
static const uint32 BIT_RoF2AndLater		= 0xFFFFFFC0;
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
		return "ClientVersion::Unknown";
	case ClientVersion::Client62:
		return "ClientVersion::Client62";
	case ClientVersion::Titanium:
		return "ClientVersion::Titanium";
	case ClientVersion::SoF:
		return "ClientVersion::SoF";
	case ClientVersion::SoD:
		return "ClientVersion::SoD";
	case ClientVersion::UF:
		return "ClientVersion::UF";
	case ClientVersion::RoF:
		return "ClientVersion::RoF";
	case ClientVersion::RoF2:
		return "ClientVersion::RoF2";
	case ClientVersion::MobNPC:
		return "ClientVersion::MobNPC";
	case ClientVersion::MobMerc:
		return "ClientVersion::MobMerc";
	case ClientVersion::MobBot:
		return "ClientVersion::MobBot";
	case ClientVersion::MobPet:
		return "ClientVersion::MobPet";
	default:
		return "<ERROR> Invalid ClientVersion";
	};
}

#endif /* CLIENTVERSIONS_H */
