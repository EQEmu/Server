/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/eq_packet_structs.h"
#include "../common/string_util.h"
#include "../common/misc_functions.h"
#include "../common/repositories/titles_repository.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "titles.h"
#include "worldserver.h"

extern WorldServer worldserver;

TitleManager::TitleManager() {
}

bool TitleManager::LoadTitles()
{
	titles.clear();

	std::string query = "SELECT `id`, `skill_id`, `min_skill_value`, `max_skill_value`, "
						"`min_aa_points`, `max_aa_points`, `class`, `gender`, `char_id`, "
						"`status`, `item_id`, `prefix`, `suffix`, `title_set` FROM titles";
	auto results = database.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return false;
	}

	for (auto row : results) {
		TitleEntry title;
		title.title_id = std::stoi(row[0]);
		title.skill_id = (EQ::skills::SkillType) std::stoi(row[1]);
		title.min_skill_value = std::stoi(row[2]);
		title.max_skill_value = std::stoi(row[3]);
		title.min_aa_points = std::stoi(row[4]);
		title.max_aa_points = std::stoi(row[5]);
		title.class_id = std::stoi(row[6]);
		title.gender_id = std::stoi(row[7]);
		title.character_id = std::stoi(row[8]);
		title.status = std::stoi(row[9]);
		title.item_id = std::stoi(row[10]);
		title.prefix = row[11];
		title.suffix = row[12];
		title.titleset = std::stoi(row[13]);
		titles.push_back(title);
	}

	return true;
}

EQApplicationPacket *TitleManager::MakeTitlesPacket(Client *client)
{
	std::vector<TitleEntry> available_titles;
	uint32 length = 4;
	for (const auto& title : titles) {
		if (!IsClientEligibleForTitle(client, title)) {
			continue;
		}

		available_titles.push_back(title);
		length += title.prefix.length() + title.suffix.length() + 6;
	}

	auto outapp = new EQApplicationPacket(OP_SendTitleList, length);
	char *buffer = (char *)outapp->pBuffer;
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, available_titles.size());
	for (const auto& available_title : available_titles) {
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, available_title.title_id);
		VARSTRUCT_ENCODE_STRING(buffer, available_title.prefix.c_str());
		VARSTRUCT_ENCODE_STRING(buffer, available_title.suffix.c_str());
	}
	return(outapp);
}

int TitleManager::NumberOfAvailableTitles(Client *client)
{
	int count = 0;
	for (const auto& title : titles) {
		if (IsClientEligibleForTitle(client, title)) {
			++count;
		}
	}

	return count;
}

std::string TitleManager::GetPrefix(int title_id)
{
	if (!title_id) {
		return "";
	}

	for (const auto& title : titles) {
		if (title.title_id == title_id) {
			return title.prefix;
		}
	}

	return "";
}

std::string TitleManager::GetSuffix(int title_id)
{
	if (!title_id) {
		return "";
	}

	for (const auto& title : titles) {
		if (title.title_id == title_id) {
			return title.suffix;
		}
	}

	return "";
}

bool TitleManager::HasTitle(Client* client, uint32 title_id)
{
	if (!client || !title_id) {
		return false;
	}

	for (const auto& title : titles) {
		if (title.title_id == title_id) {
			return IsClientEligibleForTitle(client, title);
		}
	}

	return false;
}

bool TitleManager::IsClientEligibleForTitle(Client *client, TitleEntry title)
{
	if (!client) {
		return false;
	}

	if (title.character_id >= 0 && client->CharacterID() != static_cast<uint32>(title.character_id)) {
		return false;
	}

	if (title.status >= 0 && client->Admin() < title.status) {
		return false;
	}

	if (title.gender_id >= 0 && client->GetBaseGender() != title.gender_id) {
		return false;
	}

	if (title.class_id >= 0 && client->GetBaseClass() != title.class_id) {
		return false;
	}

	if (title.min_aa_points >= 0 && client->GetSpentAA() < title.min_aa_points) {
		return false;
	}

	if (title.max_aa_points >= 0 && client->GetSpentAA() > title.max_aa_points) {
		return false;
	}

	if (title.skill_id >= 0) {
		auto skill_id = static_cast<EQ::skills::SkillType>(title.skill_id);
		if (title.min_skill_value >= 0 && client->GetRawSkill(skill_id) < static_cast<uint32>(title.min_skill_value)) {
			return false;
		}

		if (title.max_skill_value >= 0 && client->GetRawSkill(skill_id) > static_cast<uint32>(title.max_skill_value)) {
			return false;
		}
	}

	if (title.item_id >= 1 && client->GetInv().HasItem(title.item_id) == INVALID_INDEX) {
		return false;
	}

	if (title.titleset > 0 && !client->CheckTitle(title.titleset)) {
		return false;
	}

	return true;
}

