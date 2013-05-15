#ifndef _EQE_LUA_PARSER_H
#define _EQE_LUA_PARSER_H
#ifdef LUA_EQEMU

#include "QuestParserCollection.h"
#include "QuestInterface.h"
#include <string>
#include <map>

struct lua_State;
class ItemInst;
class Client;
class NPC;

class LuaParser : public QuestInterface {
public:
	LuaParser();
	~LuaParser();

	virtual int EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
	virtual int EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
	virtual int EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
	virtual int EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
	virtual int EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data);
	virtual int EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data);

	virtual bool HasQuestSub(uint32 npc_id, const char *subname);
	virtual bool HasGlobalQuestSub(const char *subname);
	virtual bool PlayerHasQuestSub(const char *subname);
	virtual bool GlobalPlayerHasQuestSub(const char *subname);
	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname);
	virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname);

	virtual void LoadNPCScript(std::string filename, int npc_id);
	virtual void LoadGlobalNPCScript(std::string filename);
	virtual void LoadPlayerScript(std::string filename);
	virtual void LoadGlobalPlayerScript(std::string filename);
	virtual void LoadItemScript(std::string filename, std::string item_script);
	virtual void LoadSpellScript(std::string filename, uint32 spell_id);

	virtual void AddVar(std::string name, std::string val);
	virtual std::string GetVar(std::string name);
	virtual void ReloadQuests();
    virtual uint32 GetIdentifier() { return 0xb0712acc; }

private:
	int _EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
	int _EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data);
	void LoadScript(std::string filename, std::string package_name);
	bool HasFunction(std::string function, std::string package_name);
	void ClearStates();
	void MapFunctions(lua_State *L);

	std::map<std::string, std::string> vars_;
	std::map<std::string, bool> loaded_;
	lua_State *L;
};

#endif
#endif
