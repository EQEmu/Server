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


#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/features.h"

#include "quest_parser_collection.h"
#include "quest_interface.h"
#include "zone.h"
#include "questmgr.h"
#include "../common/path_manager.h"
#include "../common/repositories/perl_event_export_settings_repository.h"
#include "../common/file.h"

#include <stdio.h>

extern Zone* zone;
extern void MapOpcodes();

QuestParserCollection::QuestParserCollection()
{
	_player_quest_status        = QuestUnloaded;
	_global_player_quest_status = QuestUnloaded;
	_global_npc_quest_status    = QuestUnloaded;
	_bot_quest_status           = QuestUnloaded;
	_global_bot_quest_status    = QuestUnloaded;
}

QuestParserCollection::~QuestParserCollection() { }

void QuestParserCollection::RegisterQuestInterface(QuestInterface* qi, std::string ext)
{
	_interfaces[qi->GetIdentifier()] = qi;
	_extensions[qi->GetIdentifier()] = ext;
	_load_precedence.push_back(qi);
}

void QuestParserCollection::ClearInterfaces()
{
	_interfaces.clear();
	_extensions.clear();
	_load_precedence.clear();
}

void QuestParserCollection::AddVar(std::string name, std::string val)
{
	for (const auto& e: _load_precedence) {
		e->AddVar(name, val);
	}
}

void QuestParserCollection::Init()
{
	for (const auto& e: _load_precedence) {
		e->Init();
	}
}

void QuestParserCollection::ReloadQuests(bool reset_timers)
{
	if (reset_timers) {
		quest_manager.ClearAllTimers();
	}

	MapOpcodes();

	_npc_quest_status.clear();

	_player_quest_status        = QuestUnloaded;
	_global_player_quest_status = QuestUnloaded;
	_global_npc_quest_status    = QuestUnloaded;
	_bot_quest_status           = QuestUnloaded;
	_global_bot_quest_status    = QuestUnloaded;

	_spell_quest_status.clear();
	_item_quest_status.clear();
	_encounter_quest_status.clear();

	for (const auto& e: _load_precedence) {
		e->ReloadQuests();
	}
}

void QuestParserCollection::RemoveEncounter(const std::string& name)
{
	for (const auto& e: _load_precedence) {
		e->RemoveEncounter(name);
	}
}

bool QuestParserCollection::HasQuestSub(uint32 npc_id, QuestEventID event_id)
{
	return (
		HasQuestSubLocal(npc_id, event_id) ||
		HasQuestSubGlobal(event_id) ||
		NPCHasEncounterSub(npc_id, event_id)
	);
}

bool QuestParserCollection::NPCHasEncounterSub(uint32 npc_id, QuestEventID event_id)
{
	return HasEncounterSub(
		event_id,
		fmt::format(
			"npc_{}",
			npc_id
		)
	);
}

bool QuestParserCollection::HasQuestSubLocal(uint32 npc_id, QuestEventID event_id)
{
	auto iter = _npc_quest_status.find(npc_id);

	if (iter != _npc_quest_status.end()) {
		if (iter->second != QuestFailedToLoad) { //loaded or failed to load
			auto qiter = _interfaces.find(iter->second);
			if (qiter->second->HasQuestSub(npc_id, event_id)) {
				return true;
			}
		}
	} else {
		std::string filename;
		auto        qi = GetQIByNPCQuest(npc_id, filename);

		if (qi) {
			_npc_quest_status[npc_id] = qi->GetIdentifier();

			qi->LoadNPCScript(filename, npc_id);
			if (qi->HasQuestSub(npc_id, event_id)) {
				return true;
			}
		} else {
			_npc_quest_status[npc_id] = QuestFailedToLoad;
		}
	}

	return false;
}

