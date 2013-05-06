#ifndef _EQE_LUA_PARSER_H
#define _EQE_LUA_PARSER_H
#ifdef LUA_EQEMU

#include <lua.hpp>

#include "QuestParserCollection.h"
#include "QuestInterface.h"

class ItemInst;
class Client;
class NPC;

class LuaParser {
public:
    virtual void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
	virtual void EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
    virtual void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
    virtual void EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
    virtual void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data);
    virtual void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data);

	virtual bool HasQuestSub(uint32 npcid, const char *subname);
	virtual bool HasGlobalQuestSub(const char *subname);
	virtual bool PlayerHasQuestSub(const char *subname);
    virtual bool GlobalPlayerHasQuestSub(const char *subname);
	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname);
    virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname);

    virtual void AddVar(std::string name, std::string val);
    virtual void ReloadQuests(bool reset_timers = true);
    virtual uint32 GetIdentifier() { return 0xb0712acc; }
private:
	lua_State* L;
};

#endif
#endif


