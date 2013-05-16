// Quest Parser written by Wes, Leave this here =P
//Variables causing crash with linked lists (Bad pointers being added to the lists) - Npcs causing crashes with linked lists


#include <iostream>
#include <ctype.h>
using namespace std;
#include <fstream>
using namespace std;
#include <string>
#include <list>
#include <algorithm>

using namespace std;
#include "../common/debug.h"
#include "entity.h"
#include "masterentity.h"

#include "worldserver.h"
#include "net.h"
#include "../common/skills.h"
#include "../common/classes.h"
#include "../common/races.h"
#include "zonedb.h"
#include "../common/spdat.h"
#include "../common/packet_functions.h"
#include "spawn2.h"
#include "zone.h"
#include "event_codes.h"
#include <time.h>
#include "parser.h"
#include "basic_functions.h"
#include "questmgr.h"


extern Zone* zone;
extern WorldServer worldserver;
extern EntityList entity_list;


#define Parser_DEBUG 1

int mindex1 = 0;

const char * charIn3 = "`~1234567890-=!@#$%^&*_+qwertyuiop[]asdfghjkl'zxcvbnm,./QWERTYUIOP|ASDFGHJKL:ZXCVBNM<>?\"\\";

// MYRA - restore missing commands for qst type files & add itemlink
string cmds("if 0|break 1|spawn 6|spawn2 7|settimer 2|stoptimer 1|rebind 4|echo 1|summonitem 1|selfcast 1|zone 1|castspell 2|say 1|emote 1|shout 1|shout2 1|depop 1|exp 1|level 1|safemove 1|rain 1|snow 1|givecash 4|pvp 1|doanim 1|addskill 2|flagcheck 1|me 1|write 2|settarget 2|follow 1|sfollow 1|save 1|setallskill 1|faction 2|settime 2|setguild 2|setsky 1|setstat 2|movepc 5|gmmove 3|movegrp 4|signal 1|attack 1|itemlink 1|setglobal 4|delglobal 1|targlobal 6|setskill 2|setlanguage 2|stop 0|resume 0|start 1|moveto 3|pause 1|addldonpoints 2|");


//end Myra


int GetArgs(string command)
{
	string::iterator iterator = cmds.begin();
	string buffer;
	string cmd;
	string argnum;
	while (*iterator) {
		if (*iterator == ' ')
		{
			if (buffer.compare(command)==0) {
				cmd = buffer;
				buffer="";
			}
		}
		else {
			if (*iterator != '|')
			buffer += *iterator;
		}
		if (*iterator == '|')
		{
			if (!cmd.empty()) {
			int argnums = atoi(buffer.c_str());
			return argnums;
			}
			else {
				buffer="";
			}
		}
	iterator++;
	}
	return -1;
}

int calc(string calc)
{
	string::iterator iterator = calc.begin();
	string buffer;
	string integer1;
	char op = 0;
	int returnvalue = 0;

	while (*iterator) {
		char ch = *iterator;

		if(ch >= '0' && ch <= '9') { //If this character is numeric add it to the buffer
			buffer += ch;
		}
		else if(ch == '+' || ch == '-' || ch == '/' || ch == '*') { //otherwise, are we an operator?
			int val = atoi(buffer.c_str());
			if (!op) { //if this is the first time through, set returnvalue to what we have so far
				returnvalue = val;
			}
			else { //otherwise we've got returnvalue initialized, perform operation on previous numbers
				if (buffer.length() == 0) { //Do we have a value?
					printf("Parser::calc() Error in syntax: '%s'.\n", calc.c_str());
					return 0;
				}

				//what was the previous op
				switch(op)
				{
				case '+': {
					returnvalue += val;
					break;
				}
				case '-': {
					returnvalue -= val;
					break;
				}
				case '/': {
					if(val == 0)//can't divide by zero
					{
						printf("Parser::calc() Error, Divide by zero '%s'.\n", calc.c_str());
						return 0;
					}
					returnvalue /= val;
					break;
				}
				case '*': {
					returnvalue *= val;
					break;
				}
				};

				op = ch; //save current operator and continue parsing
			}
			buffer=""; //clear buffer now that we're starting on a new number
			op = ch;
		}
		else {
			printf("Parser::calc() Error processing '%c'.\n", ch);
			return 0;
		}
	}
	return returnvalue;
}

int Parser::numtok(const char *text, char character) {
	int returnvalue=0;
	for(; *text != '\0'; text++) {
		if(*text == character) returnvalue++;
	}
	return returnvalue;
}

string strlwr(string tmp) {
	string res;
	transform(tmp.begin(), tmp.end(), res.begin(), (int(*)(int))tolower);
	return(res);
}

int strcmp(const string &com, const string &com2) {
	return strcmp(com.c_str(),com2.c_str());
}

string gettok(const char *text, char character, int index)
{
	string buffer;
	int find=0;
	for(; *text != '\0'; *text++)
	{
		if (*text != character)
			buffer += *text;
		else {
			if (find == index)
				break;
			buffer = "";
			find++;
		}
	}
	return buffer;
}

void Parser::MakeVars(string text, uint32 npcid) {
	string buffer;
	string temp;
	int pos = numtok(text.c_str(),' ')+1;
	for(int i=0;i<pos;i++)
	{
			buffer = gettok(text.c_str(),' ',i).c_str();
			temp = (string)itoa(i+1); temp += "."; temp += (string)itoa(npcid);
#if Parser_DEBUG>10
				printf("Buffer: %s, temp: %s\n",buffer.c_str(),temp.c_str());
#endif
AddVar(temp,buffer);
			temp="";
			temp=(string)itoa(i+1) + "-." + (string)itoa(npcid);
			AddVar(temp,strstr(text.c_str(),buffer.c_str()));
			buffer="";
	}

}


int Parser::CheckAliases(const char * alias, uint32 npcid, Mob* npcmob, Mob* mob)
{
/*	MyListItem <Alias> * Ptr = AliasList.First;

	while (Ptr) {
		if ( (uint32)Ptr->Data->npcid == npcid) {
			for (int i=0; i <= Ptr->Data->index; i++) {
				if (!strcmp(strlwr(Ptr->Data->name[i]),alias)) {
					CommandEx(Ptr->Data->command[i], npcid, npcmob, mob);
					return 1;
				}
			}
		}
		Ptr = Ptr->Next;
	}
	return 0;*/
	return 0;
}