bool QuestParserCollection::HasQuestSubGlobal(QuestEventID event_id)
{
	if (_global_npc_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByGlobalNPCQuest(filename);

		if (qi) {
			qi->LoadGlobalNPCScript(filename);
			_global_npc_quest_status = qi->GetIdentifier();
			if (qi->HasGlobalQuestSub(event_id)) {
				return true;
			}
		}
	} else {
		if (_global_npc_quest_status != QuestFailedToLoad) {
			auto qiter = _interfaces.find(_global_npc_quest_status);
			if (qiter->second->HasGlobalQuestSub(event_id)) {
				return true;
			}
		}
	}

	return false;
}

bool QuestParserCollection::PlayerHasQuestSub(QuestEventID event_id)
{
	return (
		PlayerHasQuestSubLocal(event_id) ||
		PlayerHasQuestSubGlobal(event_id) ||
		PlayerHasEncounterSub(event_id)
	);
}

bool QuestParserCollection::PlayerHasEncounterSub(QuestEventID event_id)
{
	return HasEncounterSub(event_id, "player");
}

bool QuestParserCollection::PlayerHasQuestSubLocal(QuestEventID event_id)
{
	if (_player_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByPlayerQuest(filename);

		if (qi) {
			_player_quest_status = qi->GetIdentifier();
			qi->LoadPlayerScript(filename);
			return qi->PlayerHasQuestSub(event_id);
		}
	} else if (_player_quest_status != QuestFailedToLoad) {
		auto iter = _interfaces.find(_player_quest_status);
		return iter->second->PlayerHasQuestSub(event_id);
	}

	return false;
}

bool QuestParserCollection::PlayerHasQuestSubGlobal(QuestEventID event_id)
{
	if (_global_player_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByGlobalPlayerQuest(filename);

		if (qi) {
			_global_player_quest_status = qi->GetIdentifier();
			qi->LoadGlobalPlayerScript(filename);
			return qi->GlobalPlayerHasQuestSub(event_id);
		}
	} else if (_global_player_quest_status != QuestFailedToLoad) {
		auto iter = _interfaces.find(_global_player_quest_status);
		return iter->second->GlobalPlayerHasQuestSub(event_id);
	}

	return false;
}

bool QuestParserCollection::SpellHasEncounterSub(uint32 spell_id, QuestEventID event_id)
{
	return HasEncounterSub(
		event_id,
		fmt::format(
			"spell_{}",
			spell_id
		)
	);
}

bool QuestParserCollection::SpellHasQuestSub(uint32 spell_id, QuestEventID event_id)
{
	if (SpellHasEncounterSub(spell_id, event_id)) {
		return true;
	}

	auto iter = _spell_quest_status.find(spell_id);
	if (iter != _spell_quest_status.end()) {
		//loaded or failed to load
		if (iter->second != QuestFailedToLoad) {
			auto qiter = _interfaces.find(iter->second);
			return qiter->second->SpellHasQuestSub(spell_id, event_id);
		}
	} else if (_spell_quest_status[spell_id] != QuestFailedToLoad) {
		std::string filename;
		auto        qi = GetQIBySpellQuest(spell_id, filename);

		if (qi) {
			_spell_quest_status[spell_id] = qi->GetIdentifier();
			qi->LoadSpellScript(filename, spell_id);
			return qi->SpellHasQuestSub(spell_id, event_id);
		} else {
			_spell_quest_status[spell_id] = QuestFailedToLoad;
		}
	}

	return false;
}

bool QuestParserCollection::ItemHasEncounterSub(EQ::ItemInstance* inst, QuestEventID event_id)
{
	if (inst) {
		return HasEncounterSub(
			event_id,
			fmt::format(
				"item_{}",
				inst->GetID()
			)
		);
	}

	return false;
}

