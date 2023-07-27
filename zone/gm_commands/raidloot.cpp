#include "../client.h"
#include "../groups.h"
#include "../raids.h"
#include "../raids.h"

void command_raidloot(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #raidloot [All|GroupLeader|RaidLeader|Selected]");
		return;
	}

	auto client_raid = c->GetRaid();
	if (!client_raid) {
		c->Message(Chat::White, "You must be in a Raid to use this command.");
		return;
	}

	if (!client_raid->IsLeader(c)) {
		c->Message(Chat::White, "You must be the Raid Leader to use this command.");
		return;
	}

	std::string raid_loot_type  = Strings::ToLower(sep->arg[1]);
	bool        is_all          = raid_loot_type.find("all") != std::string::npos;
	bool        is_group_leader = raid_loot_type.find("groupleader") != std::string::npos;
	bool        is_raid_leader  = raid_loot_type.find("raidleader") != std::string::npos;
	bool        is_selected     = raid_loot_type.find("selected") != std::string::npos;
	if (
		!is_all &&
		!is_group_leader &&
		!is_raid_leader &&
		!is_selected
		) {
		c->Message(Chat::White, "Usage: #raidloot [All|GroupLeader|RaidLeader|Selected]");
		return;
	}

	std::map<uint32, std::string> loot_types = {
		{RaidLootTypes::All,         "All"},
		{RaidLootTypes::GroupLeader, "GroupLeader"},
		{RaidLootTypes::RaidLeader,  "RaidLeader"},
		{RaidLootTypes::Selected,    "Selected"}
	};

	uint32 loot_type;
	if (is_all) {
		loot_type = RaidLootTypes::All;
	}
	else if (is_group_leader) {
		loot_type = RaidLootTypes::GroupLeader;
	}
	else if (is_raid_leader) {
		loot_type = RaidLootTypes::RaidLeader;
	}
	else if (is_selected) {
		loot_type = RaidLootTypes::Selected;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Loot type changed to {} ({}).",
			loot_types[loot_type],
			loot_type
		).c_str()
	);
}

void command_raiddelegates(Client* c, const Seperator* sep)
{
	
	
	auto raid = c->GetRaid();
	if (raid) {
		auto da1 = strlen(raid->main_assister_pcs[0]) > 0 ? entity_list.GetClientByName(raid->main_assister_pcs[0]) : nullptr;
		auto da2 = strlen(raid->main_assister_pcs[1]) > 0 ? entity_list.GetClientByName(raid->main_assister_pcs[1]) : nullptr;
		auto da3 = strlen(raid->main_assister_pcs[2]) > 0 ? entity_list.GetClientByName(raid->main_assister_pcs[2]) : nullptr;

		auto mm1 = strlen(raid->main_marker_pcs[0]) > 0 ? entity_list.GetClientByName(raid->main_marker_pcs[0]) : nullptr;
		auto mm2 = strlen(raid->main_marker_pcs[1]) > 0 ? entity_list.GetClientByName(raid->main_marker_pcs[1]) : nullptr;
		auto mm3 = strlen(raid->main_marker_pcs[2]) > 0 ? entity_list.GetClientByName(raid->main_marker_pcs[2]) : nullptr;



		/*
			<tr><td>Name< / td><td>Leader:{}< / td><td>Raid Delegate Info< / td><td>Raid id : {}< / td>< / tr>
				<tr><td>Delegate Type< / td><td>Player Name< / td><td>Target ID< / td><td>Target Name< / td>< / tr>
				<tr></tr>
				<tr><td>Delegate 1 < / td > <td>{}< / td><td>{}< / td><td>{}< / td>< / tr>
				<tr><td>Delegate 2 < / td > <td>{}< / td><td>{}< / td><td>{}< / td>< / tr>
				<tr><td>Delegate 3 < / td > <td>{}< / td><td>{}< / td><td>{}< / td>< / tr>
				<tr></tr>
				<tr><td>Marker 1 < / td > <td>{}< / td><td>{}< / td><td>{}< / td>< / tr>
				<tr><td>Marker 2 < / td > <td>{}< / td><td>{}< / td><td>{}< / td>< / tr>
				<tr><td>Marker 3 < / td > <td>{}< / td><td>{}< / td><td>{}< / td>< / tr>
		*/
		std::string popup_text = "<table>";
		popup_text += fmt::format(
			"<tr><td>Name</td><td>Leader:{}</td><td>Raid Delegate Info</td><td>Raid id:{}</td></tr>",
			raid->GetLeader()->GetCleanName(),
			raid->GetID()
		);
		popup_text += "<br>";

		popup_text += fmt::format(
			"<tr><td>Delegate 1</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			da1 ? da1->GetCleanName() : "Not Set",
			da1->GetTarget() ? da1->GetTarget()->GetID() : 0,
			da1->GetTarget() ? da1->GetTarget()->GetCleanName() : ""
		);
		popup_text += fmt::format(
			"<tr><td>Delegate 1</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			da2 ? da2->GetCleanName() : "Not Set",
			da2->GetTarget() ? da2->GetTarget()->GetID() : 0,
			da2->GetTarget() ? da2->GetTarget()->GetCleanName() : ""
		);
		popup_text += fmt::format(
			"<tr><td>Delegate 1</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			da3 ? da3->GetCleanName() : "Not Set",
			da3->GetTarget() ? da3->GetTarget()->GetID() : 0,
			da3->GetTarget() ? da3->GetTarget()->GetCleanName() : ""
		);

		popup_text += "<br>";

		popup_text += fmt::format(
			"<tr><td>Delegate 1</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			mm1 ? mm1->GetCleanName() : "Not Set",
			mm1->GetTarget() ? mm1->GetTarget()->GetID() : 0,
			mm1->GetTarget() ? mm1->GetTarget()->GetCleanName() : ""
		);
		popup_text += fmt::format(
			"<tr><td>Delegate 1</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			mm2 ? mm2->GetCleanName() : "Not Set",
			mm2->GetTarget() ? mm2->GetTarget()->GetID() : 0,
			mm2->GetTarget() ? mm2->GetTarget()->GetCleanName() : ""
		);
		popup_text += fmt::format(
			"<tr><td>Delegate 1</td><td>{}</td><td>{}</td><td>{}</td></tr>",
			mm3 ? mm3->GetCleanName() : "Not Set",
			mm3->GetTarget() ? mm3->GetTarget()->GetID() : 0,
			mm3->GetTarget() ? mm3->GetTarget()->GetCleanName() : ""
		);

		popup_text += "</table>";

		c->SendPopupToClient(
			"Raid Delegate Information",
			popup_text.c_str()
		);
	}
}
