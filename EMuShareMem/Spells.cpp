#include "../common/debug.h"
#ifdef _WINDOWS
#include <windows.h>
#else
#include "../common/unix.h"
#endif

#include <memory.h>
#include <iostream>
using namespace std;
#include "Spells.h"
#include "../common/timer.h"
//#include "../zone/masterentity.h"
#include "MMF.h"

MMF SpellsMMF;
const MMFSpells_Struct* MMFSpellsData = 0;
MMFSpells_Struct* MMFSpellsData_Writable = 0;

#ifdef _WINDOWS
extern "C" __declspec(dllexport) bool DLLLoadSPDat(const CALLBACK_FileLoadSPDat cbFileLoadSPDat, const void** oSpellsPointer, int32* oSPDAT_RECORDS, uint32 iSPDat_Struct_Size) {
	return pDLLLoadSPDat(cbFileLoadSPDat, oSpellsPointer, oSPDAT_RECORDS, iSPDat_Struct_Size);
};
#else
extern "C" bool DLLLoadSPDat(const CALLBACK_FileLoadSPDat cbFileLoadSPDat, const void** oSpellsPointer, int32* oSPDAT_RECORDS, uint32 iSPDat_Struct_Size) {
	return pDLLLoadSPDat(cbFileLoadSPDat, oSpellsPointer, oSPDAT_RECORDS, iSPDat_Struct_Size);
};

#endif

bool pDLLLoadSPDat(const CALLBACK_FileLoadSPDat cbFileLoadSPDat, const void** oSpellsPointer, int32* oSPDAT_RECORDS, uint32 iSPDat_Struct_Size) {
	if (iSPDat_Struct_Size != sizeof(SPDat_Spell_Struct)) {
		cout << "Error: EMuShareMem: DLLLoadSPDat: iSPDat_Struct_Size != sizeof(SPDat_Spell_Struct)" << endl;
		cout << "SPDat_Spell_Struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}

	uint32 tmpMemSize = sizeof(MMFSpells_Struct) + 256 + (sizeof(SPDat_Spell_Struct) * (*oSPDAT_RECORDS));
	if (SpellsMMF.Open("EQEMuSpells", tmpMemSize)) {
		if (SpellsMMF.CanWrite()) {
			MMFSpellsData_Writable = (MMFSpells_Struct*) SpellsMMF.GetWriteableHandle();
			if (!MMFSpellsData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadSPDat: !MMFSpellsData_Writable" << endl;
				return false;
			}

			memset(MMFSpellsData_Writable, 0, tmpMemSize);
			MMFSpellsData_Writable->SPDAT_RECORDS = *oSPDAT_RECORDS;
			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (MMFSpellsData_Writable->SPDAT_RECORDS > 0) {
				cbFileLoadSPDat(&MMFSpellsData_Writable->spells[0], MMFSpellsData_Writable->SPDAT_RECORDS-1);
				*oSpellsPointer = &MMFSpellsData_Writable->spells[0];
			}
			else
				*oSpellsPointer = 0;

			MMFSpellsData_Writable = 0;
			SpellsMMF.SetLoaded();
			MMFSpellsData = (const MMFSpells_Struct*) SpellsMMF.GetHandle();
			if (!MMFSpellsData) {
				cout << "Error: EMuShareMem: DLLLoadSPDat: !MMFSpellsData (CanWrite=true)" << endl;
				return false;
			}
			return true;
		}
		else {
			if (!SpellsMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!SpellsMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(100);
					Timer::SetCurrentTime();
				}
				if (!SpellsMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadSPDat: !SpellsMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
			MMFSpellsData = (const MMFSpells_Struct*) SpellsMMF.GetHandle();
			if (!MMFSpellsData) {
				cout << "Error: EMuShareMem: DLLLoadSPDat: !SpellsMMF (CanWrite=false)" << endl;
				return false;
			}
			*oSPDAT_RECORDS = MMFSpellsData->SPDAT_RECORDS;
			if (MMFSpellsData->SPDAT_RECORDS > 0)
				*oSpellsPointer = &MMFSpellsData->spells[0];
			else
				*oSpellsPointer = 0;
			return true;
		}
	}
	else {
		cout << "Error Loading SPDat: Spells.cpp: pDLLLoadSPDat: Open() == false" << endl;
		return false;
	}
	return false;
}
