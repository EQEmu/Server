#include "../common/eq_packet_structs.h"
#include "../common/strings.h"
#include "../common/misc_functions.h"
#include "../common/repositories/player_titlesets_repository.h"
#include "../common/repositories/titles_repository.h"

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
		titles.emplace_back(
			TitleEntry{
				.title_id = static_cast<int>(e.id),
				.skill_id = e.skill_id,
				.min_skill_value = e.min_skill_value,
				.max_skill_value = e.max_skill_value,
				.min_aa_points = e.max_aa_points,
				.max_aa_points = e.max_aa_points,
				.class_id = e.class_,
				.gender_id = e.gender,
				.character_id = e.char_id,
				.status = e.status,
				.item_id = e.item_id,
				.prefix = e.prefix,
				.suffix = e.suffix,
				.titleset = e.title_set
			}
		);
	}

	LogInfo("Loaded [{}] Title{}", Strings::Commify(l.size()), l.size() != 1 ? "s" : "");

	return true;
}

EQApplicationPacket* TitleManager::MakeTitlesPacket(Client* c)
{
	uint32 length = 4;

	const auto& eligible_titles = GetEligibleTitles(c);

	for (const auto& e : eligible_titles) {
		length += e.prefix.length() + e.suffix.length() + 6;
	}

	auto outapp   = new EQApplicationPacket(OP_SendTitleList, length);
	char * buffer = (char*) outapp->pBuffer;
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, eligible_titles.size());

	for (const auto& available_title : eligible_titles) {
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, available_title.title_id);
		VARSTRUCT_ENCODE_STRING(buffer, available_title.prefix.c_str());
		VARSTRUCT_ENCODE_STRING(buffer, available_title.suffix.c_str());
	}

	return outapp;
}

std::string TitleManager::GetPrefix(int title_id)
{
	if (!title_id) {
		return "";
	}

	auto e = std::find_if(
		titles.begin(),
		titles.end(),
		[title_id](const auto& title) {
			return title.title_id == title_id;
		}
	);

	return e != titles.end() ? e->prefix : "";
}

std::string TitleManager::GetSuffix(int title_id)
{
	if (!title_id) {
		return "";
	}

	auto e = std::find_if(
		titles.begin(),
		titles.end(),
		[title_id](const auto& title) {
			return title.title_id == title_id;
		}
	);

	return e != titles.end() ? e->suffix : "";
}

bool TitleManager::HasTitle(Client* c, uint32 title_id)
{
	if (!c || !title_id) {
		return false;
	}

	const auto& eligible_titles = GetEligibleTitles(c);

	return std::any_of(
		eligible_titles.begin(),
		eligible_titles.end(),
		[title_id](const auto& e) {
			return e.title_id == title_id;
		}
	);
}

const std::vector<TitleEntry>& TitleManager::GetEligibleTitles(Client* c)
{
	std::vector<TitleEntry> eligible_titles = {};
	if (!c) {
		return eligible_titles;
	}

	const auto& player_title_sets = PlayerTitlesetsRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			c->CharacterID()
		)
	);

	for (const auto& e : titles) {
		if (e.character_id >= 0 && c->CharacterID() != static_cast<uint32>(e.character_id)) {
			continue;
		}

		if (e.status >= 0 && c->Admin() < e.status) {
			continue;
		}

		if (e.gender_id >= Gender::Male && c->GetBaseGender() != e.gender_id) {
			continue;
		}

		if (e.class_id >= Class::None && c->GetBaseClass() != e.class_id) {
			continue;
		}

		if (e.min_aa_points >= 0 && c->GetSpentAA() < e.min_aa_points) {
			continue;
		}

		if (e.max_aa_points >= 0 && c->GetSpentAA() > e.max_aa_points) {
			continue;
		}

		if (e.skill_id >= 0) {
			auto skill_id = static_cast<EQ::skills::SkillType>(e.skill_id);
			if (
				e.min_skill_value >= 0 &&
				c->GetRawSkill(skill_id) < static_cast<uint32>(e.min_skill_value)
			) {
				continue;
			}

			if (
				e.max_skill_value >= 0 &&
				c->GetRawSkill(skill_id) > static_cast<uint32>(e.max_skill_value)
			) {
				continue;
			}
		}

		if (e.item_id >= 1 && c->GetInv().HasItem(e.item_id) == INVALID_INDEX) {
			continue;
		}

		if (
			e.titleset > 0 &&
			!std::any_of(
				player_title_sets.begin(),
				player_title_sets.end(),
				[](const auto& e) {
					return e.title_set == e.title_set;
				}
			)
		) {
			continue;
		}

		eligible_titles.emplace_back(e);
	}

	return eligible_titles;
}

bool TitleManager::IsNewAATitleAvailable(int aa_points, int class_id)
{
	return std::any_of(
		titles.begin(),
		titles.end(),
		[class_id, aa_points](const auto& title) {
			return (
				(title.class_id == -1 || title.class_id == class_id) &&
				title.min_aa_points == aa_points
			);
		}
	);
}

bool TitleManager::IsNewTradeSkillTitleAvailable(int skill_id, int skill_value)
{
	return std::any_of(
		titles.begin(),
		titles.end(),
		[skill_id, skill_value](const auto& title) {
			return title.skill_id == skill_id && title.min_skill_value == skill_value;
		}
	);
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

	if (!TitlesRepository::InsertOne(database, e).id) {
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

	if (!TitlesRepository::InsertOne(database, e).id) {
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
	auto strs   = (SetTitleReply_Struct*) outapp->pBuffer;

	strn0cpy(strs->title, title.c_str(), sizeof(strs->title));
	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
}

void Client::SetTitleSuffix(std::string suffix)
{
	strn0cpy(m_pp.suffix, suffix.c_str(), sizeof(m_pp.suffix));

	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
	auto strs   = (SetTitleReply_Struct*) outapp->pBuffer;

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

	if (!PlayerTitlesetsRepository::InsertOne(database, e).id) {
		LogError("Error in EnableTitle query for titleset [{}] and charid [{}]", title_set, CharacterID());
	}

}

bool Client::CheckTitle(int title_set)
{
	return !PlayerTitlesetsRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `title_set` = {}",
			CharacterID(),
			title_set
		)
	).empty();
}

void Client::RemoveTitle(int title_set)
{
	if (!CheckTitle(title_set)) {
		return;
	}

	for (const auto& title : title_manager.GetTitles()) {
		if (title.titleset == title_set) {
			if (std::string(m_pp.title) == title.prefix) {
				SetAATitle("");
			}

			if (std::string(m_pp.suffix) == title.suffix) {
				SetTitleSuffix("");
			}

			break;
		}
	}

	PlayerTitlesetsRepository::DeleteWhere(
		database,
		fmt::format(
			"`title_set` = {} AND `char_id` = {}",
			title_set,
			CharacterID()
		)
	);
}
