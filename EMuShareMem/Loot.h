#include "../common/types.h"
#include "../common/eq_packet_structs.h"
#include "../common/EMuShareMem.h"

#pragma pack(1)
struct MMFLoot_Struct {
	bool				Loaded;
	uint32				MaxLootTableID;
	uint32				LootTableCount;
	uint32				LootTableOffset;
	uint32				MaxLootDropID;
	uint32				LootDropCount;
	uint32				LootDropOffset;
	uint32				datamax;
	uint32				dataindex;
	uint8				data[0];
};
#pragma pack()

bool	pDLLLoadLoot(CALLBACK_DBLoadLoot cbDBLoadLoot, 
					 uint32 iLootTableStructsize, uint32 iLootTableCount, uint32 iMaxLootTable,
					 uint32 iLootTableEntryStructsize, uint32 iLootTableEntryCount,
					 uint32 iLootDropStructsize, uint32 iLootDropCount, uint32 iMaxLootDrop,
					 uint32 iLootDropEntryStructsize, uint32 iLootDropEntryCount
					 );
bool	pAddLootTable(uint32 id, const LootTable_Struct* lts);
bool	pAddLootDrop(uint32, const LootDrop_Struct* lds);
const LootTable_Struct*		pGetLootTable(uint32 id);
const LootDrop_Struct*		pGetLootDrop(uint32 id);
