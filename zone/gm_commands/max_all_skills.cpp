#include "../client.h"

void command_max_all_skills(Client *c, const Seperator *sep)
{
	if (c) {
		Client    *client_target = (c->GetTarget() ? (c->GetTarget()->IsClient() ? c->GetTarget()->CastToClient() : c)
			: c);
		auto      Skills         = EQ::skills::GetSkillTypeMap();
		for (auto &skills_iter : Skills) {
			auto skill_id            = skills_iter.first;
			auto current_skill_value = (
				(EQ::skills::IsSpecializedSkill(skill_id)) ?
					50 :
					content_db.GetSkillCap(client_target->GetClass(), skill_id, client_target->GetLevel())
			);
			client_target->SetSkill(skill_id, current_skill_value);
		}
	}
}

