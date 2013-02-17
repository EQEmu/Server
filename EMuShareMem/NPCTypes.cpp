#include "../common/debug.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include "../common/unix.h"
#endif

#include <memory.h>
#include <iostream>
using namespace std;
#include "NPCTypes.h"
#include "../common/timer.h"
#include "MMF.h"

MMF NPCTypesMMF;
const MMFNPCTypes_Struct* MMFNPCTypesData = 0;
MMFNPCTypes_Struct* MMFNPCTypesData_Writable = 0;

#ifdef _WINDOWS
extern "C" __declspec(dllexport) const NPCType* GetNPCType(uint32 id) {
	return pGetNPCType(id);
};

extern "C" __declspec(dllexport) bool AddNPCType(uint32 id, const NPCType* npctype) {
	return pAddNPCType(id, npctype);
};

/*extern "C" __declspec(dllexport) bool DLLLoadNPCTypes(CALLBACK_DBLoadNPCTypes cbDBLoadNPCTypes, uint32 iNPCTypeStructSize, int32* iNPCTypesCount, uint32* iMaxNPCTypeID) {
	return pDLLLoadNPCTypes(cbDBLoadNPCTypes, iNPCTypeStructSize, iNPCTypesCount, iMaxNPCTypeID);
};*/

#else
extern "C" const NPCType* GetNPCType(uint32 id) {
	return pGetNPCType(id);
};

extern "C" bool AddNPCType(uint32 id, const NPCType* npctype) {
	return pAddNPCType(id, npctype);
};

extern "C" bool DLLLoadNPCTypes(CALLBACK_DBLoadNPCTypes cbDBLoadNPCTypes, uint32 iNPCTypeStructSize, int32* iNPCTypesCount, uint32* iMaxNPCTypeID) {
	return pDLLLoadNPCTypes(cbDBLoadNPCTypes, iNPCTypeStructSize, iNPCTypesCount, iMaxNPCTypeID);
};
#endif

bool pAddNPCType(uint32 id, const NPCType* npctype) {
	if (!MMFNPCTypesData_Writable)
		return false;
	if (id > MMF_MAX_NPCTYPE_ID || MMFNPCTypesData_Writable->NextFreeIndex >= MMFNPCTypesData_Writable->NPCTypeCount)
		return false;
	if (MMFNPCTypesData_Writable->NPCTypeIndex[id] != 0xFFFFFFFF)
		return false;
	
	MMFNPCTypesData_Writable->NPCTypeIndex[id] = MMFNPCTypesData_Writable->NextFreeIndex++;
	memcpy(&MMFNPCTypesData_Writable->NPCTypes[MMFNPCTypesData_Writable->NPCTypeIndex[id]], npctype, sizeof(NPCType));

	return true;
}

/*bool pDLLLoadNPCTypes(CALLBACK_DBLoadNPCTypes cbDBLoadNPCTypes, uint32 iNPCTypeStructSize, int32* iNPCTypesCount, uint32* iMaxNPCTypeID) {
	if (iNPCTypeStructSize != sizeof(NPCType)) {
		cout << "Error: EMuShareMem: DLLLoadNPCTypes: iNPCTypeStructSize != sizeof(NPCType)" << endl;
		cout << "NPCType struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (*iMaxNPCTypeID > MMF_MAX_NPCTYPE_ID) {
		cout << "Error: EMuShareMem: pDLLLoadNPCTypes: iMaxNPCTypeID > MMF_MAX_NPCTYPE_ID" << endl;
		cout << "You need to increase the define in NPCTypes.h." << endl;
		return false;
	}
	uint32 tmpMemSize = sizeof(MMFNPCTypes_Struct) + 256 + (sizeof(NPCType) * (*iNPCTypesCount));
	if (NPCTypesMMF.Open("EQEMuNPCTypes", tmpMemSize)) {
//		MMFNPCTypesData = (const MMFNPCTypes_Struct*) NPCTypesMMF.GetHandle();
		if (NPCTypesMMF.CanWrite()) {
			MMFNPCTypesData_Writable = (MMFNPCTypes_Struct*) NPCTypesMMF.GetWriteableHandle();
			if (!MMFNPCTypesData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadNPCTypes: !MMFNPCTypesData_Writable" << endl;
				return false;
			}

			memset(MMFNPCTypesData_Writable, 0, tmpMemSize);
			for(int i=0; i<MMF_MAX_NPCTYPE_ID; i++)
				MMFNPCTypesData_Writable->NPCTypeIndex[i] = 0xFFFFFFFF;
			MMFNPCTypesData_Writable->MaxNPCTypeID = *iMaxNPCTypeID;
			MMFNPCTypesData_Writable->NPCTypeCount = *iNPCTypesCount;
			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (!cbDBLoadNPCTypes(MMFNPCTypesData_Writable->NPCTypeCount, MMFNPCTypesData_Writable->MaxNPCTypeID)) {
				cout << "Error: EMuShareMem: DLLLoadNPCTypes: !cbDBLoadNPCTypes" << endl;
				return false;
			}

			MMFNPCTypesData_Writable = 0;
			NPCTypesMMF.SetLoaded();
			MMFNPCTypesData = (const MMFNPCTypes_Struct*) NPCTypesMMF.GetHandle();
			if (!MMFNPCTypesData) {
				cout << "Error: EMuShareMem: DLLLoadNPCTypes: !MMFNPCTypesData (CanWrite=true)" << endl;
				return false;
			}
			return true;
		}
		else {
			if (!NPCTypesMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!NPCTypesMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(100);
					Timer::SetCurrentTime();
				}
				if (!NPCTypesMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadNPCTypes: !NPCTypesMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
			MMFNPCTypesData = (const MMFNPCTypes_Struct*) NPCTypesMMF.GetHandle();
			if (!MMFNPCTypesData) {
				cout << "Error: EMuShareMem: DLLLoadNPCTypes: !MMFNPCTypesData (CanWrite=false)" << endl;
				return false;
			}
			*iMaxNPCTypeID = MMFNPCTypesData->MaxNPCTypeID;
			*iNPCTypesCount = MMFNPCTypesData->NPCTypeCount;
			return true;
		}
	}
	else {
		cout << "Error Loading NPCTypes: NPCTypes.cpp: pDLLLoadNPCTypes: Open() == false" << endl;
		return false;
	}
	return false;
};*/

const NPCType* pGetNPCType(uint32 id) {
	if (MMFNPCTypesData == 0 || (!NPCTypesMMF.IsLoaded()) || id > MMF_MAX_NPCTYPE_ID || MMFNPCTypesData->NPCTypeIndex[id] == 0xFFFFFFFF)
		return 0;
	return &MMFNPCTypesData->NPCTypes[MMFNPCTypesData->NPCTypeIndex[id]];
}
