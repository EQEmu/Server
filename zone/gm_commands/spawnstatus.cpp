#include "../client.h"

void command_spawnstatus(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #spawnstatus all - Show all spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus disabled - Show all disabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus enabled - Show all enabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus [Spawn ID] - Show spawn status by ID for your current zone");
		return;
	}

	bool is_all = !strcasecmp(sep->arg[1], "all");
	bool is_disabled = !strcasecmp(sep->arg[1], "disabled");
	bool is_enabled = !strcasecmp(sep->arg[1], "enabled");
	bool is_search = sep->IsNumber(1);

	if (
		!is_all &&
		!is_disabled &&
		!is_enabled &&
		!is_search
	) {
		c->Message(Chat::White, "Usage: #spawnstatus all - Show all spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus disabled - Show all disabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus enabled - Show all enabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #spawnstatus [Spawn ID] - Show spawn status by ID for your current zone");
		return;
	}

	std::string filter_type;
	if (is_disabled) {
		filter_type = "Disabled";
	} else if (is_enabled) {
		filter_type = "Enabled";
	}

	uint32 spawn_id = 0;	
	if (is_search) {
		spawn_id = std::stoul(sep->arg[1]);
	}

	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();

	uint32 filtered_count = 0;
	uint32 spawn_count = 0;
	uint32 spawn_number = 1;
	while (iterator.MoreElements()) {
		auto e = iterator.GetData();
		auto time_remaining = e->GetTimer().GetRemainingTime();
		if (
			is_all ||
			(
				is_disabled &&
				time_remaining == 0xFFFFFFFF
			) ||
			(
				is_enabled &&
				time_remaining != 0xFFFFFFFF
			) ||
			(
				is_search &&
				e->GetID() == spawn_id
			)
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spawn {} | ID: {} Coordinates: {:.2f}, {:.2f}, {:.2f}, {:.2f}",
					spawn_number,
					e->GetID(),
					e->GetX(),
					e->GetY(),
					e->GetZ(),
					e->GetHeading()
				).c_str()
			);
			if (time_remaining != 0xFFFFFFFF) {
				auto seconds_remaining = (time_remaining / 1000);
				c->Message(
					Chat::White,
					fmt::format(
						"Spawn {} | Respawn: {} ({} Second{})",
						spawn_number,
						ConvertSecondsToTime(seconds_remaining),
						seconds_remaining,
						seconds_remaining != 1 ? "s" : ""
					).c_str()
				);
			}
			filtered_count++;
			spawn_number++;
		}
		spawn_count++;
		iterator.Advance();
	}

	if (!spawn_count) {
		c->Message(Chat::White, "No spawns were found in this zone.");
		return;
	}

	if (!is_all && !is_search && !filtered_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"No {} spawns were found in this zone.",
				filter_type
			).c_str()
		);
		return;
	}
	
	if (is_all) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} spawn{} listed.",
				spawn_count,
				spawn_count != 1 ? "s" : ""
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} of {} spawn{} listed.",
				filtered_count,
				spawn_count,
				spawn_count != 1 ? "s" : ""
			).c_str()
		);
	}
}

