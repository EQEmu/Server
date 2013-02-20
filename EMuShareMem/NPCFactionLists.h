#include "../common/types.h"
#include "../common/features.h"
#include "../zone/faction.h"
#include "../common/EMuShareMem.h"

// MMF_MAX_NPCFactionList_ID:  Make sure this is bigger than the highest NPCFactionList ID#
#ifndef MMF_MAX_NPCFactionList_ID
#define MMF_MAX_NPCFactionList_ID		50000
#endif

struct MMFNPCFactionLists_Struct {
	uint32			MaxNPCFactionListID;
	uint32			NextFreeIndex;
	uint32			NPCFactionListCount;
	uint32			NPCFactionListIndex[MMF_MAX_NPCFactionList_ID+1];
	NPCFactionList	NPCFactionLists[0];
};

bool	pDLLLoadNPCFactionLists(CALLBACK_DBLoadNPCFactionLists cbDBLoadNPCFactionLists, uint32 iNPCFactionListStructSize, int32* iNPCFactionListCount, uint32* iMaxNPCFactionListID, uint8 iMaxNPCFactions);
bool	pAddNPCFactionList(uint32 id, const NPCFactionList* nfl);
bool	pSetNPCFaction(uint32 id, uint32* factionid, int32* factionvalue, int8 *factionnpcvalue, uint8 *factiontemp);
const NPCFactionList* pGetNPCFactionList(uint32 id);
