#include "../common/types.h"
#include "../common/eq_packet_structs.h"
#include "../common/EMuShareMem.h"

// MMF_EQMAX_ITEMS:  Make sure this is bigger than the highest item ID#
#ifndef MMF_EQMAX_ITEMS
#define MMF_EQMAX_ITEMS		300000
#endif
// MMF_MEMMAX_ITEMS: Maxium number of items to load into memory. Make sure this is bigger
//                   than the total number of items in the server's database!

struct MMFItems_Struct {
	uint32		MaxItemID;
	uint32		NextFreeIndex;
	uint32		ItemCount;
	uint32		ItemIndex[MMF_EQMAX_ITEMS+1];
	Item_Struct	Items[0];
};

//#define MMF_MAX_ITEMS_MEMSIZE	sizeof(MMFItems_Struct) + 256



