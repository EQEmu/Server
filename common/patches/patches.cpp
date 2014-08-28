
#include "../debug.h"
#include "patches.h"

#include "client62.h"
#include "titanium.h"
#include "underfoot.h"
#include "sof.h"
#include "sod.h"
#include "rof.h"
//#include "rof2.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	Client62::Register(into);
	Titanium::Register(into);
	SoF::Register(into);
	SoD::Register(into);
	Underfoot::Register(into);
	RoF::Register(into);
	//RoF2::Register(into);
}

void ReloadAllPatches() {
	Client62::Reload();
	Titanium::Reload();
	SoF::Reload();
	SoD::Reload();
	Underfoot::Reload();
	RoF::Reload();
	//RoF2::Reload();
}
