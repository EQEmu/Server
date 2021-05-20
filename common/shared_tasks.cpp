#include "shared_tasks.h"
#include "repositories/character_data_repository.h"

std::vector<SharedTaskRequestMember> SharedTask::GetRequestMembers(Database &db, uint32 requestor_character_id)
{
	std::vector<SharedTaskRequestMember> request_members = {};

	// raid
	auto raid_characters = CharacterDataRepository::GetWhere(
		db,
		fmt::format(
			"id IN (select charid from raid_members where raidid = (select raidid from raid_members where charid = {}))",
			requestor_character_id
		)
	);

	if (!raid_characters.empty()) {
		request_members.reserve(raid_characters.size());
		for (auto &c: raid_characters) {
			SharedTaskRequestMember member = {};
			member.character_id   = c.id;
			member.character_name = c.name;
			member.is_raided      = true;
			member.level          = c.level;

			request_members.emplace_back(member);
		}

		return request_members;
	}

	// group
	auto group_characters = CharacterDataRepository::GetWhere(
		db,
		fmt::format(
			"id IN (select charid from group_id where groupid = (select groupid from group_id where charid = {}))",
			requestor_character_id
		)
	);

	if (!group_characters.empty()) {
		request_members.reserve(request_members.size());
		for (auto &c: group_characters) {
			SharedTaskRequestMember member = {};
			member.character_id   = c.id;
			member.character_name = c.name;
			member.is_grouped     = true;
			member.level          = c.level;

			request_members.emplace_back(member);
		}
	}

	return request_members;
}
