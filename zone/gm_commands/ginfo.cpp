#include "../client.h"
#include "../groups.h"

void command_ginfo(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto target_group = target->GetGroup();
	if (!target_group) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} not in a group.",
				(
					c == target ?
					"You are" :
					fmt::format(
						"{} ({}) is",
						target->GetCleanName(),
						target->GetID()
					)
				)
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Group Info for {} | ID: {} Members: {}",
			(
				c == target ?
				"Yourself" :
				fmt::format(
					"{} ({})",
					target->GetCleanName(),
					target->GetID()
				)
			),
			target_group->GetID(),
			target_group->GroupCount()
		).c_str()
	);

	for (int group_member = 0; group_member < MAX_GROUP_MEMBERS; group_member++) {
		if (target_group->membername[group_member][0] == '\0') {
			continue;
		}
		
		int member_number = (group_member + 1);
		bool is_assist = target_group->MemberRoles[group_member] & RoleAssist;
		bool is_puller = target_group->MemberRoles[group_member] & RolePuller;
		bool is_tank = target_group->MemberRoles[group_member] & RoleTank;
		auto member_string = (
			strcmp(target_group->membername[group_member], c->GetCleanName()) ?
			(
				fmt::format(
					"Name: {} In Zone: {}",
					target_group->membername[group_member],
					target_group->members[group_member] ? "Yes" : "No"
				)
			) :
			"You"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Member {} | {}",
				member_number,
				member_string
			).c_str()
		);

		if (
			is_assist ||
			is_puller ||
			is_tank
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Member {} Roles | Assist: {} Puller: {} Tank: {}",
					member_number,
					is_assist ? "Yes" : "No",
					is_puller ? "Yes" : "No",
					is_tank ? "Yes" : "No"
				).c_str()
			);
		}
	}
}

