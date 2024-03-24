#include "dotnet_quest_interface.h"
#include "../worldserver.h"

int event(QuestEventID event, NPC *npc, Mob *init, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers, bool player_event = false);
int initialize(Zone* zone, EntityList* entity_list, WorldServer* worldserver, EQEmuLogSys* logsys);
int reload_quests();