int Parser::pcalc(const char * string) {
	char temp[100];
	memset(temp, 0, sizeof(temp));
	char temp2[100];
	memset(temp2, 0, sizeof(temp2));
	int p =0;
	char temp3[100];
	memset(temp3, 0, sizeof(temp3));
	char temp4[100];
	memset(temp4, 0, sizeof(temp4));
	while (strrchr(string,'(')) {
		strn0cpy(temp,strrchr(string,'('), sizeof(temp));
		for ( unsigned int i=0;i < strlen(temp); i++ ) {
			if (temp[i] != '(' && temp[i] != ')') {
				temp2[p] = temp[i];
				p++;
			}
			else if (temp[i] == ')') {
				snprintf(temp3, sizeof(temp3), "(%s)", temp2);
//				Replace(string,temp3,itoa(calc(temp2),temp4,10),0);
				memset(temp, 0, sizeof(temp));
				memset(temp2, 0, sizeof(temp2));
				memset(temp3, 0, sizeof(temp3));
				memset(temp4, 0, sizeof(temp4));
				p=0;
			}
		}
	}
	return calc(string);
}

void Parser::MakeParms(const char * str, uint32 npcid) {
	char temp[100];
	memset(temp, 0, sizeof(temp));
	char temp2[100];
	memset(temp2, 0, sizeof(temp2));
	char temp3[100];
	memset(temp3, 0, sizeof(temp3));

	int tmpfor = numtok(str, ',')+1;
	for ( int i=0; i < tmpfor; i++) {
		memset(temp2, 0, sizeof(temp2));
		strn0cpy(temp2, gettok(str, ',', i).c_str(), sizeof(temp2));
		snprintf(temp, sizeof(temp), "param%s.%d", itoa(i+1 ,temp3, 10),npcid);
		AddVar(temp, temp2);
	}
}

int Parser::GetItemCount(string itemid, uint32 npcid)
{
	string temp;
	int a=0;
	for (int i=1;i<5;i++)
	{
		temp = (string)"item" + (string)itoa(i);
		if (!GetVar(temp,npcid).compare(itemid))
			a++;
		temp="";
	}
	return a;
}

int Parser::HasQuestFile(uint32 npcid)
{
	int32 qstID = GetNPCqstID(npcid);
	int success=1;
	if (qstID==-1)
		success = LoadScript(npcid, zone->GetShortName());
	if (!success)
		return false;

return true;
}

void Parser::Event(QuestEventID event, uint32 npcid, const char * data, NPC* npcmob, Mob* mob, uint32 extradata) {
	if (npcid == 0)
		return;
	if(event >= _LargestEventID)
		return;
	int32 qstID = GetNPCqstID(npcid);
	int success=1;
	if (qstID==-1)
		success = LoadScript(npcid, zone->GetShortName(),mob);
	if (!success)
		return;
	else
		qstID = GetNPCqstID(npcid);

// SCORPIOUS2K - load global variables

	if (npcmob->GetQglobal())
	{
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;
		MYSQL_RES *result;
		MYSQL_ROW row;
		char tmpname[65];
		int charid=0;

			if (mob && mob->IsClient())  // some events like waypoint and spawn don't have a player involved
			{
					charid=mob->CastToClient()->CharacterID();
			}

		else
		{
			charid=0-npcmob->GetNPCTypeID();		// make char id negative npc id as a fudge
		}

		AddVar("charid.g",itoa(charid));

		database.RunQuery(query, MakeAnyLenString(&query,
		  "SELECT name,value FROM quest_globals WHERE (npcid=%i || npcid=0) && (charid=%i || charid=0) && (zoneid=%i || zoneid=0) && expdate >= unix_timestamp(now())",
		     npcmob->GetNPCTypeID(),charid,zone->GetZoneID()), errbuf, &result);
		printf("%s\n",query);
		printf("%s\n",errbuf);
		if (result)
		{
			printf("Loading global variables for %s\n",npcmob->GetName());
			while ((row = mysql_fetch_row(result)))
			{
				sprintf(tmpname,"%s.g",row[0]);
				AddVar(tmpname, row[1]);
			}
			mysql_free_result(result);
		}
		if (query)
		{
			safe_delete_array(query);
			query=0;
		}
	}

	if (event == EVENT_TIMER)
	{
		AddVar("timername.g",data);
	}
	if (event == EVENT_SIGNAL)
	{
		AddVar("signal.g",data);
	}
	uint8 fac = 0;
	if (mob && mob->IsClient()) {
		AddVar("uguild_id.g", itoa(mob->CastToClient()->GuildID()));
		AddVar("uguildrank.g", itoa(mob->CastToClient()->GuildRank()));
	}

	if (mob && npcmob && mob->IsClient() && npcmob->IsNPC()) {
		Client* client = mob->CastToClient();
		NPC* npc = npcmob->CastToNPC();

		// Need to figure out why one of these casts would fail..
		if (client && npc) {
			fac = client->GetFactionLevel(client->GetID(), npcmob->GetID(), client->GetRace(), client->GetClass(), DEITY_AGNOSTIC, npc->GetPrimaryFaction(), npcmob);
		}
		else if (!client) {
			cerr << "WARNING: cast failure on mob->CastToClient()" << endl;
		}
		else if (!npc) {
			cerr << "WARNING: cast failure on npcmob->CastToNPC()" << endl;
		}
	}
	if (mob) {
		AddVar("name.g",   mob->GetName());
		AddVar("race.g",   GetRaceName(mob->GetRace()));
		AddVar("class.g",  GetEQClassName(mob->GetClass()));
		AddVar("ulevel.g", itoa(mob->GetLevel()));
		AddVar("userid.g", itoa(mob->GetID()));
	}
	if (npcmob)
	{
		AddVar("mname.g",npcmob->GetName());
	}

	if (fac) {
		AddVar("faction.g", itoa(fac));
	}

	if (zone) {
// SCORPIOUS2K- added variable zoneid
		AddVar("zoneid.g",itoa(zone->GetZoneID()));
		AddVar("zoneln.g",zone->GetLongName());
		AddVar("zonesn.g",zone->GetShortName());
	}

	string temp;
#if Parser_DEBUG>10
		printf("Data: %s,Id: %i\n",data,npcid);
#endif
	switch (event) {
		case EVENT_SAY: {
			MakeVars(data, npcid);
			npcmob->FaceTarget(mob);
			SendCommands("event_say", qstID, npcmob, mob);
			break;
		}
		case EVENT_TIMER: {
			SendCommands("event_timer", qstID, npcmob, mob);
			break;
		}
		case EVENT_DEATH: {
			SendCommands("event_death", qstID, npcmob, mob);
			break;
		}
		case EVENT_ITEM: {
			npcmob->FaceTarget(mob);
			SendCommands("event_item", qstID, npcmob, mob);
			break;
		}
		case EVENT_SPAWN: {
			SendCommands("event_spawn", qstID, npcmob, mob);
			break;
		}
		case EVENT_ATTACK: {
			SendCommands("event_attack", qstID, npcmob, mob);
			break;
		}
		case EVENT_COMBAT: {
			SendCommands("event_combat", qstID, npcmob, mob);
			break;
		}
		case EVENT_SLAY: {
			SendCommands("event_slay", qstID, npcmob, mob);
			break;
		}
		case EVENT_NPC_SLAY: {
			SendCommands("event_npc_slay", qstID, npcmob, mob);
			break;
		}
		case EVENT_WAYPOINT_ARRIVE: {
			temp = "wp." + (string)itoa(npcid);
			AddVar(temp,data);
			SendCommands("event_waypoint_arrive", qstID, npcmob, mob);
			break;
		}
		case EVENT_WAYPOINT_DEPART: {
			temp = "wp." + (string)itoa(npcid);
			AddVar(temp,data);
			SendCommands("event_waypoint_depart", qstID, npcmob, mob);
			break;
		}
		case EVENT_SIGNAL: {
			SendCommands("event_signal", qstID, npcmob, mob);
			break;
		}
		case EVENT_AGGRO: {
			SendCommands("event_aggro", qstID, npcmob, mob);
			break;
		}
		case EVENT_ENTER: {
			SendCommands("event_enter", qstID, npcmob, mob);
			break;
		}
		case EVENT_EXIT: {
			SendCommands("event_exit", qstID, npcmob, mob);
			break;
		}
		case EVENT_AGGRO_SAY: {
			MakeVars(data, npcid);
			SendCommands("event_aggro_say", qstID, npcmob, mob);
			break;
		}
		default: {
			// should we do anything here?
			break;
		}
	}
	DelChatAndItemVars(npcid);

}

