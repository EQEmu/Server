#ifndef CODE_SKILL_CAPS_H
#define CODE_SKILL_CAPS_H

#include "repositories/skill_caps_repository.h"
#include "types.h"
#include "classes.h"
#include "skills.h"

class SkillCaps {
public:
	inline void ClearSkillCaps() { m_skill_caps.clear(); }
	SkillCapsRepository::SkillCaps GetSkillCap(uint8 class_id, EQ::skills::SkillType skill_id, uint8 level);
	uint8 GetSkillTrainLevel(uint8 class_id, EQ::skills::SkillType skill_id, uint8 level);
	void LoadSkillCaps();
	void ReloadSkillCaps();
	static int32_t GetSkillCapMaxLevel(uint8 class_id, EQ::skills::SkillType skill_id);

	SkillCaps *SetContentDatabase(Database *db);
private:
	Database                                    *m_content_database{};
	std::map<uint64, SkillCapsRepository::SkillCaps> m_skill_caps = {};
};

extern SkillCaps skill_caps;


#endif //CODE_SKILL_CAPS_H
