#include "../common/debug.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include "../common/unix.h"
#endif

#include <memory.h>
#include <iostream>
using namespace std;
#include "NPCFactionLists.h"
#include "../common/timer.h"
#include "MMF.h"

MMF NPCFactionListsMMF;
const MMFNPCFactionLists_Struct* MMFNPCFactionListsData = 0;
MMFNPCFactionLists_Struct* MMFNPCFactionListsData_Writable = 0;

#ifdef _WINDOWS
extern "C" __declspec(dllexport) const NPCFactionList* GetNPCFactionList(uint32 id) {
	return pGetNPCFactionList(id);
};

extern "C" __declspec(dllexport) bool AddNPCFactionList(uint32 id, const NPCFactionList* nfl) {
	return pAddNPCFactionList(id, nfl);
};

extern "C" __declspec(dllexport) bool DLLLoadNPCFactionLists(CALLBACK_DBLoadNPCFactionLists cbDBLoadNPCFactionLists, uint32 iNPCFactionListStructSize, int32* iNPCFactionListsCount, uint32* iMaxNPCFactionListID, uint8 iMaxNPCFactions) {
	return pDLLLoadNPCFactionLists(cbDBLoadNPCFactionLists, iNPCFactionListStructSize, iNPCFactionListsCount, iMaxNPCFactionListID, iMaxNPCFactions);
};

extern "C" __declspec(dllexport) bool SetNPCFaction(uint32 id, uint32* factionid, int32* factionvalue, int8 *factionnpcvalue, uint8 *factiontemp) {
	return pSetNPCFaction(id, factionid, factionvalue, factionnpcvalue, factiontemp);
}
#else
extern "C" const NPCFactionList* GetNPCFactionList(uint32 id) {
	return pGetNPCFactionList(id);
};

extern "C" bool AddNPCFactionList(uint32 id, const NPCFactionList* nfl) {
	return pAddNPCFactionList(id, nfl);
};

extern "C" bool DLLLoadNPCFactionLists(CALLBACK_DBLoadNPCFactionLists cbDBLoadNPCFactionLists, uint32 iNPCFactionListStructSize, int32* iNPCFactionListsCount, uint32* iMaxNPCFactionListID, uint8 iMaxNPCFactions) {
	return pDLLLoadNPCFactionLists(cbDBLoadNPCFactionLists, iNPCFactionListStructSize, iNPCFactionListsCount, iMaxNPCFactionListID, iMaxNPCFactions);
};

extern "C" bool SetNPCFaction(uint32 id, uint32* factionid, int32* factionvalue, int8 *factionnpcvalue, uint8 *factiontemp) {
	return pSetNPCFaction(id, factionid, factionvalue, factionnpcvalue, factiontemp);
}
#endif

bool pAddNPCFactionList(uint32 id, const NPCFactionList* nfl) {
	if (!MMFNPCFactionListsData_Writable){
	    if (EQDEBUG>=1) cout<<"[Debug] !MMFNPCFactionListsData_Writable"<<endl;
		return false;
	}
	if (id > MMF_MAX_NPCFactionList_ID || MMFNPCFactionListsData_Writable->NextFreeIndex >= MMFNPCFactionListsData_Writable->NPCFactionListCount){
	    if (EQDEBUG>=1) cout<<"[Debug] id > MMF_MAX_NPCFactionList_ID || MMFNPCFactionListsData_Writable->NextFreeIndex >= MMFNPCFactionListsData_Writable->NPCFactionListCount"<<endl;
		return false;
	}
	if (MMFNPCFactionListsData_Writable->NPCFactionListIndex[id] != 0xFFFFFFFF){
	    if (EQDEBUG>=1) cout<<"[Debug] MMFNPCFactionListsData_Writable->NPCFactionListIndex[id] != 0xFFFFFFFF"<<endl;
		return false;
	}

	MMFNPCFactionListsData_Writable->NPCFactionListIndex[id] = MMFNPCFactionListsData_Writable->NextFreeIndex++;
	memcpy(&MMFNPCFactionListsData_Writable->NPCFactionLists[MMFNPCFactionListsData_Writable->NPCFactionListIndex[id]], nfl, sizeof(NPCFactionList));

	return true;
}

bool pSetNPCFaction(uint32 id, uint32* factionid, int32* factionvalue, int8 *factionnpcvalue, uint8 *factiontemp) {
	if (!MMFNPCFactionListsData_Writable) {
	    if(EQDEBUG>=1) cout<<"[Debug] !MMFNPCFactionListsData_Writable"<<endl;
		return false;
	}
	if (id > MMF_MAX_NPCFactionList_ID) {
	    if(EQDEBUG>=1) cout<<"[Debug] id > MMF_MAX_NPCFactionList_ID"<<endl;
		return false;
	}
	if (MMFNPCFactionListsData_Writable->NPCFactionListIndex[id] == 0xFFFFFFFF) {
	    if(EQDEBUG>=1) cout<<"[Debug] MMFNPCFactionListsData_Writable->NPCFactionListIndex[id="<<id<<"] == 0xFFFFFFFF"<<endl;
		return false;
	}
	
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		MMFNPCFactionListsData_Writable->NPCFactionLists[MMFNPCFactionListsData_Writable->NPCFactionListIndex[id]].factionid[i] = factionid[i];
		MMFNPCFactionListsData_Writable->NPCFactionLists[MMFNPCFactionListsData_Writable->NPCFactionListIndex[id]].factionvalue[i] = factionvalue[i];
		MMFNPCFactionListsData_Writable->NPCFactionLists[MMFNPCFactionListsData_Writable->NPCFactionListIndex[id]].factionnpcvalue[i] = factionnpcvalue[i];
		MMFNPCFactionListsData_Writable->NPCFactionLists[MMFNPCFactionListsData_Writable->NPCFactionListIndex[id]].factiontemp[i] = factiontemp[i];
	}
	return true;
}

