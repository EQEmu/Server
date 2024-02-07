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
#include "../common/strings.h"
#include "../common/misc_functions.h"
#include "../common/repositories/titles_repository.h"
#include "../common/repositories/player_titlesets_repository.h"

#include "client.h"
#include "mob.h"

#include "titles.h"
#include "worldserver.h"

extern WorldServer worldserver;

TitleManager::TitleManager() {
}

bool TitleManager::LoadTitles()
{
	titles.clear();

	const auto& l = TitlesRepository::All(database);

	if (l.empty()) {
		return false;
	}

	for (const auto& e : l) {
		TitleEntry t;

		t.title_id        = e.id;
		t.skill_id        = static_cast<EQ::skills::SkillType>(e.skill_id);
		t.min_skill_value = e.min_skill_value;
		t.max_skill_value = e.max_skill_value;
		t.min_aa_points   = e.min_aa_points;
		t.max_aa_points   = e.max_aa_points;
		t.class_id        = e.class_;
		t.gender_id       = e.gender;
		t.character_id    = e.char_id;
		t.status          = e.status;
		t.item_id         = e.item_id;
		t.prefix          = e.prefix;
		t.suffix          = e.suffix;
		t.titleset        = e.title_set;

		titles.push_back(t);
	}

	LogInfo(
		"Loaded [{}] Title{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "s" : ""
	);

	return true;
}

EQApplicationPacket* TitleManager::MakeTitlesPacket(Client* c)
{
	std::vector<TitleEntry> available_titles;
	uint32 length = 4;
	for (const auto& t : titles) {
		if (!IsClientEligibleForTitle(c, t)) {
			continue;
		}

		available_titles.push_back(t);
		length += t.prefix.length() + t.suffix.length() + 6;
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

std::string TitleManager::GetPrefix(int title_id)
{
	if (!title_id) {
		return std::string();
	}

	for (const auto& title : titles) {
		if (title.title_id == title_id) {
			return title.prefix;
		}
	}

	return std::string();
}

std::string TitleManager::GetSuffix(int title_id)
{
	if (!title_id) {
		return std::string();
	}

	for (const auto& title : titles) {
		if (title.title_id == title_id) {
			return title.suffix;
		}
	}

	return std::string();
}

bool TitleManager::HasTitle(Client* c, uint32 title_id)
{
	if (!c || !title_id) {
		return false;
	}

	for (const auto& t : titles) {
		if (t.title_id == title_id) {
			return IsClientEligibleForTitle(c, t);
		}
	}

	return false;
}

bool TitleManager::IsClientEligibleForTitle(Client* c, TitleEntry t)
{
	if (!c) {
		return false;
	}

	if (t.character_id >= 0 && c->CharacterID() != static_cast<uint32>(t.character_id)) {
		return false;
	}

	if (t.status >= 0 && c->Admin() < t.status) {
		return false;
	}

	if (t.gender_id >= Gender::Male && c->GetBaseGender() != t.gender_id) {
		return false;
	}

	if (t.class_id >= Class::None && c->GetBaseClass() != t.class_id) {
		return false;
	}

	if (t.min_aa_points >= 0 && c->GetSpentAA() < t.min_aa_points) {
		return false;
	}

	if (t.max_aa_points >= 0 && c->GetSpentAA() > t.max_aa_points) {
		return false;
	}

	if (t.skill_id >= 0) {
		auto skill_id = static_cast<EQ::skills::SkillType>(t.skill_id);
		if (t.min_skill_value >= 0 && c->GetRawSkill(skill_id) < static_cast<uint32>(t.min_skill_value)) {
			return false;
		}

		if (t.max_skill_value >= 0 && c->GetRawSkill(skill_id) > static_cast<uint32>(t.max_skill_value)) {
			return false;
		}
	}

	if (t.item_id >= 1 && c->GetInv().HasItem(t.item_id) == INVALID_INDEX) {
		return false;
	}

	if (t.titleset > 0 && !c->CheckTitle(t.titleset)) {
		return false;
	}

	return true;
}

bool TitleManager::IsNewAATitleAvailable(int aa_points, int class_id)
{
	for (const auto& t : titles) {
		if (
			(t.class_id == -1 || t.class_id == class_id) &&
			t.min_aa_points == aa_points
		) {
			return true;
		}
	}

	return false;
}

bool TitleManager::IsNewTradeSkillTitleAvailable(int skill_id, int skill_value)
{
	for (const auto& t : titles) {
		if (t.skill_id == skill_id && t.min_skill_value == skill_value) {
			return true;
		}
	}

	return false;
}

void TitleManager::CreateNewPlayerTitle(Client* c, std::string title)
{
	if (!c || title.empty()) {
		return;
	}

	c->SetAATitle(title);

	const auto& l = TitlesRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `prefix` = '{}'",
			c->CharacterID(),
			Strings::Escape(title)
		)
	);

	if (!l.empty()) {
		return;
	}

	auto e = TitlesRepository::NewEntity();

	e.char_id = c->CharacterID();
	e.prefix  = title;

	e = TitlesRepository::InsertOne(database, e);

	if (!e.id) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void TitleManager::CreateNewPlayerSuffix(Client* c, std::string suffix)
{
	if (!c || suffix.empty()) {
		return;
	}

	c->SetTitleSuffix(suffix);

	const auto& l = TitlesRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `suffix` = '{}'",
			c->CharacterID(),
			Strings::Escape(suffix)
		)
	);

	if (!l.empty()) {
		return;
	}

	auto e = TitlesRepository::NewEntity();

	e.char_id = c->CharacterID();
	e.suffix  = suffix;

	e = TitlesRepository::InsertOne(database, e);

	if (!e.id) {
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

	auto e = PlayerTitlesetsRepository::NewEntity();

	e.char_id   = CharacterID();
	e.title_set = title_set;

	e = PlayerTitlesetsRepository::InsertOne(database, e);

	if (!e.id) {
		LogError("Failed to insert titleset [{}] charid [{}]", title_set, CharacterID());
	}

}

bool Client::CheckTitle(int title_set)
{
	const auto& l = PlayerTitlesetsRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `title_set` = {} LIMIT 1",
			CharacterID(),
			title_set
		)
	);

	return !l.empty();
}

void Client::RemoveTitle(int title_set)
{
	if (!CheckTitle(title_set)) {
		return;
	}

	TitlesRepository::DeleteWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `title_set` = {}",
			CharacterID(),
			title_set
		)
	);
}