bool QuestParserCollection::ItemHasQuestSub(EQ::ItemInstance* inst, QuestEventID event_id)
{
	if (!inst) {
		return false;
	}

	if (ItemHasEncounterSub(inst, event_id)) {
		return true;
	}

	std::string item_script;
	if (inst->GetItem()->ScriptFileID != 0) {
		item_script = fmt::format(
			"script_{}",
			inst->GetItem()->ScriptFileID
		);
	} else if (strlen(inst->GetItem()->CharmFile) > 0) {
		item_script = inst->GetItem()->CharmFile;
	} else {
		item_script = std::to_string(inst->GetID());
	}

	uint32 item_id = inst->GetID();
	auto   iter    = _item_quest_status.find(item_id);
	if (iter != _item_quest_status.end()) {
		//loaded or failed to load
		if (iter->second != QuestFailedToLoad) {
			auto qiter = _interfaces.find(iter->second);
			return qiter->second->ItemHasQuestSub(inst, event_id);
		}
	} else {
		std::string filename;
		auto        qi = GetQIByItemQuest(item_script, filename);
		if (qi) {
			_item_quest_status[item_id] = qi->GetIdentifier();
			qi->LoadItemScript(filename, inst);
			return qi->ItemHasQuestSub(inst, event_id);
		} else {
			_item_quest_status[item_id] = QuestFailedToLoad;
		}
	}

	return false;
}

bool QuestParserCollection::HasEncounterSub(QuestEventID event_id, const std::string& package_name)
{
	for (auto it = _encounter_quest_status.begin(); it != _encounter_quest_status.end(); ++it) {
		if (it->second != QuestFailedToLoad) {
			auto qit = _interfaces.find(it->second);
			if (qit != _interfaces.end() && qit->second->HasEncounterSub(package_name, event_id)) {
				return true;
			}
		}
	}

	return false;
}

bool QuestParserCollection::BotHasQuestSubLocal(QuestEventID event_id)
{
	if (_bot_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByBotQuest(filename);

		if (qi) {
			_bot_quest_status = qi->GetIdentifier();
			qi->LoadBotScript(filename);
			return qi->BotHasQuestSub(event_id);
		}
	} else if (_bot_quest_status != QuestFailedToLoad) {
		auto iter = _interfaces.find(_bot_quest_status);
		return iter->second->BotHasQuestSub(event_id);
	}

	return false;
}

bool QuestParserCollection::BotHasQuestSubGlobal(QuestEventID event_id)
{
	if (_global_bot_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByGlobalBotQuest(filename);

		if (qi) {
			_global_bot_quest_status = qi->GetIdentifier();
			qi->LoadGlobalBotScript(filename);
			return qi->GlobalBotHasQuestSub(event_id);
		}
	} else if (_global_bot_quest_status != QuestFailedToLoad) {
		auto iter = _interfaces.find(_global_bot_quest_status);
		return iter->second->GlobalBotHasQuestSub(event_id);
	}

	return false;
}

bool QuestParserCollection::BotHasQuestSub(QuestEventID event_id)
{
	return BotHasQuestSubLocal(event_id) || BotHasQuestSubGlobal(event_id);
}

int QuestParserCollection::EventNPC(
	QuestEventID event_id,
	NPC* npc,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	const int local_return   = EventNPCLocal(event_id, npc, init, data, extra_data, extra_pointers);
	const int global_return  = EventNPCGlobal(event_id, npc, init, data, extra_data, extra_pointers);
	const int default_return = DispatchEventNPC(event_id, npc, init, data, extra_data, extra_pointers);

	if (local_return != 0) {
		return local_return;
	} else if (global_return != 0) {
		return global_return;
	} else if (default_return != 0) {
		return default_return;
	}

	return 0;
}

int QuestParserCollection::EventNPCLocal(
	QuestEventID event_id,
	NPC* npc,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	auto iter = _npc_quest_status.find(npc->GetNPCTypeID());
	if (iter != _npc_quest_status.end()) {
		//loaded or failed to load
		if (iter->second != QuestFailedToLoad) {
			auto qiter = _interfaces.find(iter->second);
			return qiter->second->EventNPC(event_id, npc, init, data, extra_data, extra_pointers);
		}
	} else if (_npc_quest_status[npc->GetNPCTypeID()] != QuestFailedToLoad) {
		std::string filename;
		auto        qi = GetQIByNPCQuest(npc->GetNPCTypeID(), filename);

		if (qi) {
			_npc_quest_status[npc->GetNPCTypeID()] = qi->GetIdentifier();
			qi->LoadNPCScript(filename, npc->GetNPCTypeID());
			return qi->EventNPC(event_id, npc, init, data, extra_data, extra_pointers);
		} else {
			_npc_quest_status[npc->GetNPCTypeID()] = QuestFailedToLoad;
		}
	}

	return 0;
}