Parser::Parser() : DEFAULT_QUEST_PREFIX("default") {
	MainList.clear();
	pNPCqstID = new int32[1];
	npcarrayindex=1;
}

Parser::~Parser() {
	MainList.clear();
	varlist.clear();
	AliasList.clear();
	safe_delete_array(pNPCqstID);
}

bool Parser::LoadAttempted(uint32 iNPCID) {
	if (iNPCID > pMaxNPCID)
		return false;

	return (bool) (FindNPCQuestID(iNPCID) != 0);
}
uint32 Parser::FindNPCQuestID(int32 npcid){
	for(uint32 i=0;i<npcarrayindex;i++)
		if(pNPCqstID[i]==npcid)
			return i;
	return 0;
}
uint32 Parser::AddNPCQuestID(uint32 npcid){
	int32* newpNPCqstID= new int32[npcarrayindex+2];
	for(uint32 i=0;i<npcarrayindex;i++)
		newpNPCqstID[i]=pNPCqstID[i];
	newpNPCqstID[npcarrayindex]=npcid;
	npcarrayindex++;
	safe_delete_array(pNPCqstID);
/*	pNPCqstID = new int32[npcarrayindex+1];
	for(uint32 j=0;j<npcarrayindex;j++)
		pNPCqstID[j]=newpNPCqstID[j];
	safe_delete_array(newpNPCqstID);*/
	pNPCqstID = newpNPCqstID;
	return npcarrayindex-1;
}
bool Parser::SetNPCqstID(uint32 iNPCID, int32 iValue) {
	if (iNPCID > pMaxNPCID)
		return false;
	uint32 idx = FindNPCQuestID(iNPCID);
	if(idx)
		pNPCqstID[idx] = iValue;
	else{
		idx=AddNPCQuestID(iNPCID);
		pNPCqstID[idx] = iValue;
	}
	return true;
}

int32 Parser::GetNPCqstID(uint32 iNPCID) {
	if (iNPCID > pMaxNPCID || iNPCID == 0)
		return -1;
	if(uint32 idx=FindNPCQuestID(iNPCID))
		return pNPCqstID[idx];
	else
		return -1;
}

void Parser::ClearCache() {
#if Parser_DEBUG >= 2
	cout << "Parser::ClearCache" << endl;
#endif
	//for (uint32 i=0; i<pMaxNPCID+1; i++)
	//	pNPCqstID[i] = -1;
	MainList.clear();
	safe_delete_array(pNPCqstID);
	pNPCqstID = new int32[1];
	npcarrayindex=1;
}

void Parser::SendCommands(const char * event, uint32 npcid, NPC* npcmob, Mob* mob) {
	iter_events listIt = MainList.begin();
	Events *p;
	EventList *pp;
	while (listIt != MainList.end())
	{
		p = *listIt;
		iter_eventlist listIt2 = p->Event.begin();
		if ( p->npcid == npcid ) {
			if(mob && mob->IsClient())
				quest_manager.StartQuest(npcmob, mob->CastToClient());
			else
				quest_manager.StartQuest(npcmob, nullptr);
			while (listIt2 != p->Event.end())
			{
				pp = *listIt2;
				if (pp && !strcmp(strlwr(pp->event.c_str()),strlwr(event))) {
#if Parser_DEBUG>10
						printf("PP command: %s\n",pp->command.c_str());
#endif
						ParseCommands(pp->command,0,0,npcid,npcmob,mob);
				}
				listIt2++;
			}
			quest_manager.EndQuest();
			return;
		}
		listIt++;
	}
}

void Parser::scanformat(char *string, const char *format, char arg[10][1024])
{
	int increment_arglist = 0;
	int argnum = 0;
	int i = 0;
	char lookfor = 0;

	// someone forgot to set string or format
	if(!format)
		return;
	if(!string)
		return;

	for(;;)
	{
		// increment while they're the same (and not nullptr)
		while(*format && *string && *format == *string) {
			format++;
			string++;
		}

		// format string is gone
		if(!format)
			break;
		// string is gone while the format string is still there (ERRor)
		if(!string)
			return;

		// the format string HAS to be equal to ÿ or else things are messed up
		if(*format != 'ÿ')
			return;

		format++;
		lookfor = *format;  // copy until we find char after 'y'
		format++;

		if(!lookfor)
			break;

		// start on a new arg
		if(increment_arglist) {
			arg[argnum][i] = 0;
			argnum++;
		}

		increment_arglist = 1; // we found the first crazy y
		i = 0;  // reset index

		while(*string && *string != lookfor)
			arg[argnum][i++] = *string++;
		string++;
	}

	// final part of the string
	if(increment_arglist) {
		arg[argnum][i] = 0;
		argnum++;
		i = 0;
	}

	while(*string)
		arg[argnum][i++] = *string++;

	arg[argnum][i] = 0;
}

