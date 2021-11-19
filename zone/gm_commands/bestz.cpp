#include "../client.h"
#include "../water_map.h"

void command_bestz(Client *c, const Seperator *sep)
{
	if (zone->zonemap == nullptr) {
		c->Message(Chat::White, "Map not loaded for this zone");
	}
	else {
		glm::vec3 me;
		me.x = c->GetX();
		me.y = c->GetY();
		me.z = c->GetZ() + (c->GetSize() == 0.0 ? 6 : c->GetSize()) * HEAD_POSITION;
		glm::vec3 hit;
		glm::vec3 bme(me);
		bme.z -= 500;

		float best_z = zone->zonemap->FindBestZ(me, &hit);

		if (best_z != BEST_Z_INVALID) {
			c->Message(Chat::White, "Z is %.3f at (%.3f, %.3f).", best_z, me.x, me.y);
		}
		else {
			c->Message(Chat::White, "Found no Z.");
		}
	}

	if (zone->watermap == nullptr) {
		c->Message(Chat::White, "Water Region Map not loaded for this zone");
	}
	else {
		WaterRegionType RegionType;
		float           z;

		if (c->GetTarget()) {
			z = c->GetTarget()->GetZ();
			auto position = glm::vec3(c->GetTarget()->GetX(), c->GetTarget()->GetY(), z);
			RegionType = zone->watermap->ReturnRegionType(position);
			c->Message(Chat::White, "InWater returns %d", zone->watermap->InWater(position));
			c->Message(Chat::White, "InLava returns %d", zone->watermap->InLava(position));

		}
		else {
			z = c->GetZ();
			auto position = glm::vec3(c->GetX(), c->GetY(), z);
			RegionType = zone->watermap->ReturnRegionType(position);
			c->Message(Chat::White, "InWater returns %d", zone->watermap->InWater(position));
			c->Message(Chat::White, "InLava returns %d", zone->watermap->InLava(position));

		}

		switch (RegionType) {
			case RegionTypeNormal: {
				c->Message(Chat::White, "There is nothing special about the region you are in!");
				break;
			}
			case RegionTypeWater: {
				c->Message(Chat::White, "You/your target are in Water.");
				break;
			}
			case RegionTypeLava: {
				c->Message(Chat::White, "You/your target are in Lava.");
				break;
			}
			case RegionTypeVWater: {
				c->Message(Chat::White, "You/your target are in VWater (Icy Water?).");
				break;
			}
			case RegionTypePVP: {
				c->Message(Chat::White, "You/your target are in a pvp enabled area.");
				break;
			}
			case RegionTypeSlime: {
				c->Message(Chat::White, "You/your target are in slime.");
				break;
			}
			case RegionTypeIce: {
				c->Message(Chat::White, "You/your target are in ice.");
				break;
			}
			default:
				c->Message(Chat::White, "You/your target are in an unknown region type.");
		}
	}


}


