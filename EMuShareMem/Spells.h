#ifndef MEMSHARE_SPELLS_H
#define MEMSHARE_SPELLS_H

#include "../common/types.h"
#include "../zone/spdat.h"
#include "../common/EMuShareMem.h"

struct MMFSpells_Struct {
	uint32		SPDAT_RECORDS;	// maxspellid + 1, size of array
	SPDat_Spell_Struct spells[0];
};

bool	pDLLLoadSPDat(const CALLBACK_FileLoadSPDat cbFileLoadSPDat, const void** oSpellsPointer, int32* oSPDAT_RECORDS, uint32 iSPDat_Struct_Size);

#endif

