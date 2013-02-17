/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef EMuShareMem_H
#define EMuShareMem_H
#ifdef _WINDOWS
#include <windows.h>
#else
#include "../common/unix.h"
#endif
#include "../common/eq_packet_structs.h"
#include "../zone/zonedump.h"
#include "../zone/loottable.h"
#include "SharedLibrary.h"

////////////
// Items //
///////////
typedef bool(*CALLBACK_DBLoadItems)(int32, uint32);

typedef bool(*DLLFUNC_DLLLoadItems)(const CALLBACK_DBLoadItems, uint32, int32*, uint32*);
typedef const Item_Struct*(*DLLFUNC_GetItem)(uint32);
typedef const Item_Struct*(*DLLFUNC_IterateItems)(uint32*);
typedef bool(*DLLFUNC_AddItem)(uint32, const Item_Struct*);

struct ItemsDLLFunc_Struct {
	DLLFUNC_DLLLoadItems DLLLoadItems;
	DLLFUNC_GetItem GetItem;
	DLLFUNC_IterateItems IterateItems;
	DLLFUNC_AddItem cbAddItem;
};
/*
typedef bool(*CALLBACK_DBLoadNPCTypes)(int32, uint32);

typedef bool(*DLLFUNC_DLLLoadNPCTypes)(const CALLBACK_DBLoadNPCTypes, uint32, int32*, uint32*);
typedef const NPCType*(*DLLFUNC_GetNPCType)(uint32);
typedef bool(*DLLFUNC_AddNPCType)(uint32, const NPCType*);
struct NPCTypesDLLFunc_Struct {
	DLLFUNC_DLLLoadNPCTypes DLLLoadNPCTypes;
	DLLFUNC_GetNPCType GetNPCType;
	DLLFUNC_AddNPCType cbAddNPCType;
};
*/


////////////
// Doors ///
////////////
typedef bool(*CALLBACK_DBLoadDoors)(int32, uint32);

typedef bool(*DLLFUNC_DLLLoadDoors)(const CALLBACK_DBLoadDoors, uint32, int32*, uint32*);
typedef const Door*(*DLLFUNC_GetDoor)(uint32);
typedef bool(*DLLFUNC_AddDoor)(uint32, const Door*);
struct DoorsDLLFunc_Struct {
	DLLFUNC_DLLLoadDoors DLLLoadDoors;
	DLLFUNC_GetDoor GetDoor;
	DLLFUNC_AddDoor cbAddDoor;
};

////////////
// Spells //
////////////
typedef bool(*CALLBACK_FileLoadSPDat)(void*, int32);

typedef bool(*DLLFUNC_DLLLoadSPDat)(const CALLBACK_FileLoadSPDat, const void**, int32*, uint32);
struct SpellsDLLFunc_Struct {
	DLLFUNC_DLLLoadSPDat DLLLoadSPDat;
};

//////////////
// Factions //
//////////////

typedef bool(*CALLBACK_DBLoadNPCFactionLists)(int32, uint32);

typedef bool(*DLLFUNC_DLLLoadNPCFactionLists)(const CALLBACK_DBLoadNPCFactionLists, uint32, int32*, uint32*, uint8);
typedef const NPCFactionList*(*DLLFUNC_GetNPCFactionList)(uint32);
typedef bool(*DLLFUNC_AddNPCFactionList)(uint32, const NPCFactionList*);
typedef bool(*DLLFUNC_SetFaction)(uint32, uint32*, int32*, int8*, uint8*);
struct NPCFactionListDLLFunc_Struct {
	DLLFUNC_DLLLoadNPCFactionLists DLLLoadNPCFactionLists;
	DLLFUNC_GetNPCFactionList GetNPCFactionList;
	DLLFUNC_AddNPCFactionList cbAddNPCFactionList;
	DLLFUNC_SetFaction cbSetFaction;
};

////////////
// Loot   //
///////////

typedef bool(*CALLBACK_DBLoadLoot)();

