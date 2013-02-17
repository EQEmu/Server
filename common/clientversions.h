#ifndef CLIENTVERSIONS_H
#define CLIENTVERSIONS_H

static const uint32 BIT_Client62 = 1;
static const uint32 BIT_Titanium = 2;
static const uint32 BIT_SoF = 4;
static const uint32 BIT_SoD = 8;
static const uint32 BIT_Underfoot = 16;
static const uint32 BIT_RoF = 32;
static const uint32 BIT_TitaniumAndEarlier = 3;
static const uint32 BIT_SoFAndLater = 0xFFFFFFFC;
static const uint32 BIT_SoDAndLater = 0xFFFFFFF8;
static const uint32 BIT_UnderfootAndLater = 0xFFFFFFF0;
static const uint32 BIT_RoFAndLater = 0xFFFFFFE0;
static const uint32 BIT_AllClients = 0xFFFFFFFF;

typedef enum {
	EQClientUnknown = 0,
	EQClient62,
	EQClientTitanium,
	EQClientSoF,
	EQClientSoD,
	EQClientUnderfoot,
	EQClientRoF
} EQClientVersion;

#endif /* CLIENTVERSIONS_H */
