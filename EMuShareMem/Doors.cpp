#include "../common/debug.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include "../common/unix.h"
#endif

#include <memory.h>
#include <iostream>
using namespace std;
#include "Doors.h"
#include "../common/timer.h"
#include "MMF.h"

MMF DoorsMMF;
const MMFDoors_Struct* MMFDoorsData = 0;
MMFDoors_Struct* MMFDoorsData_Writable = 0;

#ifdef _WINDOWS
extern "C" __declspec(dllexport) const Door* GetDoor(uint32 id) {
	return pGetDoor(id);
};

extern "C" __declspec(dllexport) bool AddDoor(uint32 id, const Door* door) {
	return pAddDoor(id, door);
};

extern "C" __declspec(dllexport) bool DLLLoadDoors(CALLBACK_DBLoadDoors cbDBLoadDoors, uint32 iDoorstructSize, int32* iDoorsCount, uint32* iMaxDoorID) {
	return pDLLLoadDoors(cbDBLoadDoors, iDoorstructSize, iDoorsCount, iMaxDoorID);
};
#else
extern "C" const Door* GetDoor(uint32 id) {
	return pGetDoor(id);
};

extern "C" bool AddDoor(uint32 id, const Door* door) {
	return pAddDoor(id, door);
};

extern "C" bool DLLLoadDoors(CALLBACK_DBLoadDoors cbDBLoadDoors, uint32 iDoorstructSize, int32* iDoorsCount, uint32* iMaxDoorID) {
	return pDLLLoadDoors(cbDBLoadDoors, iDoorstructSize, iDoorsCount, iMaxDoorID);
};

#endif

bool pAddDoor(uint32 id, const Door* door) {
	if (!MMFDoorsData_Writable)
		return false;
	if (id > MMF_MAX_Door_ID || MMFDoorsData_Writable->NextFreeIndex >= MMFDoorsData_Writable->DoorCount)
		return false;
	if (MMFDoorsData_Writable->DoorIndex[id] != 0xFFFFFFFF)
		return false;

	MMFDoorsData_Writable->DoorIndex[id] = MMFDoorsData_Writable->NextFreeIndex++;
	memcpy(&MMFDoorsData_Writable->Doors[MMFDoorsData_Writable->DoorIndex[id]], door, sizeof(Door));

	return true;
}

bool pDLLLoadDoors(CALLBACK_DBLoadDoors cbDBLoadDoors, uint32 iDoorstructSize, int32* iDoorsCount, uint32* iMaxDoorID) {
	if (iDoorstructSize != sizeof(Door)) {
		cout << "Error: EMuShareMem: DLLLoadDoors: iDoorstructSize != sizeof(Door)" << endl;
		cout << "Door struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (*iMaxDoorID > MMF_MAX_Door_ID) {
		cout << "Error: EMuShareMem: pDLLLoadDoors: iMaxDoorID > MMF_MAX_Door_ID" << endl;
		cout << "You need to increase the define in Doors.h." << endl;
		return false;
	}
	uint32 tmpMemSize = sizeof(MMFDoors_Struct) + 256 + (sizeof(Door) * (*iDoorsCount));
	if (DoorsMMF.Open("EQEMuDoors", tmpMemSize)) {
		if (DoorsMMF.CanWrite()) {
			MMFDoorsData_Writable = (MMFDoors_Struct*) DoorsMMF.GetWriteableHandle();
			if (!MMFDoorsData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadDoors: !MMFDoorsData_Writable" << endl;
				return false;
			}

			memset(MMFDoorsData_Writable, 0, tmpMemSize);
			for(int i=0; i<MMF_MAX_Door_ID; i++)
				MMFDoorsData_Writable->DoorIndex[i] = 0xFFFFFFFF;
			MMFDoorsData_Writable->MaxDoorID = *iMaxDoorID;
			MMFDoorsData_Writable->DoorCount = *iDoorsCount;
			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (!cbDBLoadDoors(*iDoorsCount, *iMaxDoorID)) {
				cout << "Error: EMuShareMem: DLLLoadDoors: !cbDBLoadDoors" << endl;
				return false;
			}

			MMFDoorsData_Writable = 0;
			DoorsMMF.SetLoaded();
			MMFDoorsData = (const MMFDoors_Struct*) DoorsMMF.GetHandle();
			if (!MMFDoorsData) {
				cout << "Error: EMuShareMem: DLLLoadDoors: !MMFDoorsData (CanWrite=true)" << endl;
				return false;
			}
			return true;
		}
		else {
			if (!DoorsMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!DoorsMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(10);
					Timer::SetCurrentTime();
				}
				if (!DoorsMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadDoors: !DoorsMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
			MMFDoorsData = (const MMFDoors_Struct*) DoorsMMF.GetHandle();
			if (!MMFDoorsData) {
				cout << "Error: EMuShareMem: DLLLoadDoors: !MMFDoorsData (CanWrite=false)" << endl;
				return false;
			}
			*iMaxDoorID = MMFDoorsData->MaxDoorID;
			*iDoorsCount = MMFDoorsData->DoorCount;
			return true;
		}
	}
	else {
		cout << "Error Loading Doors: Doors.cpp: pDLLLoadDoors: ret == 0" << endl;
		return false;
	}
	return false;
};

const Door* pGetDoor(uint32 id) {
	if (MMFDoorsData == 0 || (!DoorsMMF.IsLoaded()) || id > MMF_MAX_Door_ID || MMFDoorsData->DoorIndex[id] == 0xFFFFFFFF)
		return 0;
	return &MMFDoorsData->Doors[MMFDoorsData->DoorIndex[id]];
}
