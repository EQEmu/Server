
#include "../debug.h"
#include "patches.h"

#include "titanium.h"
#include "underfoot.h"
#include "sof.h"
#include "sod.h"
#include "rof.h"
#include "rof2.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	Titanium::Register(into);
	SoF::Register(into);
	SoD::Register(into);
	Underfoot::Register(into);
	RoF::Register(into);
	RoF2::Register(into);
}

void ReloadAllPatches() {
	Titanium::Reload();
	SoF::Reload();
	SoD::Reload();
	Underfoot::Reload();
	RoF::Reload();
	RoF2::Reload();
}
