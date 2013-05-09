#ifndef EQEMU_EMBPARSER_H
#define EQMEU_EMBPARSER_H
#ifdef EMBPERL


#include "client.h"
#include "../common/features.h"
#include "QuestParserCollection.h"
#include "QuestInterface.h"
#include <string>
#include <map>
#include <queue>
#include "embperl.h"

typedef enum 
{
	questUnloaded,
	questLoaded,
	questFailedToLoad
} PerlQuestStatus;

class PerlembParser : public QuestInterface {
	typedef struct {
		QuestEventID event;
		uint32 objid;
		std::string data;
		NPC* npcmob;
		ItemInst* iteminst;
		Mob* mob;
		uint32 extradata;
		bool global;
	} EventRecord;

public:
	PerlembParser();
	~PerlembParser();
	
	virtual double EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
	virtual double EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
    virtual double EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
    virtual double EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
    virtual double EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data);
    virtual double EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data);

	virtual bool HasQuestSub(uint32 npcid, const char *subname);
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
    virtual uint32 GetIdentifier() { return 0xf8b05c11; }
private:
	Embperl *perl;
	
	void ExportHash(const char *pkgprefix, const char *hashname, std::map<std::string, std::string> &vals);
	void ExportVar(const char *pkgprefix, const char *varname, const char *value) const;
	void ExportVar(const char *pkgprefix, const char *varname, int32 value) const;
	void ExportVar(const char *pkgprefix, const char *varname, uint32 value) const;
	void ExportVar(const char *pkgprefix, const char *varname, float value) const;
	void ExportVarComplex(const char *pkgprefix, const char *varname, const char *value) const;

	void EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, 
		uint32 extradata, bool global);
	void SendCommands(const char *pkgprefix, const char *event, uint32 npcid, Mob* other, Mob* mob, ItemInst* iteminst);
	void MapFunctions();

	void HandleQueue();
	void AddQueueEvent(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, 
		uint32 extradata, bool global);

	void GetQuestTypes(bool &isPlayerQuest, bool &isGlobalPlayerQuest, bool &isGlobalNPC, bool &isItemQuest, 
		bool &isSpellQuest, QuestEventID event, NPC* npcmob, ItemInst* iteminst, Mob* mob, bool global);
	void GetQuestPackageName(bool &isPlayerQuest, bool &isGlobalPlayerQuest, bool &isGlobalNPC, bool &isItemQuest, 
		bool &isSpellQuest, std::string &package_name, QuestEventID event, uint32 objid, const char * data, 
		NPC* npcmob, ItemInst* iteminst, bool global);
	void ExportCharID(const std::string &package_name, int &char_id, NPC *npcmob, Mob *mob);
	void ExportQGlobals(bool isPlayerQuest, bool isGlobalPlayerQuest, bool isGlobalNPC, bool isItemQuest, 
		bool isSpellQuest, std::string &package_name, NPC *npcmob, Mob *mob, int char_id);
	void ExportMobVariables(bool isPlayerQuest, bool isGlobalPlayerQuest, bool isGlobalNPC, bool isItemQuest, 
		bool isSpellQuest, std::string &package_name, Mob *mob, NPC *npcmob);
	void ExportZoneVariables(std::string &package_name);
	void ExportItemVariables(std::string &package_name, Mob *mob);
	void ExportEventVariables(std::string &package_name, QuestEventID event, uint32 objid, const char * data, 
		NPC* npcmob, ItemInst* iteminst, Mob* mob, uint32 extradata);
	
	std::map<uint32, PerlQuestStatus> npc_quest_status_;
	PerlQuestStatus global_npc_quest_status_;
	PerlQuestStatus player_quest_status_;
	PerlQuestStatus global_player_quest_status_;
	std::map<std::string, PerlQuestStatus> item_quest_status_;
	std::map<uint32, PerlQuestStatus> spell_quest_status_;

	bool event_queue_in_use_;
	std::queue<EventRecord> event_queue_;

	std::map<std::string, std::string> vars_;
	SV *_empty_sv;
};

#endif
#endif