int QuestParserCollection::EventNPCGlobal(
	QuestEventID event_id,
	NPC* npc,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (_global_npc_quest_status != QuestUnloaded && _global_npc_quest_status != QuestFailedToLoad) {
		auto qiter = _interfaces.find(_global_npc_quest_status);
		return qiter->second->EventGlobalNPC(event_id, npc, init, data, extra_data, extra_pointers);
	} else if (_global_npc_quest_status != QuestFailedToLoad) {
		std::string filename;
		auto        qi = GetQIByGlobalNPCQuest(filename);

		if (qi) {
			_global_npc_quest_status = qi->GetIdentifier();
			qi->LoadGlobalNPCScript(filename);
			return qi->EventGlobalNPC(event_id, npc, init, data, extra_data, extra_pointers);
		} else {
			_global_npc_quest_status = QuestFailedToLoad;
		}
	}

	return 0;
}

int QuestParserCollection::EventPlayer(
	QuestEventID event_id,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	const int local_return   = EventPlayerLocal(event_id, client, data, extra_data, extra_pointers);
	const int global_return  = EventPlayerGlobal(event_id, client, data, extra_data, extra_pointers);
	const int default_return = DispatchEventPlayer(event_id, client, data, extra_data, extra_pointers);

	if (local_return != 0) {
		return local_return;
	} else if (global_return != 0) {
		return global_return;
	} else if (default_return != 0) {
		return default_return;
	}

	return 0;
}

int QuestParserCollection::EventPlayerLocal(
	QuestEventID event_id,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (_player_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByPlayerQuest(filename);

		if (qi) {
			_player_quest_status = qi->GetIdentifier();
			qi->LoadPlayerScript(filename);
			return qi->EventPlayer(event_id, client, data, extra_data, extra_pointers);
		}
	} else {
		if (_player_quest_status != QuestFailedToLoad) {
			auto iter = _interfaces.find(_player_quest_status);
			return iter->second->EventPlayer(event_id, client, data, extra_data, extra_pointers);
		}
	}

	return 0;
}

int QuestParserCollection::EventPlayerGlobal(
	QuestEventID event_id,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (_global_player_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByGlobalPlayerQuest(filename);

		if (qi) {
			_global_player_quest_status = qi->GetIdentifier();
			qi->LoadGlobalPlayerScript(filename);
			return qi->EventGlobalPlayer(event_id, client, data, extra_data, extra_pointers);
		}
	} else {
		if (_global_player_quest_status != QuestFailedToLoad) {
			auto iter = _interfaces.find(_global_player_quest_status);
			return iter->second->EventGlobalPlayer(event_id, client, data, extra_data, extra_pointers);
		}
	}

	return 0;
}

