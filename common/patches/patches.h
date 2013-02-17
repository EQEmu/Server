#ifndef PATCHES_H_
#define PATCHES_H_

/*enum {
	Patch_062,
	Patch_Titanium,
	Patch_Live
};*/

class EQStreamIdentifier;

void RegisterAllPatches(EQStreamIdentifier &into);
void ReloadAllPatches();

#endif /*PATCHES_H_*/