////extends the parser to include perl
////Eglin
//
//#ifndef EMBPARSER_H
//#define EMBPARSER_H
//
//#ifdef EMBPERL
//
//#include "client.h"
//#include "parser.h"
//#include "embperl.h"
//#include "../common/features.h"
//#include "QuestParserCollection.h"
//#include "QuestInterface.h"
//
//#include <string>
//#include <map>
//#include <queue>
//using namespace std;
//
//class Seperator;
//
//typedef enum {
//	questDefault = 1,
//	questDefaultByZone,
//	questByName,
//	questTemplate,
//	questTemplateByID,
//	questByID
//} questMode;
//
//typedef enum {
//	itemQuestUnloaded = 1,
//	itemQuestScale,
//	itemQuestLore,
//	itemQuestID,
//	itemScriptFileID
//} itemQuestMode;
//
//typedef enum {
//	pQuestLoaded = 1,
//	pQuestUnloaded,
//	pQuestEventCast,	// player.pl loaded, has an EVENT_CAST sub
//    pQuestReadyToLoad
//} playerQuestMode;
//
//typedef enum {
//	nQuestLoaded = 1,
//	nQuestUnloaded,
//    nQuestReadyToLoad
//} GlobalNPCQuestMode;
//
//typedef enum {
//	spellQuestUnloaded = 1,
//	spellQuestFullyLoaded,
//	spellQuestFailed
//} spellQuestMode;
//
//
//struct EventRecord {
//	QuestEventID event;
//	uint32 objid;
//	string data;
//	NPC* npcmob;
//	ItemInst* iteminst;
//	Mob* mob;
//	uint32 extradata;
//	bool global;
//};
//
//class PerlembParser : public Parser
//{
//protected:
//	
//	//could prolly get rid of this map now, since I check for the
//	//actual subroutine in the quest package as opposed to just seeing
//	//if they do not have a quest or the default.
//	map<uint32, questMode> hasQuests;	//npcid -> questMode
//	map<std::string, playerQuestMode> playerQuestLoaded; //zone shortname -> playerQuestMode
//    playerQuestMode globalPlayerQuestLoaded;
//	GlobalNPCQuestMode globalNPCQuestLoaded;
//	map<std::string, itemQuestMode> itemQuestLoaded;		// package name - > itemQuestMode
//	map<uint32, spellQuestMode> spellQuestLoaded;
//
//	queue<EventRecord> eventQueue;		//for events that happen when perl is in use.
//	bool eventQueueProcessing;
//	
//	void HandleQueue();
//
//	void EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, uint32 extradata, bool global = false);
//
//	Embperl * perl;
//	//export a symbol table of sorts
//	virtual void map_funs();
//public:
//	PerlembParser(void);
//	~PerlembParser();
//	Embperl * getperl(void) { return perl; };
//	//todo, consider making the following two methods static (need to check for perl!=null, first, then)
//	bool isloaded(const char *packagename) const;
//
//    //interface stuff
//    virtual void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
//	virtual void EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data);
//    virtual void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
//    virtual void EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data);
//    virtual void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data);
//    virtual void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data);
//
//    virtual bool HasQuestSub(uint32 npcid, const char *subname);
//	virtual bool HasGlobalQuestSub(const char *subname);
//	virtual bool PlayerHasQuestSub(const char *subname);
//    virtual bool GlobalPlayerHasQuestSub(const char *subname);
//	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname);
//	virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname);
//
//    virtual void ReloadQuests(bool with_timers = false);
//    virtual void AddVar(std::string name, std::string val) { Parser::AddVar(name, val); };
//    virtual uint32 GetIdentifier() { return 0xf8b05c11; }
//
//	virtual void LoadNPCScript(std::string filename, int npc_id);
//	virtual void LoadGlobalNPCScript(std::string filename);
//	virtual void LoadPlayerScript(std::string filename);
//	virtual void LoadGlobalPlayerScript(std::string filename);
//	virtual void LoadItemScript(std::string filename, std::string item_script);
//	virtual void LoadSpellScript(std::string filename, uint32 spell_id);
//
//    //int LoadScript(int npcid, const char * zone, Mob* activater=0);
//	//int LoadGlobalNPCScript();
//	//int LoadPlayerScript(const char *zone);
//    //int LoadGlobalPlayerScript();
//	//int LoadItemScript(ItemInst* iteminst, string packagename, itemQuestMode Qtype);
//	//int LoadSpellScript(uint32 id);
//	
//	//expose a var to the script (probably parallels addvar))
//	//i.e. exportvar("qst1234", "name", "somemob"); 
//	//would expose the variable $name='somemob' to the script that handles npc1234
//	void ExportHash(const char *pkgprefix, const char *hashname, std::map<string,string> &vals);
//	void ExportVar(const char * pkgprefix, const char * varname, const char * value) const;
//	void ExportVar(const char * pkgprefix, const char * varname, int value) const;
//	void ExportVar(const char * pkgprefix, const char * varname, unsigned int value) const;
//	void ExportVar(const char * pkgprefix, const char * varname, float value) const;
//	//I don't escape the strings, so use caution!!
//	//Same as export var, except value is not quoted, and is evaluated as perl
//	void ExportVarComplex(const char * pkgprefix, const char * varname, const char * value) const;
//	
//	//get an appropriate namespage/packagename from an npcid
//	std::string GetPkgPrefix(uint32 npcid, bool defaultOK = true);
//	//call the appropriate perl handler. afterwards, parse and dispatch the command queue
//	//SendCommands("qst1234", "EVENT_SAY") would trigger sub EVENT_SAY() from the qst1234.pl file
//	virtual void SendCommands(const char * pkgprefix, const char *event, uint32 npcid, Mob* other, Mob* mob, ItemInst* iteminst);
//	
//	int	HasQuestFile(uint32 npcid);
//
//	
//};
//
//#endif //EMBPERL
//
//#endif //EMBPARSER_H
