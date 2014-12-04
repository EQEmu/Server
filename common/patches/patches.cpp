
#include "../debug.h"
#include "patches.h"

#include "client62.h"
#include "titanium.h"
#include "underfoot.h"
#include "sof.h"
#include "sod.h"
#include "rof.h"
#include "rof2.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	Client62::Register(into);
	Titanium::Register(into);
	SoF::Register(into);
	SoD::Register(into);
	Underfoot::Register(into);
	RoF::Register(into);
	// Preprocessor #define ENABLE_ROF2 to enable RoF2 Client
#ifdef ENABLE_ROF2
	RoF2::Register(into);
#endif
}

void ReloadAllPatches() {
	Client62::Reload();
	Titanium::Reload();
	SoF::Reload();
	SoD::Reload();
	Underfoot::Reload();
	RoF::Reload();
	// Preprocessor #define ENABLE_ROF2 to enable RoF2 Client
#ifdef ENABLE_ROF2
	RoF2::Reload();
#endif
}
