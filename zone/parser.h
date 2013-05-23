#ifndef PARSER_H
#define PARSER_H

#define Parser_MaxVars	1024
#include "../common/timer.h"
#include <string>
#include <list>
#include "event_codes.h"
#include "QuestInterface.h"


struct EventList {
	std::string event;
	std::string command;
};

struct Events {
uint32 npcid;
std::list<EventList*> Event;
};

struct Alias {
	int index;
	uint32 npcid;
	char name[100][100];
	char command[100][1024];
};

struct vars {
	std::string name;
	std::string value;
};

struct command_list {
	char command_name[100];
	int param_amount[17];
};


class Parser : public QuestInterface
{
public:
	Parser();
	virtual ~Parser();
	int mindex;
	const std::string DEFAULT_QUEST_PREFIX;

	typedef std::list<Events*>::iterator iter_events;
	typedef std::list<EventList*>::iterator iter_eventlist;
	std::list<Events*> MainList;
	std::list<vars*> varlist;
	std::list<Alias*> AliasList;
	uint32	npcarrayindex;

	uint32	AddNPCQuestID(uint32 npcid);
	uint32	FindNPCQuestID(int32 npcid);
	int		CheckAliases(const char * alias, uint32 npcid, Mob* npcmob, Mob* mob);
	void	ClearAliasesByNPCID(uint32 iNPCID);
	void	ClearCache();
	void	ClearEventsByNPCID(uint32 iNPCID);

	void	DelChatAndItemVars(uint32 npcid);
	void	DeleteVar(std::string name);

	void	ExCommands(std::string command, std::string parms, int argnums, uint32 npcid, Mob* other, Mob* mob );

	void	GetCommandName(char * command1, char * arg);
	int		GetFreeID();
	int		GetItemCount(std::string itemid, uint32 npcid);
	int32	GetNPCqstID(uint32 iNPCID);
	std::string	GetVar(std::string varname, uint32 npcid);

	void	HandleVars(std::string varname, std::string varparms, std::string& origstring, std::string format, uint32 npcid, Mob* mob);

	bool	LoadAttempted(uint32 iNPCID);
	void	LoadCommands(const char * filename);
	virtual int 	LoadScript(int npcid, const char * zone, Mob* activater=0);

	void	MakeParms(const char * string, uint32 npcid);
	void	MakeVars(std::string text, uint32 npcid);

	int		numtok(const char *text, char character);

	int		ParseCommands(std::string text, int line, int justcheck, uint32 npcid, Mob* other, Mob* mob, std::string filename=std::string("none"));
	int		ParseIf(std::string text);
	int		pcalc(const char * string);
	void	ParseVars(std::string& text, uint32 npcid, Mob* mob);

	void	Replace(std::string& string1, std::string repstr, std::string rep, int all=0);

	void	scanformat(char *string, const char *format, char arg[10][1024]);
	bool	SetNPCqstID(uint32 iNPCID, int32 iValue);
	char *	strrstr(char* string, const char * sub);
	virtual void	SendCommands(const char * event, uint32 npcid, NPC* npcmob, Mob* mob);

	int	HasQuestFile(uint32 npcid);

	//interface stuff
	virtual void EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {}
	virtual void EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {}
	virtual void EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {}
	virtual void EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {}
	virtual void EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {}
	virtual bool HasQuestSub(uint32 npcid, const char *subname) { return HasQuestFile(npcid) != 0; }
	virtual bool PlayerHasQuestSub(const char *subname) { return true; }
	virtual bool GlobalPlayerHasQuestSub(const char *subname) { return true; }
	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname) { return true; }
	virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname) { return true; }
	virtual void AddVar(std::string varname, std::string varval);
	virtual void ReloadQuests(bool with_timers = false);
	virtual uint32 GetIdentifier() { return 0x04629fff; }

private:
	//void	Event(int event, uint32 npcid, const char * data, Mob* npcmob, Mob* mob);
	//changed - Eglin. more reasonable (IMHO) than changing every single referance to the global pointer.
	//that's what you get for using globals! :)
	virtual void Event(QuestEventID event, uint32 npcid, const char * data, NPC* npcmob, Mob* mob, uint32 extradata = 0);

	uint32	pMaxNPCID;
	int32*	pNPCqstID;
};

#endif