void Parser::ClearEventsByNPCID(uint32 iNPCID) {
	list<Events*>::iterator iterator = MainList.begin();
	Events* p;
	while (iterator != MainList.end())
	{
		p = *iterator;
		if (p->npcid == iNPCID) {
			p->Event.clear();
			return;
		}
		iterator++;
	}
}

void Parser::ClearAliasesByNPCID(uint32 iNPCID) {
/*	MyListItem<Alias>* Ptr = AliasList.First;
	MyListItem<Alias>* next = 0;
	while (Ptr) {
		next = Ptr->Next;
		if ( (uint32)Ptr->Data->npcid == iNPCID) {
			AliasList.DeleteItemAndData(Ptr);
		}
		Ptr = next;
	}*/
}

void Parser::ExCommands(string o_command, string parms, int argnums, uint32 npcid, Mob* other, Mob* mob )
{
	char arglist[10][1024];
	//Work out the argument list, if there needs to be one
#if Parser_DEBUG>10
		printf("Parms: %s\n", parms.c_str());
#endif
	if (argnums > 1) {
		string buffer;
		string::iterator iterator = parms.begin();
		int quote=0,alist=0,ignore=0;
		while (1) {
			if (*iterator == '"') {
				if (quote) quote--;
				else	  { quote++; if (buffer.empty()) ignore=1; }
			}
			if (((*iterator != '"' && *iterator != ',' && *iterator != ' ' && *iterator != ')') || quote) && !ignore) {
				buffer+=*iterator;
			}
			if (ignore && *iterator == '"')ignore--;
			if ((*iterator == ',' && !quote)) {
				strcpy(arglist[alist],buffer.c_str());
				alist++;
				buffer="";
			}
			if (iterator == parms.end())
			{
				strcpy(arglist[alist],buffer.c_str());
				alist++;
				break;
			}
			iterator++;
		}
	}
	else {
		string::iterator iterator = parms.begin();
		if (*iterator == '"')
		{
			int quote=0,ignore=0;	//once=0
			string tmp;
			while (iterator != parms.end())
			{
				if (*iterator == '"')
				{
					if (quote)quote--;
					else quote++;
					ignore++;
				}

				if (!ignore  && (quote || (*iterator != '"')))
					tmp+=*iterator;

				else if (*iterator == '"' && ignore)
					ignore--;
				iterator++;
			}
			strcpy(arglist[0],tmp.c_str());
		}
		else
			strcpy(arglist[0],parms.c_str());
	}
#if Parser_DEBUG>10
		printf("After: %s\n", arglist[0]);
#endif

	char command[256];
	strncpy(command, o_command.c_str(), 255);
	command[255] = 0;
	char *cptr = command;
	while(*cptr != '\0') {
		if(*cptr >= 'A' && *cptr <= 'Z')
			*cptr = *cptr + ('a' - 'A');
		 *cptr++;
	}


	if (!strcmp(command,"write")) {
		quest_manager.write(arglist[0], arglist[1]);
	}
	else if (!strcmp(command,"me")) {
// MYRA - fixed comma bug for me command
		quest_manager.me(parms.c_str());
//end Myra
	}
	else if (!strcmp(command,"spawn") || !strcmp(command,"spawn2"))
	{

		float hdng;
		if (!strcmp(command,"spawn"))
		{
			hdng=mob->CastToClient()->GetHeading();
		}
		else
		{
			hdng=atof(arglist[6]);
		}
		quest_manager.spawn2(atoi(arglist[0]), atoi(arglist[1]), 0,
			atof(arglist[3]), atof(arglist[4]), atof(arglist[5]), hdng);
	}
	else if (!strcmp(command,"unique_spawn"))
	{

		float hdng;
		hdng=mob->CastToClient()->GetHeading();
		quest_manager.unique_spawn(atoi(arglist[0]), atoi(arglist[1]), 0,
			atof(arglist[3]), atof(arglist[4]), atof(arglist[5]), hdng);
	}
	else if (!strcmp(command,"echo")) {
		quest_manager.echo(atoi(arglist[0]), parms.c_str());
	}
	else if (!strcmp(command,"summonitem")) {
		quest_manager.summonitem(atoi(arglist[0]));
	}
	else if (!strcmp(command,"setstat")) {
		quest_manager.setstat(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"setanim")) {
		quest_manager.setanim(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"castspell")) {
		quest_manager.castspell(atoi(arglist[1]), atoi(arglist[0]));
	}
	else if (!strcmp(command,"selfcast")) {
		quest_manager.selfcast(atoi(arglist[0]));
	}
	else if (!strcmp(command,"addloot")) {//Cofruben: add an item to the mob.
		quest_manager.addloot(atoi(arglist[0]),atoi(arglist[1]));
	}
	else if (!strcmp(command,"zone")) {
		quest_manager.Zone(arglist[0]);
	}
	else if (!strcmp(command,"settimer")) {
		quest_manager.settimer(arglist[0], atoi(arglist[1]));
	}
	else if (!strcmp(command,"say")) {
		quest_manager.say(parms.c_str());
	}
	else if (!strcmp(command,"stoptimer")) {
		quest_manager.stoptimer(arglist[0]);
	}
	else if (!strcmp(command,"emote")) {
		quest_manager.emote(parms.c_str());
	}
	else if (!strcmp(command,"shout2")) {
		quest_manager.shout2(parms.c_str());
	}
	else if (!strcmp(command,"shout")) {
		quest_manager.shout(parms.c_str());
	}
	else if (!strcmp(command,"gmsay")) {
		quest_manager.shout2(parms.c_str());
	}
	else if (!strcmp(command,"depop")) {
		quest_manager.depop(atoi(arglist[0]));
	}
	else if (!strcmp(command,"settarget")) {
		quest_manager.settarget(arglist[0], atoi(arglist[1]));
	}
	else if (!strcmp(command,"follow"))  {
		quest_manager.follow(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"sfollow"))  {
		quest_manager.sfollow();
	}
	else if (!strcmp(command,"changedeity")) {
		quest_manager.changedeity(atoi(arglist[0]));
	}
	else if (!strcmp(command,"exp")) {
		quest_manager.exp(atoi(arglist[0]));
	}
	else if (!strcmp(command,"level")) {
		quest_manager.level(atoi(arglist[0]));
	}
	else if (!strcmp(command,"traindisc")) {
		quest_manager.traindisc(atoi(arglist[0]));
	}
	else if (!strcmp(command,"safemove")) {
		quest_manager.safemove();
	}
	else if (!strcmp(command,"rain")) {
		quest_manager.rain(atoi(arglist[0]));
	}
	else if (!strcmp(command,"snow")) {
		quest_manager.snow(atoi(arglist[0]));
	}
	else if (!strcmp(command,"surname")) {
		quest_manager.surname(arglist[0]);
	}
	else if (!strcmp(command,"permaclass")) {
		quest_manager.permaclass(atoi(arglist[0]));
	}
	else if (!strcmp(command,"permarace")) {
		quest_manager.permarace(atoi(arglist[0]));
	}
	else if (!strcmp(command,"permagender")) {
		quest_manager.permagender(atoi(arglist[0]));
	}
	else if (!strcmp(command,"scribespells")) {
		quest_manager.scribespells(atoi(arglist[0]));
	}
	else if (!strcmp(command,"traindiscs")) {
		quest_manager.traindiscs(atoi(arglist[0]));
	}
	else if (!strcmp(command,"givecash")) {
		quest_manager.givecash(atoi(arglist[0]), atoi(arglist[1]), atoi(arglist[2]), atoi(arglist[3]));
	}
	else if (!strcmp(command,"pvp")) {
		quest_manager.pvp(arglist[0]);
	}
	else if (!strcmp(command,"movepc")) {
		quest_manager.movepc((atoi(arglist[0])),(atof(arglist[1])),(atof(arglist[2])),(atof(arglist[3])),(atof(arglist[4])));
	}
	else if (!strcmp(command,"gmmove")) {
		quest_manager.gmmove(atof(arglist[0]), atof(arglist[1]), atof(arglist[2]));
	}
	else if (!strcmp(command,"movegrp")) {
		quest_manager.movegrp((atoi(arglist[0])),(atof(arglist[1])),(atof(arglist[2])),(atof(arglist[3])));
	}
	else if (!strcmp(command,"doanim")) {
		quest_manager.doanim(atoi(arglist[0]));
	}
	else if (!strcmp(command,"addskill")) {
		quest_manager.addskill(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"setlanguage")) {
		quest_manager.setlanguage(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"setskill")) {
		quest_manager.setskill(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"setallskill")) {
		quest_manager.setallskill(atoi(arglist[0]));
	}
	else if (!strcmp(command,"attack")) {
		quest_manager.attack(arglist[0]);
	}
	else if (!strcmp(command,"save")) {
		quest_manager.save();
	}
	/*else if (!strcmp(command,"flagcheck")) {
		quest_manager.flagcheck(atoi(arglist[0]), atoi(arglist[1]));
	}*/
	else if (!strcmp(command,"faction")) {
		quest_manager.faction(atoi(arglist[0]), atoi(arglist[1]), atoi(arglist[2]));
	}
	else if (!strcmp(command,"setsky")) {
		quest_manager.setsky(atoi(arglist[0]));
	}
	else if (!strcmp(command,"setguild")) {
		quest_manager.setguild(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"settime")) {
		quest_manager.settime(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"itemlink")) {
		quest_manager.itemlink(atoi(arglist[0]));
	}
	else if (!strcmp(command,"signal")) {
		quest_manager.signal(atoi(arglist[0]));
	}
	else if (!strcmp(command,"setglobal")) {
		quest_manager.setglobal(arglist[0], arglist[1], atoi(arglist[2]), arglist[3]);
	}
	else if (!strcmp(command,"targlobal")) {
		quest_manager.targlobal(arglist[0], arglist[1], arglist[2], atoi(arglist[3]), atoi(arglist[4]), atoi(arglist[5]));
	}
	else if (!strcmp(command,"ding")) {
		quest_manager.ding();
	}
	else if (!strcmp(command,"delglobal")) {
		quest_manager.delglobal(arglist[0]);
	}
	else if (!strcmp(command,"rebind")) {
		quest_manager.rebind((atoi(arglist[0])),(atof(arglist[1])),(atof(arglist[2])),(atof(arglist[3])));
	}
	else if (!strcmp(command,"stop")) {
		quest_manager.stop();
	}
	else if (!strcmp(command,"pause")) {
		quest_manager.pause(atoi(arglist[0]));
	}
	else if (!strcmp(command,"moveto")) {
		quest_manager.moveto(atof(arglist[0]), atof(arglist[1]), atof(arglist[2]), atof(arglist[3]), atoi(arglist[4]));
	}
	else if (!strcmp(command,"pathto")) {
		quest_manager.pathto(atof(arglist[0]), atof(arglist[1]), atof(arglist[2]));
	}
	else if (!strcmp(command,"showpath")) {
		quest_manager.showpath(atof(arglist[0]), atof(arglist[1]), atof(arglist[2]));
	}
	else if (!strcmp(command,"showgrid")) {
		quest_manager.showgrid(atoi(arglist[0]));
	}
	else if (!strcmp(command,"toggle_spawn_event")) {
		quest_manager.toggle_spawn_event(atoi(arglist[0]),(atoi(arglist[1])!=0),(atoi(arglist[2])!=0));
	}
	else if (!strcmp(command,"spawn_condition")) {
		quest_manager.spawn_condition(arglist[0], 0, atoi(arglist[1]), atoi(arglist[2]));
	}
	else if (!strcmp(command,"resume")) {
		quest_manager.resume();
	}
	else if (!strcmp(command,"start")) {
		quest_manager.start(atoi(arglist[0]));
	}
	else if (!strcmp(command,"addldonpoints")) {
		quest_manager.addldonpoints(atoi(arglist[0]), atoi(arglist[1]));
	}
	else if (!strcmp(command,"setnexthpevent")) {
		quest_manager.setnexthpevent(atoi(arglist[0]));
	}
	else if (!strcmp(command,"setnextinchpevent")) {
		quest_manager.setnextinchpevent(atoi(arglist[0]));
	}
	else if (!strcmp(command,"clear_zone_flag")) {
		quest_manager.clear_zone_flag(atoi(arglist[0]));
	}
	else if (!strcmp(command,"set_zone_flag")) {
		quest_manager.set_zone_flag(atoi(arglist[0]));
	}
	else if (!strcmp(command,"set_proximity")) {
		float v1 = atof(arglist[4]);
		float v2 = atof(arglist[5]);
		if(v1 == v2)	//omitted, or wrong, either way, skip them
			quest_manager.set_proximity(atof(arglist[0]), atof(arglist[1]), atof(arglist[2]), atof(arglist[3]));
		else
			quest_manager.set_proximity(atof(arglist[0]), atof(arglist[1]), atof(arglist[2]), atof(arglist[3]), v1, v2);
	}
	else if (!strcmp(command,"clear_proximity")) {
		quest_manager.clear_proximity();
	}
	else if (!strcmp(command,"respawn"))
	{
		quest_manager.respawn(atoi(arglist[0]), atoi(arglist[1]));
	}
	else
		printf("\nUnknown perl function used:%s",command);


}