int QuestParserCollection::EventItem(
	QuestEventID event_id,
	Client* client,
	EQ::ItemInstance* inst,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (!inst) {
		return 0;
	}

	std::string item_script;
	if (inst->GetItem()->ScriptFileID != 0) {
		item_script = fmt::format(
			"script_{}",
			inst->GetItem()->ScriptFileID
		);
	} else if (strlen(inst->GetItem()->CharmFile) > 0) {
		item_script = inst->GetItem()->CharmFile;
	} else {
		item_script = std::to_string(inst->GetID());
	}

	uint32 item_id = inst->GetID();
	auto   iter    = _item_quest_status.find(item_id);
	if (iter != _item_quest_status.end()) {
		//loaded or failed to load
		if (iter->second != QuestFailedToLoad) {
			auto qiter = _interfaces.find(iter->second);
			int  ret   = DispatchEventItem(event_id, client, inst, mob, data, extra_data, extra_pointers);
			int  i     = qiter->second->EventItem(event_id, client, inst, mob, data, extra_data, extra_pointers);
			if (i != 0) {
				ret = i;
			}

			return ret;
		}

		return DispatchEventItem(event_id, client, inst, mob, data, extra_data, extra_pointers);
	} else if (_item_quest_status[item_id] != QuestFailedToLoad) {
		std::string filename;
		auto        qi = GetQIByItemQuest(item_script, filename);

		if (qi) {
			_item_quest_status[item_id] = qi->GetIdentifier();

			qi->LoadItemScript(filename, inst);

			int ret = DispatchEventItem(
				event_id,
				client,
				inst,
				mob,
				data,
				extra_data,
				extra_pointers
			);

			int i = qi->EventItem(event_id, client, inst, mob, data, extra_data, extra_pointers);
			if (i != 0) {
				ret = i;
			}

			return ret;
		} else {
			_item_quest_status[item_id] = QuestFailedToLoad;
			return DispatchEventItem(event_id, client, inst, mob, data, extra_data, extra_pointers);
		}
	}

	return 0;
}

int QuestParserCollection::EventSpell(
	QuestEventID event_id,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	auto iter = _spell_quest_status.find(spell_id);
	if (iter != _spell_quest_status.end()) {
		//loaded or failed to load
		if (iter->second != QuestFailedToLoad) {
			auto qi  = _interfaces.find(iter->second);
			int  ret = DispatchEventSpell(event_id, mob, client, spell_id, data, extra_data, extra_pointers);
			int  i   = qi->second->EventSpell(event_id, mob, client, spell_id, data, extra_data, extra_pointers);
			if (i != 0) {
				ret = i;
			}

			return ret;
		}

		return DispatchEventSpell(event_id, mob, client, spell_id, data, extra_data, extra_pointers);
	} else if (_spell_quest_status[spell_id] != QuestFailedToLoad) {
		std::string filename;
		auto        qi = GetQIBySpellQuest(spell_id, filename);

		if (qi) {
			_spell_quest_status[spell_id] = qi->GetIdentifier();

			qi->LoadSpellScript(filename, spell_id);

			int ret = DispatchEventSpell(
				event_id,
				mob,
				client,
				spell_id,
				data,
				extra_data,
				extra_pointers
			);

			int i = qi->EventSpell(
				event_id,
				mob,
				client,
				spell_id,
				data,
				extra_data,
				extra_pointers
			);
			if (i != 0) {
				ret = i;
			}

			return ret;
		} else {
			_spell_quest_status[spell_id] = QuestFailedToLoad;
			return DispatchEventSpell(event_id, mob, client, spell_id, data, extra_data, extra_pointers);
		}
	}

	return 0;
}

int QuestParserCollection::EventEncounter(
	QuestEventID event_id,
	std::string encounter_name,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	auto iter = _encounter_quest_status.find(encounter_name);
	if (iter != _encounter_quest_status.end()) {
		if (iter->second != QuestFailedToLoad) { // Loaded or failed to load
			auto qiter = _interfaces.find(iter->second);
			return qiter->second->EventEncounter(event_id, encounter_name, data, extra_data, extra_pointers);
		}
	} else if (_encounter_quest_status[encounter_name] != QuestFailedToLoad) {
		std::string filename;
		auto        qi = GetQIByEncounterQuest(encounter_name, filename);

		if (qi) {
			_encounter_quest_status[encounter_name] = qi->GetIdentifier();
			qi->LoadEncounterScript(filename, encounter_name);
			return qi->EventEncounter(event_id, encounter_name, data, extra_data, extra_pointers);
		} else {
			_encounter_quest_status[encounter_name] = QuestFailedToLoad;
		}
	}

	return 0;
}

