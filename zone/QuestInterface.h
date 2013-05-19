#ifndef _EQE_QUESTINTERFACE_H
#define _EQE_QUESTINTERFACE_H

#include "../common/types.h"
#include "event_codes.h"

class ItemInst;
class Client;
class NPC;

class QuestInterface {
public:
	virtual void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) { }
	virtual void EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) { }
	virtual void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) { }
	virtual void EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) { }
	virtual void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) { }
	virtual void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) { }

	virtual bool HasQuestSub(uint32 npcid, const char *subname) { return false; }
	virtual bool HasGlobalQuestSub(const char *subname) { return false; }
	virtual bool PlayerHasQuestSub(const char *subname) { return false; }
	virtual bool GlobalPlayerHasQuestSub(const char *subname) { return false; }
	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname) { return false; }
	virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname) { return false; }

	virtual void AddVar(std::string name, std::string val) { }
	virtual void ReloadQuests(bool reset_timers = true) { }
	virtual uint32 GetIdentifier() { return 0; }
};

#endif

