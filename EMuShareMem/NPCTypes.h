#include "../common/types.h"
#include "../zone/zonedump.h"
#include "../common/EMuShareMem.h"

// MMF_MAX_NPCTYPE_ID:  Make sure this is bigger than the highest NPCType ID#
#define MMF_MAX_NPCTYPE_ID		400000

struct MMFNPCTypes_Struct {
	uint32		MaxNPCTypeID;
	uint32		NextFreeIndex;
	uint32		NPCTypeCount;
	uint32		NPCTypeIndex[MMF_MAX_NPCTYPE_ID+1];
	NPCType		NPCTypes[0];
};

//bool	pDLLLoadNPCTypes(CALLBACK_DBLoadNPCTypes cbDBLoadNPCTypes, uint32 iNPCTypeStructSize, int32* iNPCTypesCount, uint32* iMaxNPCTypeID);
bool	pAddNPCType(uint32 id, const NPCType* npctype);
const NPCType* pGetNPCType(uint32 id);
