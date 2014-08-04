#ifndef CLIENTVERSIONS_H
#define CLIENTVERSIONS_H

static const uint32 BIT_Client62	= 1;
static const uint32 BIT_Titanium	= 2;
static const uint32 BIT_SoF			= 4;
static const uint32 BIT_SoD			= 8;
static const uint32 BIT_Underfoot	= 16;
static const uint32 BIT_RoF			= 32;
static const uint32 BIT_RoF2		= 64;

static const uint32 BIT_TitaniumAndEarlier	= 0x00000003;
static const uint32 BIT_SoFAndLater			= 0xFFFFFFFC;
static const uint32 BIT_SoDAndLater			= 0xFFFFFFF8;
static const uint32 BIT_UnderfootAndLater	= 0xFFFFFFF0;
static const uint32 BIT_RoFAndLater			= 0xFFFFFFE0;
static const uint32 BIT_RoF2AndLater		= 0xFFFFFFC0;
static const uint32 BIT_AllClients			= 0xFFFFFFFF;

typedef enum {
	EQClientUnknown = 0,
	EQClient62,				// Build: 'Aug  4 2005 15:40:59'
	EQClientTitanium,		// Build: 'Oct 31 2005 10:33:37'
	EQClientSoF,			// Build: 'Sep  7 2007 09:11:49'
	EQClientSoD,			// Build: 'Dec 19 2008 15:22:49'
	EQClientUnderfoot,		// Build: 'Jun  8 2010 16:44:32'
	EQClientRoF,			// Build: 'Dec 10 2012 17:35:44'
	EQClientRoF2,			// Build: 'May 10 2013 23:30:08'
	
	_EQClientCount,			// place new clients before this point (preferably, in release/attribute order)
	
	// Values below are not implemented, as yet...
	
	EmuNPC = _EQClientCount,
	EmuMerc,
	EmuBot,
	EmuPet,
	
	_EmuClientCount			// array size for EQLimits
} EQClientVersion;

#endif /* CLIENTVERSIONS_H */