int Parser::LoadScript(int npcid, const char * zone, Mob* activater)
{
	SetNPCqstID(npcid, npcid);
	ClearEventsByNPCID(npcid);
	ClearAliasesByNPCID(npcid);

	//string strnpcid = ("default");
	string strnpcid = (DEFAULT_QUEST_PREFIX);

	if (npcid)
		strnpcid = itoa(npcid);
	string filename;
	filename = "./quests/" + (string)zone + "/" + (string)strnpcid + ".qst";
	string line,buffer,temp;
	ifstream file( filename.c_str() );
	if (!file)
	{
		if (npcid) {
			SetNPCqstID(npcid, 0);
			LoadScript(0, zone);
		}
		else
			SetNPCqstID(0, -1);
	}

	int quote=0,ignore=0,bracket=0,line_num=0,paren=0;
	EventList* event1 = new EventList;
	Events * NewEventList = new Events;
	while (file && !file.eof())
	{
		getline(file,line);
		string::iterator iterator = line.begin();
        while (*iterator)
		{
			if (iterator[0] == '/' && iterator[1] == '/') break;
			if (!ignore && *iterator == '/' && iterator[1] == '*') { ignore++; iterator++; iterator++; }
			if (*iterator == '*' && iterator[1] == '/') { ignore--; iterator++; iterator++; }
			if (!ignore && (strchr(charIn,*iterator) || quote || paren))
				buffer+=*iterator;
			if (!ignore)
			{
			if (*iterator == '{')
			{
				bracket++;
				if (bracket == 1)
				{
					event1 = new EventList;
					NewEventList->npcid = npcid;
					buffer.replace(buffer.length()-1,buffer.length(),"");
					event1->event = buffer;
					buffer="";
				}
			}
			if (*iterator == '}')
			{
				bracket--;
				if (bracket == 0)
				{
				buffer.replace(buffer.length()-1,buffer.length(),"");
				int heh = ParseCommands(buffer,line_num,0,0,0,0,filename);
				if (!heh){
					safe_delete(NewEventList);
					return 0;
				}
				event1->command = buffer;
				buffer="";
				NewEventList->Event.push_back(event1);
				}
				if (bracket==-1)
				{
					if(activater && activater->IsClient())
					activater->CastToClient()->Message(10,"Line: %d,File: %s | error C0006: syntax error : too many ')'s",line_num,filename.c_str());
					return 0;
				}
			}
			if (*iterator == '"')if(quote)quote--;else quote++;
			if (*iterator == '(')paren++;
			if (*iterator == ')')paren--;
			}
			iterator++;
		}
		line_num++;

	}
	MainList.push_back(NewEventList);
	return 1;
}


