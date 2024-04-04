#ifndef DOTNET_RUNTIME_H
#define DOTNET_RUNTIME_H

#include "dotnet_quest_interface.h"
#include "../worldserver.h"
#include "../../common/item_instance.h"

enum EventSubtype {
    Event_Npc = 0,
    Event_GlobalNpc,
    Event_Player,
    Event_GlobalPlayer,
    Event_Item,
    Event_Spell,
    Event_Encounter,
    Event_Bot,
    Event_GlobalBot
};


struct event_payload {
    int event_type;
    int quest_event_id;
    NPC *npc;
    Mob *mob;
    Client *client;
    Bot *bot;
    EQ::ItemInstance *item;
    const char *data;
    const char *encounter_name;
    uint32 extra_data;
    uint32 spell_id;
    std::vector<EQ::ItemInstance *> *item_array;
    std::vector<Mob *> *mob_array;
    std::vector<EQApplicationPacket *> *packet_array;
    std::vector<std::string> *string_array;
};

int event(event_payload event, std::vector<std::any>* extra_pointers);
int initialize(Zone* zone, EntityList* entity_list, WorldServer* worldserver, EQEmuLogSys* logsys);
int reload_quests();

#endif
