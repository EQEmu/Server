#include "../common/debug.h"
#include <memory.h>
#include <iostream>
using namespace std;
#include "Loot.h"
#include "../common/timer.h"
#include "MMF.h"

MMF LootMMF;
const MMFLoot_Struct* MMFLootData = 0;
MMFLoot_Struct* MMFLootData_Writable = 0;
uint32* LootTable;
uint32* LootDrop;

#ifdef _WINDOWS
	#define exportfunc extern "C" __declspec(dllexport)
#else
	#define exportfunc extern "C"
#endif

exportfunc const LootTable_Struct* GetLootTable(uint32 id) {
	return pGetLootTable(id);
};
exportfunc const LootDrop_Struct* GetLootDrop(uint32 id) {
	return pGetLootDrop(id);
};

exportfunc bool AddLootTable(uint32 id, const LootTable_Struct* lts) {
	return pAddLootTable(id, lts);
};
exportfunc bool AddLootDrop(uint32 id, const LootDrop_Struct* lds) {
	return pAddLootDrop(id, lds);
};

exportfunc bool DLLLoadLoot(CALLBACK_DBLoadLoot cbDBLoadLoot, 
					 uint32 iLootTableStructsize, uint32 iLootTableCount, uint32 iMaxLootTable,
					 uint32 iLootTableEntryStructsize, uint32 iLootTableEntryCount,
					 uint32 iLootDropStructsize, uint32 iLootDropCount, uint32 iMaxLootDrop,
					 uint32 iLootDropEntryStructsize, uint32 iLootDropEntryCount
					 ) {
	return pDLLLoadLoot(cbDBLoadLoot, 
					 iLootTableStructsize, iLootTableCount, iMaxLootTable,
					 iLootTableEntryStructsize, iLootTableEntryCount,
					 iLootDropStructsize, iLootDropCount, iMaxLootDrop,
					 iLootDropEntryStructsize, iLootDropEntryCount);
};


bool pAddLootTable(uint32 id, const LootTable_Struct* lts) {
	if (!MMFLootData_Writable)
		return false;
	if (id > MMFLootData_Writable->MaxLootTableID)
		return false;
	if (!LootTable || LootTable[id] != 0)
		return false;
	
	uint32 tmp = sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * lts->NumEntries);
	if (MMFLootData_Writable->dataindex + tmp >= MMFLootData_Writable->datamax)
		return false;
	LootTable[id] = MMFLootData_Writable->dataindex;
	memcpy(&MMFLootData_Writable->data[MMFLootData_Writable->dataindex], lts, tmp);
	MMFLootData_Writable->dataindex += tmp;

	return true;
}

bool pAddLootDrop(uint32 id, const LootDrop_Struct* lds) {
	if (!MMFLootData_Writable)
		return false;
	if (id > MMFLootData_Writable->MaxLootDropID)
		return false;
	if (!LootDrop || LootDrop[id] != 0)
		return false;
	
	uint32 tmp = sizeof(LootDrop_Struct) + (sizeof(LootDropEntries_Struct) * lds->NumEntries);
	if (MMFLootData_Writable->dataindex + tmp >= MMFLootData_Writable->datamax)
		return false;
	LootDrop[id] = MMFLootData_Writable->dataindex;
	memcpy(&MMFLootData_Writable->data[MMFLootData_Writable->dataindex], lds, tmp);
	MMFLootData_Writable->dataindex += tmp;

	return true;
}

