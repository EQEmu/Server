#include "../client.h"

void command_showzonepoints(Client *c, const Seperator *sep)
{
	auto      &mob_list = entity_list.GetMobList();
	for (auto itr : mob_list) {
		Mob *mob = itr.second;
		if (mob->IsNPC() && mob->GetRace() == 2254) {
			mob->Depop();
		}
	}

	int found_zone_points = 0;

	c->Message(Chat::White, "Listing zone points...");
	c->SendChatLineBreak();

	for (auto &virtual_zone_point : zone->virtual_zone_point_list) {
		std::string zone_long_name = ZoneLongName(virtual_zone_point.target_zone_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Virtual Zone Point x [{}] y [{}] z [{}] h [{}] width [{}] height [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				virtual_zone_point.x,
				virtual_zone_point.y,
				virtual_zone_point.z,
				virtual_zone_point.heading,
				virtual_zone_point.width,
				virtual_zone_point.height,
				zone_long_name.c_str(),
				virtual_zone_point.target_zone_id,
				virtual_zone_point.target_x,
				virtual_zone_point.target_y,
				virtual_zone_point.target_z,
				virtual_zone_point.target_heading
			).c_str()
		);

		std::string node_name = fmt::format("ZonePoint To [{}]", zone_long_name);

		float half_width = ((float) virtual_zone_point.width / 2);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x + half_width,
				(float) virtual_zone_point.y + half_width,
				virtual_zone_point.z,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x + half_width,
				(float) virtual_zone_point.y - half_width,
				virtual_zone_point.z,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x - half_width,
				(float) virtual_zone_point.y - half_width,
				virtual_zone_point.z,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x - half_width,
				(float) virtual_zone_point.y + half_width,
				virtual_zone_point.z,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x + half_width,
				(float) virtual_zone_point.y + half_width,
				(float) virtual_zone_point.z + (float) virtual_zone_point.height,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x + half_width,
				(float) virtual_zone_point.y - half_width,
				(float) virtual_zone_point.z + (float) virtual_zone_point.height,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x - half_width,
				(float) virtual_zone_point.y - half_width,
				(float) virtual_zone_point.z + (float) virtual_zone_point.height,
				virtual_zone_point.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) virtual_zone_point.x - half_width,
				(float) virtual_zone_point.y + half_width,
				(float) virtual_zone_point.z + (float) virtual_zone_point.height,
				virtual_zone_point.heading
			));

		found_zone_points++;
	}

	LinkedListIterator<ZonePoint *> iterator(zone->zone_point_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		ZonePoint   *zone_point    = iterator.GetData();
		std::string zone_long_name = ZoneLongName(zone_point->target_zone_id);
		std::string node_name      = fmt::format("ZonePoint To [{}]", zone_long_name);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				zone_point->x,
				zone_point->y,
				zone_point->z,
				zone_point->heading
			)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Client Side Zone Point x [{}] y [{}] z [{}] h [{}] number [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				zone_point->x,
				zone_point->y,
				zone_point->z,
				zone_point->heading,
				zone_point->number,
				zone_long_name.c_str(),
				zone_point->target_zone_id,
				zone_point->target_x,
				zone_point->target_y,
				zone_point->target_z,
				zone_point->target_heading
			).c_str()
		);

		iterator.Advance();

		found_zone_points++;
	}

	if (found_zone_points == 0) {
		c->Message(Chat::White, "There were no zone points found...");
	}

	c->SendChatLineBreak();

}

