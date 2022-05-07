#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../corpse.h"

void command_summon(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments && !c->GetTarget()) {
		c->Message(Chat::White, "Usage: #summon - Summon your target, if you have one, to your position");
		c->Message(Chat::White, "Usage: #summon [Character Name] - Summon a character by name to your position");
		c->Message(Chat::White, "Note: You may also summon your target if you have one.");
		return;
	}

	Mob* target;

	if (arguments == 1) {
		std::string character_name = sep->arg[1];
		auto character_id = database.GetCharacterID(character_name.c_str());
		if (!character_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character '{}' does not exist.",
					character_name
				).c_str()
			);
			return;
		}

		auto search_client = entity_list.GetClientByName(character_name.c_str());
		if (search_client) {
			target = search_client->CastToMob();
		} else {
			if (!worldserver.Connected()) {
				c->Message(Chat::White, "World server is currently disconnected.");
				return;
			}

			auto pack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
			auto szp = (ServerZonePlayer_Struct *) pack->pBuffer;
			strn0cpy(szp->adminname, c->GetName(), sizeof(szp->adminname));
			szp->adminrank = c->Admin();
			szp->ignorerestrictions = 2;
			strn0cpy(szp->name, character_name.c_str(), sizeof(szp->name));
			strn0cpy(szp->zone, zone->GetShortName(), sizeof(szp->zone));
			szp->x_pos = c->GetX();
			szp->y_pos = c->GetY();
			szp->z_pos = c->GetZ();
			szp->instance_id = zone->GetInstanceID();
			worldserver.SendPacket(pack);
			safe_delete(pack);
			return;
		}
	} else if (c->GetTarget()) {
		target = c->GetTarget();
	}
	
	if (c == target) {
		c->Message(Chat::White, "You cannot summon yourself.");
		return;
	}
	
	c->Message(
		Chat::White,
		fmt::format(
			"Summoning {} to {:.2f}, {:.2f}, {:.2f} in {} ({}).",
			c->GetTargetDescription(target),
			c->GetX(),
			c->GetY(),
			c->GetZ(),
			zone->GetLongName(),
			zone->GetZoneID()
		).c_str()
	);

	if (target->IsClient()) {
		target->CastToClient()->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			c->GetX(),
			c->GetY(),
			c->GetZ(),
			c->GetHeading(),
			2,
			GMSummon
		);
		return;
	}

	target->GMMove(c->GetPosition());

	if (target->IsNPC()) {
		target->CastToNPC()->SaveGuardSpot(glm::vec4(0.0f));
	}
}