bool pDLLLoadNPCFactionLists(CALLBACK_DBLoadNPCFactionLists cbDBLoadNPCFactionLists, uint32 iNPCFactionListStructSize, int32* iNPCFactionListsCount, uint32* iMaxNPCFactionListID, uint8 iMaxNPCFactions) {
	if (iNPCFactionListStructSize != sizeof(NPCFactionList)) {
		cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: iNPCFactionListStructSize != sizeof(NPCFactionList)" << endl;
		cout << "NPCFactionList struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (iMaxNPCFactions != MAX_NPC_FACTIONS) {
		cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: iMaxNPCFactions != MAX_NPC_FACTIONS" << endl;
		cout << "NPCFactionList struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (*iMaxNPCFactionListID > MMF_MAX_NPCFactionList_ID) {
		cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: iMaxNPCFactions > MMF_MAX_NPCFactionList_ID" << endl;
		cout << "You need to increase the define in NPCFactionList.h." << endl;
		return false;
	}
	uint32 tmpMemSize = sizeof(MMFNPCFactionLists_Struct) + 256 + (sizeof(NPCFactionList) * (*iNPCFactionListsCount));
	if (NPCFactionListsMMF.Open("EQEMuFactionLists", tmpMemSize)) {
//		MMFNPCFactionListsData = (const MMFNPCFactionLists_Struct*) NPCFactionListsMMF.GetHandle();
		if (NPCFactionListsMMF.CanWrite()) {
			MMFNPCFactionListsData_Writable = (MMFNPCFactionLists_Struct*) NPCFactionListsMMF.GetWriteableHandle();
			if (!MMFNPCFactionListsData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: !MMFNPCFactionListsData_Writable" << endl;
				return false;
			}

			memset(MMFNPCFactionListsData_Writable, 0, tmpMemSize);
			for(int i=0; i<MMF_MAX_NPCFactionList_ID; i++)
				MMFNPCFactionListsData_Writable->NPCFactionListIndex[i] = 0xFFFFFFFF;
			MMFNPCFactionListsData_Writable->MaxNPCFactionListID = *iMaxNPCFactionListID;
			MMFNPCFactionListsData_Writable->NPCFactionListCount = *iNPCFactionListsCount;
			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (!cbDBLoadNPCFactionLists(MMFNPCFactionListsData_Writable->NPCFactionListCount, MMFNPCFactionListsData_Writable->MaxNPCFactionListID)) {
				cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: !cbDBLoadNPCFactionLists" << endl;
				return false;
			}

			MMFNPCFactionListsData_Writable = 0;
			NPCFactionListsMMF.SetLoaded();
			MMFNPCFactionListsData = (const MMFNPCFactionLists_Struct*) NPCFactionListsMMF.GetHandle();
			if (!MMFNPCFactionListsData) {
				cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: !MMFNPCFactionListsData (CanWrite=true)" << endl;
				return false;
			}
			return true;
		}
		else {
			if (!NPCFactionListsMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!NPCFactionListsMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(100);
					Timer::SetCurrentTime();
				}
				if (!NPCFactionListsMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: !NPCFactionListsMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
			MMFNPCFactionListsData = (const MMFNPCFactionLists_Struct*) NPCFactionListsMMF.GetHandle();
			if (!MMFNPCFactionListsData) {
				cout << "Error: EMuShareMem: DLLLoadNPCFactionLists: !MMFNPCFactionListsData (CanWrite=false)" << endl;
				return false;
			}
			*iMaxNPCFactionListID = MMFNPCFactionListsData->MaxNPCFactionListID;
			*iNPCFactionListsCount = MMFNPCFactionListsData->NPCFactionListCount;
			return true;
		}
	}
	else {
		cout << "Error Loading NPCFactionLists: NPCFactionLists.cpp: pDLLLoadNPCFactionLists: Open() == false" << endl;
		return false;
	}
	return false;
};

const NPCFactionList* pGetNPCFactionList(uint32 id) {
	if (MMFNPCFactionListsData == 0 || (!NPCFactionListsMMF.IsLoaded()) || id > MMF_MAX_NPCFactionList_ID || MMFNPCFactionListsData->NPCFactionListIndex[id] == 0xFFFFFFFF)
		return 0;
	return &MMFNPCFactionListsData->NPCFactionLists[MMFNPCFactionListsData->NPCFactionListIndex[id]];
}
