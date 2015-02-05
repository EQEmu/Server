#include "platform.h"

EQEmuExePlatform exe_platform = ExePlatformNone;

void RegisterExecutablePlatform(EQEmuExePlatform p) {
	exe_platform = p;
}

const EQEmuExePlatform& GetExecutablePlatform() {
	return exe_platform;
}

int GetExecutablePlatformInt(){
	return exe_platform;
}