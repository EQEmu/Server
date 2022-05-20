#include "../client.h"

void command_showskills(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 start_skill_id = 0;
	if (sep->IsNumber(1)) {
		start_skill_id = std::stoul(sep->arg[1]);
	}

	bool show_all = !strcasecmp(sep->arg[2], "all");

	uint32 max_skill_id = (start_skill_id + 49);

	std::string popup_text = "<table>";

	popup_text += "<tr><td>ID</td><td>Name</td><td>Current</td><td>Max</td><td>Raw</td></tr>";

	for (
		EQ::skills::SkillType skill_id = (EQ::skills::SkillType) start_skill_id;
		skill_id <= (EQ::skills::SkillType) max_skill_id;
		skill_id = (EQ::skills::SkillType) (skill_id + 1)
	) {
		if ((EQ::skills::SkillType) skill_id >= EQ::skills::SkillCount) {
			max_skill_id = (EQ::skills::SkillCount - 1);
			break;
		}

		if (show_all || (target->CanHaveSkill(skill_id) && target->MaxSkill(skill_id))) {
			popup_text += fmt::format(
				"<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>",
				skill_id,
				EQ::skills::GetSkillName(skill_id),
				target->GetSkill(skill_id),
				target->MaxSkill(skill_id),
				target->GetRawSkill(skill_id)
			);
		}
	}

	popup_text += "</table>";

	std::string popup_title = fmt::format(
		"Skills for {} [{} to {}]",
		c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
		start_skill_id,
		max_skill_id
	);

	c->SendPopupToClient(
		popup_title.c_str(),
		popup_text.c_str()
	);
	

	c->Message(
		Chat::White,
		fmt::format(
			"Viewing skill levels from {} ({}) to {} ({}) for {}.",
			EQ::skills::GetSkillName((EQ::skills::SkillType) start_skill_id),
			start_skill_id,
			EQ::skills::GetSkillName((EQ::skills::SkillType) max_skill_id),
			max_skill_id,
			c->GetTargetDescription(target)
		).c_str()
	);

	int next_skill_id = (max_skill_id + 1);
	if ((EQ::skills::SkillType) next_skill_id < EQ::skills::SkillCount) {
		auto next_list_string = fmt::format(
			"#showskills {}",
			next_skill_id
		);

		auto next_list_link = EQ::SayLinkEngine::GenerateQuestSaylink(
			next_list_string,
			false,
			next_list_string
		);

		auto next_list_all_string = fmt::format(
			"#showskills {} all",
			next_skill_id
		);

		auto next_list_all_link = EQ::SayLinkEngine::GenerateQuestSaylink(
			next_list_all_string,
			false,
			next_list_all_string
		);

		c->Message(
			Chat::White,
			fmt::format(
				"To view the next 50 skill levels, you can use {} or {} to show skills the player cannot normally have.",
				next_list_link,
				next_list_all_link
			).c_str()
		);
	}
}

