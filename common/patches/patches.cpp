#include "../debug.h"
#include "patches.h"
#include "Underfoot.h"
#include "SoF.h"
#include "SoD.h"
#include "RoF.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	SoF::Register(into);
	SoD::Register(into);
	Underfoot::Register(into);
	RoF::Register(into);
}

void ReloadAllPatches() {
	SoF::Reload();
	SoD::Reload();
	Underfoot::Reload();
	RoF::Reload();
}
