#ifndef EQEMU_PLATFORM_H
#define EQEMU_PLATFORM_H

enum EQEmuExePlatform
{
	ExePlatformNone = 0,
	ExePlatformZone,
	ExePlatformWorld,
	ExePlatformLogin,
	ExePlatformQueryServ,
	ExePlatformUCS,
	ExePlatformLaunch,
	ExePlatformSharedMemory
};

void RegisterExecutablePlatform(EQEmuExePlatform p);
const EQEmuExePlatform& GetExecutablePlatform();

#endif