int QuestParserCollection::EventBot(
	QuestEventID event_id,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	const int local_return   = EventBotLocal(event_id, bot, init, data, extra_data, extra_pointers);
	const int global_return  = EventBotGlobal(event_id, bot, init, data, extra_data, extra_pointers);
	const int default_return = DispatchEventBot(event_id, bot, init, data, extra_data, extra_pointers);

	if (local_return != 0) {
		return local_return;
	} else if (global_return != 0) {
		return global_return;
	} else if (default_return != 0) {
		return default_return;
	}

	return 0;
}

int QuestParserCollection::EventBotLocal(
	QuestEventID event_id,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (_bot_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByBotQuest(filename);

		if (qi) {
			_bot_quest_status = qi->GetIdentifier();
			qi->LoadBotScript(filename);
			return qi->EventBot(event_id, bot, init, data, extra_data, extra_pointers);
		}
	} else {
		if (_bot_quest_status != QuestFailedToLoad) {
			auto iter = _interfaces.find(_bot_quest_status);
			return iter->second->EventBot(event_id, bot, init, data, extra_data, extra_pointers);
		}
	}

	return 0;
}

int QuestParserCollection::EventBotGlobal(
	QuestEventID event_id,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (_global_bot_quest_status == QuestUnloaded) {
		std::string filename;
		auto        qi = GetQIByGlobalBotQuest(filename);

		if (qi) {
			_global_bot_quest_status = qi->GetIdentifier();
			qi->LoadGlobalBotScript(filename);
			return qi->EventGlobalBot(event_id, bot, init, data, extra_data, extra_pointers);
		}
	} else {
		if (_global_bot_quest_status != QuestFailedToLoad) {
			auto iter = _interfaces.find(_global_bot_quest_status);
			return iter->second->EventGlobalBot(event_id, bot, init, data, extra_data, extra_pointers);
		}
	}

	return 0;
}

