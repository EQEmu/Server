#include "skill_caps.h"

SkillCaps *SkillCaps::SetContentDatabase(Database *db)
{
	m_content_database = db;

	return this;
}

SkillCapsRepository::SkillCaps SkillCaps::GetSkillCap(uint8 class_id, EQ::skills::SkillType skill_id, uint8 level)
{
	if (!IsPlayerClass(class_id)) {
		return SkillCapsRepository::NewEntity();
	}

	auto pos = m_skill_caps.find(fmt::format("{}-{}-{}", class_id, level, skill_id));
	if (pos != m_skill_caps.end()) {
		return pos->second;
	}
	return SkillCapsRepository::NewEntity();
}

uint8 SkillCaps::GetTrainLevel(uint8 class_id, EQ::skills::SkillType skill_id, uint8 level)
{
	if (
		!IsPlayerClass(class_id) ||
		class_id > Class::PLAYER_CLASS_COUNT ||
		static_cast<uint32>(skill_id) > (EQ::skills::HIGHEST_SKILL + 1)
		) {
		return 0;
	}

	const uint8 skill_cap_max_level = (
		RuleI(Character, SkillCapMaxLevel) > 0 ?
			RuleI(Character, SkillCapMaxLevel) :
			RuleI(Character, MaxLevel)
	);

	const uint8 max_level = level > skill_cap_max_level ? level : skill_cap_max_level;

	for (const auto &e: m_skill_caps) {
		for (uint8 current_level = 1; current_level <= max_level; current_level++) {
			auto pos = m_skill_caps.find(fmt::format("{}-{}-{}", class_id, current_level, skill_id));
			if (pos != m_skill_caps.end()) {
				return current_level;
			}
		}
	}

	return 0;
}

void SkillCaps::LoadSkillCaps()
{
	const auto &l = SkillCapsRepository::All(*m_content_database);

	m_skill_caps.clear();

	for (const auto &e: l) {
		if (
			e.level < 1 ||
			!IsPlayerClass(e.class_id) ||
			static_cast<EQ::skills::SkillType>(e.skill_id) >= EQ::skills::SkillCount
			) {
			continue;
		}

		m_skill_caps[fmt::format("{}-{}-{}", e.class_id, e.level, e.skill_id)] = e;
	}

	LogInfo(
		"Loaded [{}] Skill Cap Entr{}",
		l.size(),
		l.size() != 1 ? "ies" : "y"
	);
}

void SkillCaps::ReloadSkillCaps()
{
	ClearSkillCaps();
	LoadSkillCaps();
}