void Parser::Replace(string& string1, string repstr, string rep, int all) {
	while (string1.find(repstr.c_str()) != string::npos) {
		string1.replace(string1.find(repstr.c_str()),repstr.length(),rep.c_str());
		if (!all)
			break;
	}
}

string Parser::GetVar(string varname, uint32 npcid)
{
	list<vars*>::iterator iterator = varlist.begin();
	vars * p;
	string checkfirst;
	string checksecond;
	checkfirst = varname + (string)"." + (string)itoa(npcid);
	checksecond = varname + (string)".g";

	while(iterator != varlist.end())
	{
		p = *iterator;
		if (!strcasecmp(p->name.c_str(), checkfirst.c_str()) || !strcasecmp(p->name.c_str(),checksecond.c_str()))
		{
printf("GetVar(%s) = '%s'\n", varname.c_str(), p->value.c_str());
			return p->value;
		}
		iterator++;
	}
	checkfirst="";
	checkfirst = "nullptr";
	return checkfirst;
}

void Parser::DeleteVar(string name)
{
	list<vars*>::iterator iterator = varlist.begin();
	vars* p;
	while(iterator != varlist.end())
	{
		p = *iterator;
		if (!p->name.compare(name))
		{
			varlist.erase(iterator);
			return;
		}
		iterator++;
	}
}

void Parser::DelChatAndItemVars(uint32 npcid)
{
//	MyListItem <vars> * Ptr;
	string temp;
	int i=0;
	for (i=0;i<10;i++)
	{
		temp = (string)itoa(i) + "." + (string)itoa(npcid);
		DeleteVar(temp);
		temp = (string)itoa(i) + "-." + (string)itoa(npcid);
		DeleteVar(temp);
	}
	for (i=1;i<5;i++)
	{
		temp = "item"+(string)itoa(i) + "." + (string)itoa(npcid);
		DeleteVar(temp);
		temp = "item"+(string)itoa(i) + ".stack." + (string)itoa(npcid);
		DeleteVar(temp);
	}
}

void Parser::AddVar(string varname, string varval)
{
	list<vars*>::iterator iterator = varlist.begin();
	vars* p;
	while(iterator != varlist.end())
	{
		p  = *iterator;
		if (!p->name.compare(varname))
		{
			p->value="";
			p->value = varval;
			return;
		}
		iterator++;
	}
	vars * newvar = new vars;
	newvar->name = varname;
	newvar->value = varval;
	varlist.push_back(newvar);
}

