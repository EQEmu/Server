//extends the parser to include perl
//Eglin

#ifndef EMBPARSER_H
#define EMBPARSER_H

#ifdef EMBPERL

#include "client.h"
#include "parser.h"
#include "embperl.h"
#include "../common/features.h"
#include "QuestParserCollection.h"
#include "QuestInterface.h"

#include <string>
#include <map>
#include <queue>

class Seperator;

typedef enum {
	questDefault = 1,
	questDefaultByZone,
	questByName,
	questTemplate,
	questTemplateByID,
	questByID
} questMode;

typedef enum {
	itemQuestUnloaded = 1,
	itemQuestScale,
	itemQuestLore,
	itemQuestID,
	itemScriptFileID
} itemQuestMode;

typedef enum {
	pQuestLoaded = 1,
	pQuestUnloaded,
	pQuestEventCast,	// player.pl loaded, has an EVENT_CAST sub
	pQuestReadyToLoad
} playerQuestMode;

typedef enum {
	nQuestLoaded = 1,
	nQuestUnloaded,
	nQuestReadyToLoad
} GlobalNPCQuestMode;

typedef enum {
	spellQuestUnloaded = 1,
	spellQuestFullyLoaded,
	spellQuestFailed
} spellQuestMode;


struct EventRecord {
	QuestEventID event;
	uint32 objid;
	std::string data;
	NPC* npcmob;
	ItemInst* iteminst;
	Mob* mob;
	uint32 extradata;
	bool global;
};

class PerlembParser : public Parser
{
protected:

	//could prolly get rid of this map now, since I check for the
	//actual subroutine in the quest package as opposed to just seeing
	//if they do not have a quest or the default.
	std::map<uint32, questMode> hasQuests;	//npcid -> questMode
	std::map<std::string, playerQuestMode> playerQuestLoaded; //zone shortname -> playerQuestMode
	playerQuestMode globalPlayerQuestLoaded;
	GlobalNPCQuestMode globalNPCQuestLoaded;
	std::map<std::string, itemQuestMode> itemQuestLoaded;		// package name - > itemQuestMode
	std::map<uint32, spellQuestMode> spellQuestLoaded;

	std::queue<EventRecord> eventQueue;		//for events that happen when perl is in use.
	bool eventQueueProcessing;

	void HandleQueue();

	void EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, uint32 extradata, bool global = false);

	Embperl * perl;
	//export a symbol table of sorts
	virtual void map_funs();
public:
	PerlembParser(void);
	~PerlembParser();
	Embperl * getperl(void) { return perl; };
	//todo, consider making the following two methods static (need to check for perl!=null, first, then)
	bool isloaded(const char *packagename) const;

	//interface stuff
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

	virtual void ReloadQuests(bool with_timers = false);
	virtual void AddVar(std::string name, std::string val) { Parser::AddVar(name, val); };
	virtual uint32 GetIdentifier() { return 0xf8b05c11; }

	int LoadScript(int npcid, const char * zone, Mob* activater=0);
	int LoadGlobalNPCScript();
	int LoadPlayerScript(const char *zone);
	int LoadGlobalPlayerScript();
	int LoadItemScript(ItemInst* iteminst, std::string packagename, itemQuestMode Qtype);
	int LoadSpellScript(uint32 id);

	//expose a var to the script (probably parallels addvar))
	//i.e. exportvar("qst1234", "name", "somemob");
	//would expose the variable $name='somemob' to the script that handles npc1234
	void ExportHash(const char *pkgprefix, const char *hashname, std::map<std::string,std::string> &vals);
	void ExportVar(const char * pkgprefix, const char * varname, const char * value) const;
	void ExportVar(const char * pkgprefix, const char * varname, int value) const;
	void ExportVar(const char * pkgprefix, const char * varname, unsigned int value) const;
	void ExportVar(const char * pkgprefix, const char * varname, float value) const;
	//I don't escape the strings, so use caution!!
	//Same as export var, except value is not quoted, and is evaluated as perl
	void ExportVarComplex(const char * pkgprefix, const char * varname, const char * value) const;

	//get an appropriate namespage/packagename from an npcid
	std::string GetPkgPrefix(uint32 npcid, bool defaultOK = true);
	//call the appropriate perl handler. afterwards, parse and dispatch the command queue
	//SendCommands("qst1234", "EVENT_SAY") would trigger sub EVENT_SAY() from the qst1234.pl file
	virtual void SendCommands(const char * pkgprefix, const char *event, uint32 npcid, Mob* other, Mob* mob, ItemInst* iteminst);

	int	HasQuestFile(uint32 npcid);

#ifdef EMBPERL_COMMANDS
	void ExecCommand(Client *c, Seperator *sep);
#endif

};

#endif //EMBPERL

#endif //EMBPARSER_H