bool pDLLLoadLoot(CALLBACK_DBLoadLoot cbDBLoadLoot, 
					 uint32 iLootTableStructsize, uint32 iLootTableCount, uint32 iMaxLootTable,
					 uint32 iLootTableEntryStructsize, uint32 iLootTableEntryCount,
					 uint32 iLootDropStructsize, uint32 iLootDropCount, uint32 iMaxLootDrop,
					 uint32 iLootDropEntryStructsize, uint32 iLootDropEntryCount
					 ) {
#if 0
cout << "iLootTableCount: " << iLootTableCount << endl;
cout << "iMaxLootTable: " << iMaxLootTable << endl;
cout << "iLootTableEntryCount: " << iLootTableEntryCount << endl;
cout << "iLootDropCount: " << iLootDropCount << endl;
cout << "iMaxLootDrop: " << iMaxLootDrop << endl;
cout << "iLootDropEntryCount: " << iLootDropEntryCount << endl;
#endif
	if (iLootTableStructsize != sizeof(LootTable_Struct)) {
		cout << "Error: EMuShareMem: DLLLoadLoot: iLootTableStructsize != sizeof(LootTable_Struct)" << endl;
		cout << "Item_Struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (iLootTableEntryStructsize != sizeof(LootTableEntries_Struct)) {
		cout << "Error: EMuShareMem: DLLLoadLoot: iLootTableEntryStructsize != sizeof(LootTableEntries_Struct)" << endl;
		cout << "Item_Struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (iLootDropStructsize != sizeof(LootDrop_Struct)) {
		cout << "Error: EMuShareMem: DLLLoadLoot: iLootDropStructsize != sizeof(LootDrop_Struct)" << endl;
		cout << "Item_Struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (iLootDropEntryStructsize != sizeof(LootDropEntries_Struct)) {
		cout << "Error: EMuShareMem: DLLLoadLoot: iLootDropEntryStructsize != sizeof(LootDropEntries_Struct)" << endl;
		cout << "Item_Struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}

	uint32 tmpMemSize = sizeof(MMFLoot_Struct) + 256
		+ (sizeof(uint32) * (iMaxLootTable+1))
		+ (sizeof(LootTable_Struct) * iLootTableCount) + (sizeof(LootTableEntries_Struct) * iLootTableEntryCount)
		+ (sizeof(uint32) * (iMaxLootDrop+1))
		+ (sizeof(LootDrop_Struct) * iLootDropCount) + (sizeof(LootDropEntries_Struct) * iLootDropEntryCount)
		;
	if (LootMMF.Open("EQEMuLoot", tmpMemSize)) {
		if (LootMMF.CanWrite()) {
			MMFLootData_Writable = (MMFLoot_Struct*) LootMMF.GetWriteableHandle();
			if (!MMFLootData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadLoot: !MMFLootData_Writable" << endl;
				return false;
			}

			memset(MMFLootData_Writable, 0, tmpMemSize);
			MMFLootData_Writable->LootTableCount = iLootTableCount;
			MMFLootData_Writable->MaxLootTableID = iMaxLootTable;
			MMFLootData_Writable->LootDropCount = iLootDropCount;
			MMFLootData_Writable->MaxLootDropID = iMaxLootDrop;
			MMFLootData_Writable->datamax = tmpMemSize - sizeof(MMFLoot_Struct);

			MMFLootData_Writable->dataindex = 0;
			MMFLootData_Writable->LootTableOffset = MMFLootData_Writable->dataindex;
			MMFLootData_Writable->dataindex += (sizeof(uint32) * (iMaxLootTable+1));
			MMFLootData_Writable->LootDropOffset = MMFLootData_Writable->dataindex;
			MMFLootData_Writable->dataindex += (sizeof(uint32) * (iMaxLootDrop+1));

			LootTable = (uint32*) &MMFLootData_Writable->data[MMFLootData_Writable->LootTableOffset];
			LootDrop = (uint32*) &MMFLootData_Writable->data[MMFLootData_Writable->LootDropOffset];

			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (!cbDBLoadLoot()) {
				cout << "Error: EMuShareMem: DLLLoadLoot: !cbDBLoadLoot" << endl;
				return false;
			}

			MMFLootData_Writable = 0;
			LootMMF.SetLoaded();
		}
		else {
			if (!LootMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!LootMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(10);
					Timer::SetCurrentTime();
				}
				if (!LootMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadLoot: !LootMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
		}
	}
	else {
		cout << "Error Loading Loot: Loot.cpp: pDLLLoadLoot: Open() == false" << endl;
		return false;
	}
	MMFLootData = (const MMFLoot_Struct*) LootMMF.GetHandle();
	if (!MMFLootData) {
		cout << "Error: EMuShareMem: DLLLoadLoot: !MMFLootData" << endl;
		MMFLootData = 0;
		return false;
	}
	if (MMFLootData->LootTableCount != iLootTableCount
		|| MMFLootData->MaxLootTableID != iMaxLootTable
		|| MMFLootData->LootDropCount != iLootDropCount
		|| MMFLootData->MaxLootDropID != iMaxLootDrop) {
		cout << "Error: EMuShareMem: DLLLoadLoot: Count/Max mismatch" << endl;
		MMFLootData = 0;
		return false;
	}
	LootTable = (uint32*) &MMFLootData->data[MMFLootData->LootTableOffset];
	LootDrop = (uint32*) &MMFLootData->data[MMFLootData->LootDropOffset];
	return true;
};

const LootTable_Struct* pGetLootTable(uint32 id) {
	if (MMFLootData == 0 || !LootMMF.IsLoaded())
		return 0;
	if (id > MMFLootData->MaxLootTableID || LootTable[id] == 0)
		return 0;
	return (LootTable_Struct*) &MMFLootData->data[LootTable[id]];
}

const LootDrop_Struct* pGetLootDrop(uint32 id) {
	if (MMFLootData == 0 || !LootMMF.IsLoaded())
		return 0;
	if (id > MMFLootData->MaxLootDropID || LootDrop[id] == 0)
		return 0;
	return (LootDrop_Struct*) &MMFLootData->data[LootDrop[id]];
}


