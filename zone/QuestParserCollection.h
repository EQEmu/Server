#ifndef _EQE_QUESTPARSERCOLLECTION_H
#define _EQE_QUESTPARSERCOLLECTION_H

#include "../common/types.h"
#include <string.h>
#include <string>
#include <list>
#include <map>

#include "masterentity.h"
#include "../common/Item.h"
#include "QuestInterface.h"

#define QuestFailedToLoad 0xFFFFFFFF
#define QuestUnloaded 0x00

class QuestParserCollection {
public:
	QuestParserCollection();
	~QuestParserCollection();

	void RegisterQuestInterface(QuestInterface *qi, std::string ext);

	void AddVar(std::string name, std::string val);
	void ReloadQuests(bool reset_timers = true);

	bool HasQuestSub(uint32 npcid, const char *subname);
	bool PlayerHasQuestSub(const char *subname);
	bool SpellHasQuestSub(uint32 spell_id, const char *subname);
	bool ItemHasQuestSub(ItemInst *itm, const char *subname);

	void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
	void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
	void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data);
	void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data);

private:
	QuestInterface *GetQIByNPCQuest(uint32 npcid);
	QuestInterface *GetQIByGlobalNPCQuest();
	QuestInterface *GetQIByPlayerQuest();
	QuestInterface *GetQIByGlobalPlayerQuest();
	QuestInterface *GetQIBySpellQuest(uint32 spell_id);
	QuestInterface *GetQIByItemQuest(std::string item_script);

	std::map<uint32, QuestInterface*> _interfaces;
	std::map<uint32, std::string> _extensions;
	std::list<QuestInterface*> _load_precedence;

	//0x00 = Unloaded
	//0xFFFFFFFF = Failed to Load
	std::map<uint32, uint32> _npc_quest_status;
	uint32 _global_npc_quest_status;
	uint32 _player_quest_status;
	uint32 _global_player_quest_status;
	std::map<uint32, uint32> _spell_quest_status;
	std::map<std::string, uint32> _item_quest_status;
};

extern QuestParserCollection *parse;

#endif

