
#include "../global_define.h"
#include "patches.h"

#include "titanium.h"
#include "uf.h"
#include "sof.h"
#include "sod.h"
#include "rof.h"
#include "rof2.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	Titanium::Register(into);
	SoF::Register(into);
	SoD::Register(into);
	UF::Register(into);
	RoF::Register(into);
	RoF2::Register(into);
}

void ReloadAllPatches() {
	Titanium::Reload();
	SoF::Reload();
	SoD::Reload();
	UF::Reload();
	RoF::Reload();
	RoF2::Reload();
}
