/*
  Note: Do NOT change this to load items on an as-needed basis. Since this memory is
  accessed from multiple threads, you'd need mutex's all over the place if it was
  ever to be modified/updated/added to. The overhead of the mutexes would be alot more
  in the long run than the delay in loading.

  -Quagmire
*/

#ifdef _WINDOWS
#include <winsock2.h>
#include <windows.h>
#else
#include "../common/unix.h"
#endif

#include <memory.h>
#include <iostream>
using namespace std;
#include "Items.h"
#include "../common/timer.h"
#include "MMF.h"

MMF ItemsMMF;
const MMFItems_Struct* MMFItemsData = 0;
MMFItems_Struct* MMFItemsData_Writable = 0;

DLLFUNC bool AddItem(uint32 id, const Item_Struct* item) {
	if (!MMFItemsData_Writable) {
		return false;
	}
	if (id > MMF_EQMAX_ITEMS || MMFItemsData_Writable->NextFreeIndex >= MMFItemsData_Writable->ItemCount) {
		return false;
	}
	if (MMFItemsData_Writable->ItemIndex[id] != 0xFFFF) {
		return false;
	}
	
	uint32 nextid = MMFItemsData_Writable->NextFreeIndex++;
	MMFItemsData_Writable->ItemIndex[id] = nextid;
	memcpy(&MMFItemsData_Writable->Items[nextid], item, sizeof(Item_Struct));

	return true;
}

DLLFUNC bool DLLLoadItems(CALLBACK_DBLoadItems cbDBLoadItems, uint32 iItemStructSize, int32* iItemCount, uint32* iMaxItemID) {
	if (iItemStructSize != sizeof(Item_Struct)) {
		cout << "Error: EMuShareMem: DLLLoadItems: iItemStructSize != sizeof(Item_Struct)" << endl;
		cout << "Item_Struct has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	if (*iMaxItemID > MMF_EQMAX_ITEMS) {
		cout << "Error: EMuShareMem: pDLLLoadItems: iMaxItemID > MMF_EQMAX_ITEMS" << endl;
		cout << "You need to increase the define in Items.h." << endl;
		return false;
	}
	
	MMFItemsData_Writable = 0;
	//Allocate the shared memory for the item structures
	uint32 tmpMemSize = sizeof(MMFItems_Struct) + 256 + (sizeof(Item_Struct) * (*iItemCount));
	//cout << tmpMemSize << endl;
	if (ItemsMMF.Open("EQEMuItems", tmpMemSize)) {
		if (ItemsMMF.CanWrite()) {
			MMFItemsData_Writable = (MMFItems_Struct*) ItemsMMF.GetWriteableHandle();
			if (!MMFItemsData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadItems: !MMFItemsData_Writable" << endl;
				return false;
			}

			memset(MMFItemsData_Writable, 0, tmpMemSize);
			for(int i=0; i<MMF_EQMAX_ITEMS; i++)
				MMFItemsData_Writable->ItemIndex[i] = 0xFFFF;
			MMFItemsData_Writable->MaxItemID = *iMaxItemID;
			MMFItemsData_Writable->ItemCount = *iItemCount;
			//the writable handle has been created, do the load below after we have the 
			//serialization handle as well.
		} else {
			if (!ItemsMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!ItemsMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(10);
					Timer::SetCurrentTime();
				}
				if (!ItemsMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadItems: !ItemsMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
			MMFItemsData = (const MMFItems_Struct*) ItemsMMF.GetHandle();
			if (!MMFItemsData) {
				cout << "Error: EMuShareMem: DLLLoadItems: !MMFItemsData (CanWrite=false)" << endl;
				return false;
			}
			*iMaxItemID = MMFItemsData->MaxItemID;
			*iItemCount = MMFItemsData->ItemCount;

			return true;
		}
	} else {
		cout << "Error Loading Items: Items.cpp: pDLLLoadItems: Open() == false" << endl;
		return false;
	}
	/*
	
			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (!cbDBLoadItems(*iItemCount, *iMaxItemID)) {
				cout << "Error: EMuShareMem: DLLLoadItems: !cbDBLoadItems" << endl;
				return false;
			}

	*/
	
	// use a callback so the DB functions are done in the main exe
	// this way the DLL doesnt have to open a connection to mysql
	if (!cbDBLoadItems(*iItemCount, *iMaxItemID)) {
		cout << "Error: EMuShareMem: DLLLoadItems: !cbDBLoadItems" << endl;
		return false;
	}
	
	
	//Now, Disable the write handle and get the read handle.
	//do this for both item struct and serialization data
	
	MMFItemsData_Writable = 0;
	ItemsMMF.SetLoaded();
	MMFItemsData = (const MMFItems_Struct*) ItemsMMF.GetHandle();
	if (!MMFItemsData) {
		cout << "Error: EMuShareMem: DLLLoadItems: !MMFItemsData (CanWrite=true)" << endl;
		return false;
	}
	
	return true;
};

DLLFUNC const Item_Struct* GetItem(uint32 id) {
	if (MMFItemsData == 0 || (!ItemsMMF.IsLoaded()) || id > MMF_EQMAX_ITEMS || MMFItemsData->ItemIndex[id] == 0xFFFF)
		return 0;
	return &MMFItemsData->Items[MMFItemsData->ItemIndex[id]];
}

DLLFUNC const Item_Struct* IterateItems(uint32* NextIndex) {
	if (MMFItemsData == 0 || (!ItemsMMF.IsLoaded()) || (*NextIndex) > MMF_EQMAX_ITEMS)
		return 0;
	do {
		if (MMFItemsData->ItemIndex[*NextIndex] != 0xFFFF)
			return &MMFItemsData->Items[MMFItemsData->ItemIndex[(*NextIndex)++]];
	} while (++(*NextIndex) < MMF_EQMAX_ITEMS);
	
	return 0;
}
