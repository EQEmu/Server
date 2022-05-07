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

#ifndef _EQE_QUESTPARSERCOLLECTION_H
#define _EQE_QUESTPARSERCOLLECTION_H

#include "../common/types.h"

#include "encounter.h"
#include "beacon.h"
#include "client.h"
#include "corpse.h"
#include "doors.h"
#include "groups.h"
#include "mob.h"
#include "object.h"
#include "raids.h"
#include "trap.h"

#include "quest_interface.h"

#include "zone_config.h"

#include <list>
#include <map>

#define QuestFailedToLoad 0xFFFFFFFF
#define QuestUnloaded 0x00

extern const ZoneConfig *Config;
class Client;
class Mob;
class NPC;
class QuestInterface;

namespace EQ
{
	class Any;
	class ItemInstance;
}

class QuestParserCollection {
public:
	QuestParserCollection();
	~QuestParserCollection();

	void RegisterQuestInterface(QuestInterface *qi, std::string ext);
	void UnRegisterQuestInterface(QuestInterface *qi, std::string ext);
	void ClearInterfaces();
	void AddVar(std::string name, std::string val);
	void Init();
	void ReloadQuests(bool reset_timers = true);
	void RemoveEncounter(const std::string name);

	bool HasQuestSub(uint32 npcid, QuestEventID evt);
	bool PlayerHasQuestSub(QuestEventID evt);
	bool SpellHasQuestSub(uint32 spell_id, QuestEventID evt);
	bool ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt);

	int EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers = nullptr);
	int EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers = nullptr);
	int EventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers = nullptr);
	int EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers = nullptr);
	int EventEncounter(QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers = nullptr);
	
	void GetErrors(std::list<std::string> &quest_errors);

	/*
		Internally used memory reference for all Perl Event Export Settings
		Some exports are very taxing on CPU given how much an event is called.

		These are loaded via DB and have defaults loaded in PerlEventExportSettingsDefaults.

		Database loaded via Database::LoadPerlEventExportSettings(log_settings)
	*/

	struct PerlEventExportSettings {
		uint8 qglobals;
		uint8 mob;
		uint8 zone;
		uint8 item;
		uint8 event_variables;
	};

	PerlEventExportSettings perl_event_export_settings[_LargestEventID];

	void LoadPerlEventExportSettings(PerlEventExportSettings* perl_event_export_settings);

private:
	bool HasQuestSubLocal(uint32 npcid, QuestEventID evt);
	bool HasQuestSubGlobal(QuestEventID evt);
	bool PlayerHasQuestSubLocal(QuestEventID evt);
	bool PlayerHasQuestSubGlobal(QuestEventID evt);

	int EventNPCLocal(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers);
	int EventNPCGlobal(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers);
	int EventPlayerLocal(QuestEventID evt, Client *client, std::string data, uint32 extra_data,	std::vector<EQ::Any> *extra_pointers);
	int EventPlayerGlobal(QuestEventID evt, Client *client, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers);

	QuestInterface *GetQIByNPCQuest(uint32 npcid, std::string &filename);
	QuestInterface *GetQIByGlobalNPCQuest(std::string &filename);
	QuestInterface *GetQIByPlayerQuest(std::string &filename);
	QuestInterface *GetQIByGlobalPlayerQuest(std::string &filename);
	QuestInterface *GetQIBySpellQuest(uint32 spell_id, std::string &filename);
	QuestInterface *GetQIByItemQuest(std::string item_script, std::string &filename);
	QuestInterface *GetQIByEncounterQuest(std::string encounter_name, std::string &filename);
	
	int DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	int DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	int DispatchEventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
	int DispatchEventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);

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
	std::map<uint32, uint32> _item_quest_status;
	std::map<std::string, uint32> _encounter_quest_status;
};

extern QuestParserCollection *parse;

#endif