typedef bool(*DLLFUNC_DLLLoadLoot)(const CALLBACK_DBLoadLoot, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32);
typedef bool(*DLLFUNC_AddLootTable)(uint32, const LootTable_Struct*);
typedef bool(*DLLFUNC_AddLootDrop)(uint32, const LootDrop_Struct*);
typedef const LootTable_Struct*(*DLLFUNC_GetLootTable)(uint32);
typedef const LootDrop_Struct*(*DLLFUNC_GetLootDrop)(uint32);
struct LootDLLFunc_Struct {
	DLLFUNC_DLLLoadLoot DLLLoadLoot;
	DLLFUNC_AddLootTable cbAddLootTable;
	DLLFUNC_AddLootDrop cbAddLootDrop;
	DLLFUNC_GetLootTable GetLootTable;
	DLLFUNC_GetLootDrop GetLootDrop;
};

/////////////
// Opcodes //
/////////////

typedef bool(*CALLBACK_DBLoadOpcodes)(const char *filename);

typedef bool(*DLLFUNC_DLLLoadOpcodes)(const CALLBACK_DBLoadOpcodes, uint32 opsize, uint32 eq_count, uint32 emu_count, const char *filename);
typedef uint16 (*DLLFUNC_GetEQOpcode)(uint16 emu_op);
typedef uint16 (*DLLFUNC_GetEmuOpcode)(uint16 eq_op);
typedef void (*DLLFUNC_ClearEQOpcodes)();
typedef bool(*DLLFUNC_SetOpcodePair)(uint16 emu_op, uint16 eq_op);
struct OpcodeDLLFunc_Struct {
	DLLFUNC_DLLLoadOpcodes DLLLoadOpcodes;
	DLLFUNC_GetEQOpcode GetEQOpcode;
	DLLFUNC_GetEmuOpcode GetEmuOpcode;
	DLLFUNC_SetOpcodePair SetOpcodePair;
	DLLFUNC_ClearEQOpcodes ClearEQOpcodes;
};

////////////////
// Skill Caps //
////////////////

typedef bool(*CALLBACK_DBLoadSkillCaps)();

typedef bool(*DLLFUNC_DLLLoadSkillCaps)(const CALLBACK_DBLoadSkillCaps, uint32 opsize, uint8 ClassCount, uint8 SkillCount, uint8 LevelCount);
typedef uint16 (*DLLFUNC_GetSkillCap)(uint8 Class_, uint8 Skill, uint8 Level);
typedef void (*DLLFUNC_ClearSkillCaps)();
typedef bool(*DLLFUNC_SetSkillCap)(uint8 Class_, uint8 Skill, uint8 Level, uint16 cap);
typedef uint8 (*DLLFUNC_GetTrainLevel)(uint8 Class_, uint8 Skill, uint8 Level);
struct SkillCapDLLFunc_Struct {
	DLLFUNC_DLLLoadSkillCaps LoadSkillCaps;
	DLLFUNC_GetSkillCap GetSkillCap;
	DLLFUNC_SetSkillCap SetSkillCap;
	DLLFUNC_ClearSkillCaps ClearSkillCaps;
	DLLFUNC_GetTrainLevel GetTrainLevel;
};


class LoadEMuShareMemDLL : public SharedLibrary {
public:
	LoadEMuShareMemDLL();
	~LoadEMuShareMemDLL();

	bool Load();
	void Unload();
	
	ItemsDLLFunc_Struct				Items;
	//NPCTypesDLLFunc_Struct			NPCTypes;
	DoorsDLLFunc_Struct				Doors;
	SpellsDLLFunc_Struct			Spells;
	NPCFactionListDLLFunc_Struct	NPCFactionList;
	LootDLLFunc_Struct				Loot;
	OpcodeDLLFunc_Struct			Opcodes;
	SkillCapDLLFunc_Struct			SkillCaps;
private:
	void ClearFunc();
	
	bool loaded;
	
#ifdef _WINDOWS
#else
	static uint32  refCount;
	static uint32  refCountU() { return ++refCount; };
	static uint32  refCountD() { return --refCount; };
#endif
};
#endif
