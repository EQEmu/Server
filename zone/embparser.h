/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef EQEMU_EMBPARSER_H
#define EQEMU_EMBPARSER_H
#ifdef EMBPERL

#include "quest_parser_collection.h"
#include "quest_interface.h"
#include <string>
#include <queue>
#include <map>
#include "embperl.h"

class Mob;
class Client;
class NPC;

namespace EQ
{
	class ItemInstance;
}

typedef enum 
{
	questUnloaded,
	questLoaded,
	questFailedToLoad
} PerlQuestStatus;

class PerlembParser : public QuestInterface {
public:
	PerlembParser();
	~PerlembParser();
	
	virtual int EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	virtual int EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	virtual int EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	virtual int EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	virtual int EventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	virtual int EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);

	virtual bool HasQuestSub(uint32 npcid, QuestEventID evt);
	virtual bool HasGlobalQuestSub(QuestEventID evt);
	virtual bool PlayerHasQuestSub(QuestEventID evt);
	virtual bool GlobalPlayerHasQuestSub(QuestEventID evt);
	virtual bool SpellHasQuestSub(uint32 spell_id, QuestEventID evt);
	virtual bool ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt);

	virtual void LoadNPCScript(std::string filename, int npc_id);
	virtual void LoadGlobalNPCScript(std::string filename);
	virtual void LoadPlayerScript(std::string filename);
	virtual void LoadGlobalPlayerScript(std::string filename);
	virtual void LoadItemScript(std::string filename, EQ::ItemInstance *item);
	virtual void LoadSpellScript(std::string filename, uint32 spell_id);

	virtual void AddVar(std::string name, std::string val);
	virtual std::string GetVar(std::string name);
	virtual void ReloadQuests();
	virtual uint32 GetIdentifier() { return 0xf8b05c11; }

private:
	Embperl *perl;
	
	void ExportHash(const char *pkgprefix, const char *hashname, std::map<std::string, std::string> &vals);
	void ExportVar(const char *pkgprefix, const char *varname, const char *value);
	void ExportVar(const char *pkgprefix, const char *varname, int32 value);
	void ExportVar(const char *pkgprefix, const char *varname, uint32 value);
	void ExportVar(const char *pkgprefix, const char *varname, float value);
	void ExportVarComplex(const char *pkgprefix, const char *varname, const char *value);

	int EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, EQ::ItemInstance* item_inst, const SPDat_Spell_Struct* spell, Mob* mob, 
		uint32 extradata, bool global, std::vector<EQ::Any> *extra_pointers);
	int SendCommands(const char *pkgprefix, const char *event, uint32 spell_id, Mob* other, Mob* mob, EQ::ItemInstance *item_inst, const SPDat_Spell_Struct *spell);
	void MapFunctions();

	void GetQuestTypes(bool &isPlayerQuest, bool &isGlobalPlayerQuest, bool &isGlobalNPC, bool &isItemQuest, 
		bool &isSpellQuest, QuestEventID event, NPC* npcmob, EQ::ItemInstance* item_inst, Mob* mob, bool global);
	void GetQuestPackageName(bool &isPlayerQuest, bool &isGlobalPlayerQuest, bool &isGlobalNPC, bool &isItemQuest, 
		bool &isSpellQuest, std::string &package_name, QuestEventID event, uint32 objid, const char * data, 
		NPC* npcmob, EQ::ItemInstance* item_inst, bool global);
	void ExportCharID(const std::string &package_name, int &char_id, NPC *npcmob, Mob *mob);
	void ExportQGlobals(bool isPlayerQuest, bool isGlobalPlayerQuest, bool isGlobalNPC, bool isItemQuest, 
		bool isSpellQuest, std::string &package_name, NPC *npcmob, Mob *mob, int char_id);
	void ExportMobVariables(bool isPlayerQuest, bool isGlobalPlayerQuest, bool isGlobalNPC, bool isItemQuest, 
		bool isSpellQuest, std::string &package_name, Mob *mob, NPC *npcmob);
	void ExportZoneVariables(std::string &package_name);
	void ExportItemVariables(std::string &package_name, Mob *mob);
	void ExportEventVariables(std::string &package_name, QuestEventID event, uint32 objid, const char * data, 
		NPC* npcmob, EQ::ItemInstance* item_inst, Mob* mob, uint32 extradata, std::vector<EQ::Any> *extra_pointers);
	
	std::map<uint32, PerlQuestStatus> npc_quest_status_;
	PerlQuestStatus global_npc_quest_status_;
	PerlQuestStatus player_quest_status_;
	PerlQuestStatus global_player_quest_status_;
	std::map<uint32, PerlQuestStatus> item_quest_status_;
	std::map<uint32, PerlQuestStatus> spell_quest_status_;

	std::map<std::string, std::string> vars_;
	SV *_empty_sv;
	std::map<std::string, int> clear_vars_;
};

#endif
#endif