void Parser::HandleVars(string varname, string varparms, string& origstring, string format, uint32 npcid, Mob* mob)
{
	string tempvar;
	tempvar = GetVar(varname,npcid);
	char arglist[10][1024];
	string::iterator iterator = varparms.begin();
	string buffer;
	int quote=0;
	int alist=0;
	while (*iterator)
	{
		if (*iterator != '"' && *iterator != ',' && *iterator != ' ' || (quote && *iterator != '"'))
			buffer+=*iterator;
		if (*iterator == '"')
		{
			if (quote)quote--;
			else quote++;
		}
		if (*iterator == ',' && !quote)
		{
			strcpy(arglist[alist],buffer.c_str());
			alist++;
			buffer="";
		}
		iterator++;
	}
	strcpy(arglist[alist],buffer.c_str());
	if (!strcmp(strlwr((const char*)varname.c_str()),"mid")) {
		int pos=0;
		int one = atoi(arglist[1]);
		int two = atoi(arglist[2]);
		string buffer2;
		string find = arglist[0];
		string::iterator iterator = find.begin();
		while (*iterator)
		{
			pos++;
			if (pos>=one)
				buffer2+=*iterator;
			if (pos==two)
				break;
			iterator++;
		}
		Replace(origstring,format,buffer2.c_str());
	}
	else if (!strcmp(strlwr((const char*)varname.c_str()),"+")) {
		string temp;
		temp = (string)" "+format+(string)" ";
		Replace(origstring, temp, " REPLACETHISSHIT ",1);
	}
	else if (!strcmp(strlwr((const char*)varname.c_str()),"replace")) {
		string temp;
		temp = (string)arglist[0];
		Replace(temp, arglist[1], arglist[2],1);
		Replace(origstring, format, temp);
	}
	else if (!strcmp(strlwr(varname.c_str()),"itemcount")) {
		string temp;
		int o=0;
		o = GetItemCount(varparms,npcid);
		temp = (string)itoa(o);
		Replace(origstring,format,temp,1);
	}
	else if (!strcmp(strlwr(varname.c_str()),"calc")) {
		Replace(origstring,format,itoa(pcalc(varparms.c_str())));
	}
	else if (!strcmp(strlwr(varname.c_str()),"status") && mob &&  mob->IsClient()) {
		Replace(origstring,format,itoa(mob->CastToClient()->Admin()));
	}
	else if (!strcmp(strlwr(varname.c_str()),"hasitem") && mob && mob->IsClient()) {
		int has=0;
		for (int i=0; i<=30;i++) {
			if (mob->CastToClient()->GetItemIDAt(i) == (uint32) atoi(varparms.c_str())) {
				Replace(origstring,format,"true");
				has = 1;
				break;
			}
		}
		if (!has)
			Replace(origstring,format,"false");
	}
	else if (!strcmp(strlwr((const char*)varname.c_str()),"read")) {
		ifstream file(arglist[0]);
		if (file)
		{
			string line;
			int index=0,stop=atoi(arglist[1]);
			while (!file.eof())
			{
				getline(file,line);
				if (index == stop)break;
				index++;
			}
			Replace(origstring,format,line);
		}
	}
	else if (!strcmp(strlwr((const char*)varname.c_str()),"npc_status")) {
		Mob * tmp;
		if (!atoi(varparms.c_str())) {
			tmp = entity_list.GetMob(varparms.c_str());
		}
		else {
			tmp = entity_list.GetMobByNpcTypeID(atoi(varparms.c_str()));
		}
		if (tmp && tmp->GetHP() > 0) Replace(origstring,format,"up");
		else Replace(origstring,format,"down");
	}
	else if (!strcmp(strlwr((const char*)varname.c_str()),"strlen")) {
		Replace(origstring,format,itoa(varparms.length()-1));
	}
	else if (!strcmp(strlwr((const char*)varname.c_str()),"chr")) {
		char temp[4];
		memset(temp, 0x0, 4);
		temp[0] = atoi(varparms.c_str());
		Replace(origstring,format,temp);
	}
	//used_pawn - random implementation.
	else if (!strcmp(strlwr((const char*)varname.c_str()),"random")) {
		Replace(origstring,format,itoa(MakeRandomInt(0, varparms[0]-1)));
	}
   else if (!strcmp(strlwr((const char*)varname.c_str()),"asc")) {
      Replace(origstring,format,itoa(varparms[0]));
   }
	else if (!strcmp(strlwr((const char*)varname.c_str()),"gettok")) {
		Replace(origstring,format,gettok(arglist[0],arglist[1][0],atoi(arglist[2])));
	}
	else {
		Replace(origstring,format,tempvar);
	}
	gClient = 0;
}

void Parser::ParseVars(string& text, uint32 npcid, Mob* mob)
{
	if (text.find("$") == string::npos && text.find("%") == string::npos)
		return;
	string buffer2;
	string fname;
	string parms;
	while (text.find("%") != string::npos)
	{
		string temp;
		temp = (string)text.substr(text.find("%")).c_str();
		string::iterator iterator = temp.begin();
		string buffer;
		while (*iterator)
		{
			if (!strrchr(notin,*iterator))
				buffer+=*iterator;
			else
			{
				HandleVars(buffer,0,text,buffer,npcid,mob);
				Replace(text,buffer,"testing",0);
				break;
			}
			iterator++;
		}
	}
	while (text.find("$") != string::npos)
	{
		string temp;
		temp = (string)text.substr(text.rfind("$")).c_str();
		string::iterator iterator = temp.begin();
		int paren=0;
		int fin=0;
		string buffer;
		while (iterator != temp.end())
		{
			if (!strrchr(notin,*iterator) || paren)
			{
				if (*iterator != '(' && *iterator != ')')
					buffer+=*iterator;
			}
			else
			{
					buffer.replace(0,1,"",0);
					HandleVars(buffer,"",text,buffer2,npcid,mob);
					buffer="";
					buffer2="";
					break;
			}
			buffer2+=*iterator;
			if (*iterator == '(')
			{
				paren++;
				if (paren == 1)
				{
					fname = buffer;
					buffer="";
				}
			}
			if (*iterator == ')')
			{
				paren--;
				if (paren == 0)
				{
					parms = buffer;
					fname.replace(0,1,"",0);
					HandleVars(fname,parms,text,buffer2,npcid,mob);
					buffer="";
					buffer2="";
					parms="";
					fname="";
					fin=1;
					break;
				}
			}
			iterator++;
		}
	}
}

/*
char * fixstring(char * string)
{
	char tmp[255];
	memset(tmp,0x0,255);

	static char tmp2[255];
	memset(tmp2,0x0,255);
	int quote=0;
	int	o=0;
	strcpy(tmp,string);
	uint32 len = strlen(tmp);
	for (uint32 i=0;i<len;i++)
	{
		if (quote || tmp[i] != ' ') {
			tmp2[o] = tmp[i];
			o++;
		}
		if (tmp[i] == '"')
			if (quote) quote--;
			else	   quote++;
	}
	return tmp2;
}*/

