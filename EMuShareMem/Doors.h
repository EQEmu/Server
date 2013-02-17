#include "../common/types.h"
#include "../zone/zonedump.h"
#include "../common/EMuShareMem.h"

// MMF_MAX_Door_ID:  Make sure this is bigger than the highest Door ID#
#ifndef MMF_MAX_Door_ID
#define MMF_MAX_Door_ID		30000
#endif
// MMF_MAX_Door_MEM: Maxium number of Doors to load into memory. Make sure this is bigger
//                      than the total number of Doors in the server's database!

struct MMFDoors_Struct {
	uint32		MaxDoorID;
	uint32		NextFreeIndex;
	uint32		DoorCount;
	uint32		DoorIndex[MMF_MAX_Door_ID+1];
	Door		Doors[0];
};

bool	pDLLLoadDoors(CALLBACK_DBLoadDoors cbDBLoadDoors, uint32 iDoorstructSize, int32* iDoorsCount, uint32* iMaxDoorID);
bool	pAddDoor(uint32 id, const Door* door);
const Door* pGetDoor(uint32 id);
