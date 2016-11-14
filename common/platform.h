#ifndef EQEMU_PLATFORM_H
#define EQEMU_PLATFORM_H

enum EQEmuExePlatform
{
	ExePlatformNone = 0,
	ExePlatformZone,
	ExePlatformWorld,
	ExePlatformLogin,
	ExePlatformQueryServ,
	ExePlatformSocket_Server,
	ExePlatformUCS,
	ExePlatformLaunch,
	ExePlatformSharedMemory,
	ExePlatformClientImport,
	ExePlatformClientExport,
	ExePlatformHC
};

void RegisterExecutablePlatform(EQEmuExePlatform p);
const EQEmuExePlatform& GetExecutablePlatform();
int GetExecutablePlatformInt();

#endif