QuestInterface* QuestParserCollection::GetQIByNPCQuest(uint32 npc_id, std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	const auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (!npc_type && npc_id != ZONE_CONTROLLER_NPC_ID) {
		return nullptr;
	}

	std::string npc_name = npc_id == ZONE_CONTROLLER_NPC_ID ? "zone_controller" : npc_type->name;

	Strings::FindReplace(npc_name, "`", "-");

	const std::string& global_path = fmt::format(
		"{}/{}",
		path.GetQuestsPath(),
		QUEST_GLOBAL_DIRECTORY
	);

	const std::string& zone_path = fmt::format(
		"{}/{}",
		path.GetQuestsPath(),
		zone->GetShortName()
	);

	std::vector<std::string> file_names = {
		fmt::format("{}/{}", zone_path, npc_id), // Local by NPC ID
		fmt::format("{}/{}", zone_path, npc_name), // Local by NPC Name
		fmt::format("{}/{}", global_path, npc_id), // Global by NPC ID
		fmt::format("{}/{}", global_path, npc_name), // Global by NPC ID
		fmt::format("{}/default", zone_path), // Zone Default
		fmt::format("{}/default", global_path), // Global Default
	};

	std::string file_name;

	for (auto & file : file_names) {
		for (auto* e: _load_precedence) {
			file_name = fmt::format(
				"{}.{}",
				file,
				_extensions.find(e->GetIdentifier())->second
			);

			if (File::Exists(file_name)) {
				filename = file_name;
				return e;
			}
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByPlayerQuest(std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	const std::string& global_path = fmt::format(
		"{}/{}",
		path.GetQuestsPath(),
		QUEST_GLOBAL_DIRECTORY
	);

	const std::string& zone_path = fmt::format(
		"{}/{}",
		path.GetQuestsPath(),
		zone->GetShortName()
	);

	std::vector<std::string> file_names = {
		fmt::format("{}/player_v{}", zone_path, zone->GetInstanceVersion()), // Local by Instance Version
		fmt::format("{}/player", zone_path), // Local
		fmt::format("{}/player", global_path) // Global
	};

	std::string file_name;
	for (auto & file : file_names) {
		for (auto* e: _load_precedence) {
			file_name = fmt::format(
				"{}.{}",
				file,
				_extensions.find(e->GetIdentifier())->second
			);

			if (File::Exists(file_name)) {
				filename = file_name;
				return e;
			}
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByGlobalNPCQuest(std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	std::string file_name;

	for (auto* e: _load_precedence) {
		file_name = fmt::format(
			"{}/{}/global_npc.{}",
			path.GetQuestsPath(),
			QUEST_GLOBAL_DIRECTORY,
			_extensions.find(e->GetIdentifier())->second
		);

		if (File::Exists(file_name)) {
			filename = file_name;
			return e;
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByGlobalPlayerQuest(std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	std::string file_name;
	for (auto* e: _load_precedence) {
		file_name = fmt::format(
			"{}/{}/global_player.{}",
			path.GetQuestsPath(),
			QUEST_GLOBAL_DIRECTORY,
			_extensions.find(e->GetIdentifier())->second
		);

		if (File::Exists(file_name)) {
			filename = file_name;
			return e;
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIBySpellQuest(uint32 spell_id, std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	const std::string& global_path = fmt::format(
		"{}/{}/spells",
		path.GetQuestsPath(),
		QUEST_GLOBAL_DIRECTORY
	);

	const std::string& zone_path = fmt::format(
		"{}/{}/spells",
		path.GetQuestsPath(),
		zone->GetShortName()
	);

	std::vector<std::string> file_names = {
		fmt::format("{}/{}", zone_path, spell_id), // Local
		fmt::format("{}/{}", global_path, spell_id), // Global
		fmt::format("{}/default", zone_path), // Local Default
		fmt::format("{}/default", global_path) // Global Default
	};

	std::string file_name;
	for (auto & file : file_names) {
		for (auto* e: _load_precedence) {
			file_name = fmt::format(
				"{}.{}",
				file,
				_extensions.find(e->GetIdentifier())->second
			);

			if (File::Exists(file_name)) {
				filename = file_name;
				return e;
			}
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByItemQuest(std::string item_script, std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	const std::string& global_path = fmt::format(
		"{}/{}/items",
		path.GetQuestsPath(),
		QUEST_GLOBAL_DIRECTORY
	);

	const std::string& zone_path = fmt::format(
		"{}/{}/items",
		path.GetQuestsPath(),
		zone->GetShortName()
	);

	std::vector<std::string> file_names = {
		fmt::format("{}/{}", zone_path, item_script), // Local
		fmt::format("{}/{}", global_path, item_script), // Global
		fmt::format("{}/default", zone_path), // Local Default
		fmt::format("{}/default", global_path) // Global Default
	};

	std::string file_name;
	for (auto & file : file_names) {
		for (auto* e: _load_precedence) {
			file_name = fmt::format(
				"{}.{}",
				file,
				_extensions.find(e->GetIdentifier())->second
			);

			if (File::Exists(file_name)) {
				filename = file_name;
				return e;
			}
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByEncounterQuest(std::string encounter_name, std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	const std::string& global_path = fmt::format(
		"{}/{}/encounters",
		path.GetQuestsPath(),
		QUEST_GLOBAL_DIRECTORY
	);

	const std::string& zone_path = fmt::format(
		"{}/{}/encounters",
		path.GetQuestsPath(),
		zone->GetShortName()
	);

	std::vector<std::string> file_names = {
		fmt::format("{}/{}", zone_path, encounter_name), // Local
		fmt::format("{}/{}", global_path, encounter_name) // Global
	};

	std::string file_name;
	for (auto & file : file_names) {
		for (auto* e: _load_precedence) {
			file_name = fmt::format(
				"{}.{}",
				file,
				_extensions.find(e->GetIdentifier())->second
			);

			if (File::Exists(file_name)) {
				filename = file_name;
				return e;
			}
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByBotQuest(std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	const std::string& global_path = fmt::format(
		"{}/{}",
		path.GetQuestsPath(),
		QUEST_GLOBAL_DIRECTORY
	);

	const std::string& zone_path = fmt::format(
		"{}/{}",
		path.GetQuestsPath(),
		zone->GetShortName()
	);

	std::vector<std::string> file_names = {
		fmt::format("{}/bot_v{}", zone_path, zone->GetInstanceVersion()), // Local by Instance Version
		fmt::format("{}/bot", zone_path), // Local
		fmt::format("{}/bot", global_path) // Global
	};

	std::string file_name;
	for (auto & file : file_names) {
		for (auto* e: _load_precedence) {
			file_name = fmt::format(
				"{}.{}",
				file,
				_extensions.find(e->GetIdentifier())->second
			);

			if (File::Exists(file_name)) {
				filename = file_name;
				return e;
			}
		}
	}

	return nullptr;
}

QuestInterface* QuestParserCollection::GetQIByGlobalBotQuest(std::string& filename)
{
	if (!zone || !zone->IsLoaded()) {
		return nullptr;
	}

	std::string file_name;
	for (auto* e: _load_precedence) {
		file_name = fmt::format(
			"{}/{}/global_bot.{}",
			path.GetQuestsPath(),
			QUEST_GLOBAL_DIRECTORY,
			_extensions.find(e->GetIdentifier())->second
		);

		if (File::Exists(file_name)) {
			filename = file_name;
			return e;
		}
	}

	return nullptr;
}

void QuestParserCollection::GetErrors(std::list<std::string>& quest_errors)
{
	quest_errors.clear();

	for (const auto& e: _load_precedence) {
		e->GetErrors(quest_errors);
	}
}

int QuestParserCollection::DispatchEventNPC(
	QuestEventID event_id,
	NPC* npc,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	int ret = 0;

	for (const auto& e: _load_precedence) {
		const int i = e->DispatchEventNPC(event_id, npc, init, data, extra_data, extra_pointers);
		if (i != 0) {
			ret = i;
		}
	}

	return ret;
}

int QuestParserCollection::DispatchEventPlayer(
	QuestEventID event_id,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	int ret = 0;

	for (const auto& e: _load_precedence) {
		const int i = e->DispatchEventPlayer(event_id, client, data, extra_data, extra_pointers);
		if (i != 0) {
			ret = i;
		}
	}

	return ret;
}

int QuestParserCollection::DispatchEventItem(
	QuestEventID event_id,
	Client* client,
	EQ::ItemInstance* inst,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	int ret = 0;

	for (const auto& e: _load_precedence) {
		const int i = e->DispatchEventItem(event_id, client, inst, mob, data, extra_data, extra_pointers);
		if (i != 0) {
			ret = i;
		}
	}

	return ret;
}

int QuestParserCollection::DispatchEventSpell(
	QuestEventID event_id,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	int ret = 0;

	for (const auto& e: _load_precedence) {
		const int i = e->DispatchEventSpell(event_id, mob, client, spell_id, data, extra_data, extra_pointers);
		if (i != 0) {
			ret = i;
		}
	}

	return ret;
}

int QuestParserCollection::DispatchEventBot(
	QuestEventID event_id,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	int ret = 0;

	for (const auto& e: _load_precedence) {
		int i = e->DispatchEventBot(event_id, bot, init, data, extra_data, extra_pointers);
		if (i != 0) {
			ret = i;
		}
	}

	return ret;
}

void QuestParserCollection::LoadPerlEventExportSettings(PerlEventExportSettings* s)
{
	for (int i = 0; i < _LargestEventID; i++) {
		s[i].qglobals        = 1;
		s[i].mob             = 1;
		s[i].zone            = 1;
		s[i].item            = 1;
		s[i].event_variables = 1;
	}

	const auto& l = PerlEventExportSettingsRepository::All(database);

	for (const auto& e: l) {
		s[e.event_id].qglobals        = e.export_qglobals;
		s[e.event_id].mob             = e.export_mob;
		s[e.event_id].zone            = e.export_zone;
		s[e.event_id].item            = e.export_item;
		s[e.event_id].event_variables = e.export_event;
	}

	LogInfo("Loaded [{}] Perl Event Export Settings", l.size());
}
