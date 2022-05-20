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
				"{} {} not in a group.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "are" : "is"
			).c_str()
		);
		return;
	}

	std::string popup_title = fmt::format(
		"Group Info for {}",
		c->GetTargetDescription(target, TargetDescriptionType::UCSelf)
	);
	std::string popup_text = "<table>";
	popup_text += fmt::format(
		"<tr><td>Group ID</td><td>{}</td><td>Members</td><td>{}</td>",
		target_group->GetID(),
		target_group->GroupCount()
	);
	popup_text += "<br><br>";
	popup_text += "<tr>";
	popup_text += "<td>Index</td>";
	popup_text += "<td>Name</td>";
	popup_text += "<td>In Zone</td>";
	popup_text += "<td>Assist</td>";
	popup_text += "<td>Puller</td>";
	popup_text += "<td>Tank</td>";
	popup_text += "</tr>";

	for (int group_member = 0; group_member < MAX_GROUP_MEMBERS; group_member++) {
		if (target_group->membername[group_member][0] == '\0') {
			continue;
		}
		
		bool is_assist = target_group->MemberRoles[group_member] & RoleAssist;
		bool is_puller = target_group->MemberRoles[group_member] & RolePuller;
		bool is_tank = target_group->MemberRoles[group_member] & RoleTank;

		popup_text += fmt::format(
			"<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			group_member,
			(
				strcmp(target_group->membername[group_member], c->GetCleanName()) ?
				target_group->membername[group_member] :
				fmt::format(
					"{} (You)",
					target_group->membername[group_member]
				)
			),
			target_group->members[group_member] ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>",
			is_assist ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>",
			is_puller ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>",
			is_tank ? "<c \"#00FF00\">✔</c>" : "<c \"#F62217\">❌</c>"
		);
	}

	popup_text += "</table>";

	c->SendPopupToClient(
		popup_title.c_str(),
		popup_text.c_str()
	);
}