bool TitleManager::IsNewAATitleAvailable(int aa_points, int class_id)
{
	for (const auto& title : titles) {
		if (
			(title.class_id == -1 || title.class_id == class_id) &&
			title.min_aa_points == aa_points
		) {
			return true;
		}
	}

	return false;
}

bool TitleManager::IsNewTradeSkillTitleAvailable(int skill_id, int skill_value)
{
	for (const auto& title : titles) {
		if (title.skill_id == skill_id && title.min_skill_value == skill_value) {
			return true;
		}
	}

	return false;
}

void TitleManager::CreateNewPlayerTitle(Client *client, std::string title)
{
	if (!client || title.empty()) {
		return;
	}

	client->SetAATitle(title);

	auto query = fmt::format(
		"SELECT `id` FROM titles WHERE `prefix` = '{}' AND char_id = {}",
		EscapeString(title),
		client->CharacterID()
	);
	auto results = database.QueryDatabase(query);
	if (results.Success() && results.RowCount()){
		return;
	}

	query = fmt::format(
		"INSERT INTO titles (`char_id`, `prefix`) VALUES ({}, '{}')",
		client->CharacterID(),
		EscapeString(title)
	);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void TitleManager::CreateNewPlayerSuffix(Client *client, std::string suffix)
{
	if (!client || suffix.empty()) {
		return;
	}

	client->SetTitleSuffix(suffix);

	auto query = fmt::format(
		"SELECT `id` FROM titles WHERE `suffix` = '{}' AND char_id = {}",
		EscapeString(suffix),
		client->CharacterID()
	);
	auto results = database.QueryDatabase(query);
	if (results.Success() && results.RowCount()) {
		return;
	}

	query = fmt::format(
		"INSERT INTO titles (`char_id`, `suffix`) VALUES ({}, '{}')",
		client->CharacterID(),
		EscapeString(suffix)
	);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Client::SetAATitle(std::string title)
{
	strn0cpy(m_pp.title, title.c_str(), sizeof(m_pp.title));
	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
	auto strs = (SetTitleReply_Struct *) outapp->pBuffer;
	strn0cpy(strs->title, title.c_str(), sizeof(strs->title));
	strs->entity_id = GetID();
	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
}

void Client::SetTitleSuffix(std::string suffix)
{
	strn0cpy(m_pp.suffix, suffix.c_str(), sizeof(m_pp.suffix));
	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
	auto strs = (SetTitleReply_Struct *) outapp->pBuffer;
	strs->is_suffix = 1;
	strn0cpy(strs->title, suffix.c_str(), sizeof(strs->title));
	strs->entity_id = GetID();
	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
}

void Client::EnableTitle(int title_set)
{
	if (CheckTitle(title_set)) {
		return;
	}

	std::string query = fmt::format(
		"INSERT INTO player_titlesets (char_id, title_set) VALUES ({}, {})",
		CharacterID(),
		title_set
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in EnableTitle query for titleset [{}] and charid [{}]", title_set, CharacterID());
	}

}

bool Client::CheckTitle(int title_set)
{
	std::string query = fmt::format(
		"SELECT `id` FROM player_titlesets WHERE `title_set` = {} AND `char_id` = {} LIMIT 1",
		title_set,
		CharacterID()
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return false;
	}

	return true;
}

void Client::RemoveTitle(int title_set)
{
	if (!CheckTitle(title_set)) {
		return;
	}

	TitlesRepository::DeleteWhere(
		database,
		fmt::format(
			"`title_set` = {} AND `char_id` = {}",
			title_set,
			CharacterID()
		)
	);
}