int DoCompare(string compare1, string sign, string compare2)
{
#if Parser_DEBUG>10
		printf("compare1: %s,sign: %s, compare2: %s\n",compare1.c_str(),sign.c_str(),compare2.c_str());
#endif
	if (!strcmp(sign.c_str(),"==")) {
		if (strcmp(strlwr((const char*)compare1.c_str()),strlwr((const char*)compare2.c_str())))
			return 0;
	}
	else if (!strcmp(sign.c_str(),"!=")) {
		if (!strcmp(strlwr((const char*)compare1.c_str()),strlwr((const char*)compare2.c_str())))
			return 0;
	}
	else if (!strcmp(sign.c_str(),"=~")) {
		if (!strstr(strlwr(compare1.c_str()).c_str(),strlwr(compare2.c_str()).c_str()))
			return 0;
	}
	else if (!strcmp(sign.c_str(),"!~")) {
		if (strstr(strlwr(compare1.c_str()).c_str(),strlwr(compare2.c_str()).c_str()))
			return 0;
	}
	else if (!strcmp(sign.c_str(),"<")) {
		if (atoi(compare1.c_str()) > atoi(compare2.c_str()) || atoi(compare1.c_str()) == atoi(compare2.c_str()))
			return 0;
	}
	else if (!strcmp(sign.c_str(),">")) {
		if (atoi(compare1.c_str()) < atoi(compare2.c_str()) || atoi(compare1.c_str()) == atoi(compare2.c_str()))
			return 0;
	}
	else if (!strcmp(sign.c_str(),"<=")) {
		if (atoi(compare1.c_str()) > atoi(compare2.c_str()) || atoi(compare1.c_str()) != atoi(compare2.c_str()))
			return 0;
	}
	else if (!strcmp(sign.c_str(),">=")) {
		if (atoi(compare1.c_str()) < atoi(compare2.c_str()) || atoi(compare1.c_str()) != atoi(compare2.c_str()))
			return 0;
	}
	return 1;
}

int Parser::ParseIf(string text)
{
	string::iterator iterator = text.begin();
	string com1,com2,sign,next,buffer;
	while (*iterator)
	{
		if (!strchr(notin,*iterator))
			buffer+=*iterator;
		if (*iterator == '=' || *iterator == '!' || *iterator == '~')
		{
			sign+=*iterator;
			if (sign.length() == 1)
			{
				com1 = buffer;
				buffer="";
				next="";
			}
		}
		if ((*iterator == '<' || *iterator == '>') && iterator[1] != '=')
		{
			sign+=*iterator;
			if (sign.length() == 1)
			{
				com1 = buffer;
				buffer="";
				next="";
			}
			iterator++;
		}
		if (*iterator == '&' || *iterator == '|')
		{
			next+=*iterator;
			if (next.length() == 1)
			{
				com2 = buffer;
				buffer="";
			}
			if (next.length() == 2)
			{
				if (!DoCompare(com1,sign,com2) && strcmp(next.c_str(),"||"))
					return 0;
				com1="";
				sign="";
				com2="";
			}
		}
		iterator++;
		if (iterator == text.end()) {
			com2 = buffer;
			//com2.replace(0,1,"");
			buffer="";
			if (!DoCompare(com1,sign,com2))
				return 0;
		}
	}
	return 1;
}

int Parser::ParseCommands(string text, int line, int justcheck, uint32 npcid, Mob* other, Mob* mob,  std::string filename)
{
	string buffer,command,parms,temp,temp2;
	temp2 = text;
	ParseVars(temp2,npcid,mob);
	int bracket=0,paren=0,lastif=0,last_finished=0,quote=0,escape=0,ignore=0,argnums=0,argit=1;
	string::iterator iterator = temp2.begin();
	while (iterator != temp2.end())
	{
		if (*iterator == '\\' && !escape) {
			escape++;
		}
		//"`~1234567890-=!@#$%^&*_+qwertyuiop[]asdfghjkl'zxcvbnm,./QWERTYUIOP|ASDFGHJKL:ZXCVBNM<>?\"\\"
		if (!ignore && *iterator != ')' && (strchr(charIn3,*iterator) || quote || escape || paren))
		{
			buffer+=*iterator;
		}
		if (*iterator == '"' && !escape && !ignore)
			if (quote)quote--;
			else quote++;

		if (*iterator == ',' && !ignore && !quote && !escape && paren)
			argit++;

		if (*iterator == '(' && !ignore && !quote && !escape)
		{
			paren++;
			if (paren == 1)
			{
				if (last_finished)
				{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0008: syntax error : missing ';' before function '%s'", line, filename.c_str(), command.c_str());
					return 0;
				}
				command = buffer;
				if(!strcmp(strlwr(command.c_str()),"break") && other)
					return 1;
				buffer="";
				argnums = GetArgs(command);
#if Parser_DEBUG>10
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Command: %s, Num Args: %i\n",command.c_str(),argnums);
#endif
				if (argnums == -1)
				{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0007: '%s' : Unknown function", line, filename.c_str(), command.c_str());
					return 0;
				}
			}
		}
		else if (*iterator == ')' && !ignore && !quote && !escape)
		{
			paren--;
			if (paren == 0)
			{
				lastif=0,quote=0,escape=0,ignore=0;
				parms = buffer;
				buffer="";
				if (!strcmp(strlwr((const char*)command.c_str()),"if")) { last_finished=0; }
				else {
					last_finished=1;
				}
			}
			if (paren<0)
			{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0006: syntax error : too many ')'s",line,filename.c_str() );
				return 0;
			}
		}
		else if (*iterator == '{' && !escape)
		{
			bracket++;
			if (!ignore) {
				if (!strcmp(strlwr((const char*)command.c_str()),"if")) {
					lastif = ParseIf(parms);
#if Parser_DEBUG>10
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Parms: %s\n",parms.c_str());
#endif
					if (!lastif) ignore=1;
					else ignore=0;
				}
			}
		}
		else if (*iterator == '}' && !escape)
		{
			bracket--;
			if (last_finished)
			{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0008: syntax error : missing ';' before '}'", line,filename.c_str() );
				return 0;
			}
			if (bracket<0)
			{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0006: syntax error : too many '}'s",line, filename.c_str() );
				return 0;
			}
			if (bracket == 0)
			{
				if (!ignore) lastif=1;
				else		 lastif=0;
				lastif=0,last_finished=0,quote=0,escape=0,ignore=0,argnums=0,argit=1;
			}
		}
		else if (*iterator == ';' && !escape && !ignore && !quote)
		{
			if (last_finished)
			{
				last_finished=0;
				if (argnums != 1 && argnums!=argit)
				{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d, File: %s | error C0001: '%s' : function does not take %d parameter(s)", line, filename.c_str(), command.c_str(), argit);
					return 0;
				}
				if (!justcheck)
					ExCommands((const char*)command.c_str(),(const char*)parms.c_str(),argnums, npcid, other, mob);
				argit=1;
			}
			else {
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0002: '%s' :syntax error : '(' %d '('s still not closed.", line, filename.c_str(), command.c_str(), paren);
				}
		}
		if (escape) escape--;
			iterator++;
	}
	if (last_finished)
	{
					if(mob && mob->IsClient())
					mob->CastToClient()->Message(10,"Line: %d,File: %s | error C0008: syntax error : missing ';' before '}'", line, filename.c_str() );
		return 0;
	}
	return 1;
}

void Parser::ReloadQuests(bool with_timers) {
	ClearCache();
}

