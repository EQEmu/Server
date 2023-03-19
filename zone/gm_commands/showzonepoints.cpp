#include "../client.h"

void command_showzonepoints(Client *c, const Seperator *sep)
{
	auto      &mob_list = entity_list.GetMobList();
	for (auto itr : mob_list) {
		Mob *mob = itr.second;
		if (mob->IsNPC() && mob->GetRace() == RACE_NODE_2254) {
			mob->Depop();
		}
	}

	int found_zone_points = 0;

	c->Message(Chat::White, "Listing zone points...");
	c->SendChatLineBreak();

	for (auto &p : zone->virtual_zone_point_list) {
		std::string zone_long_name = ZoneLongName(p.target_zone_id);

		std::string saylink = fmt::format(
			"#goto {:.0f} {:.0f} {:.0f}",
			p.x,
			p.y,
			p.z
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Virtual Zone Point [{}] x [{}] y [{}] z [{}] h [{}] width [{}] height [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				Saylink::Silent(saylink, "Goto").c_str(),
				p.x,
				p.y,
				p.z,
				p.heading,
				p.width,
				p.height,
				zone_long_name.c_str(),
				p.target_zone_id,
				p.target_x,
				p.target_y,
				p.target_z,
				p.target_heading
			).c_str()
		);

		std::string node_name = fmt::format("ZonePoint To [{}]", zone_long_name);

		float half_width = ((float) p.width / 2);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x + half_width,
				(float) p.y + half_width,
				p.z,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x + half_width,
				(float) p.y - half_width,
				p.z,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x - half_width,
				(float) p.y - half_width,
				p.z,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x - half_width,
				(float) p.y + half_width,
				p.z,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x + half_width,
				(float) p.y + half_width,
				(float) p.z + (float) p.height,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x + half_width,
				(float) p.y - half_width,
				(float) p.z + (float) p.height,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x - half_width,
				(float) p.y - half_width,
				(float) p.z + (float) p.height,
				p.heading
			));

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				(float) p.x - half_width,
				(float) p.y + half_width,
				(float) p.z + (float) p.height,
				p.heading
			));

		found_zone_points++;
	}

	LinkedListIterator<ZonePoint *> iterator(zone->zone_point_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		ZonePoint   *p             = iterator.GetData();
		std::string zone_long_name = ZoneLongName(p->target_zone_id);
		std::string node_name      = fmt::format("ZonePoint To [{}]", zone_long_name);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				p->x,
				p->y,
				p->z,
				p->heading
			)
		);

		//  {:.0f}

		std::string saylink = fmt::format(
			"#goto {:.0f} {:.0f} {:.0f}",
			p->x,
			p->y,
			p->z
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Client Side Zone Point [{}] x [{}] y [{}] z [{}] h [{}] number [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				Saylink::Silent(saylink, "Goto").c_str(),
				p->x,
				p->y,
				p->z,
				p->heading,
				p->number,
				zone_long_name.c_str(),
				p->target_zone_id,
				p->target_x,
				p->target_y,
				p->target_z,
				p->target_heading
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

