#include "../client.h"

void command_proximity(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || (c->GetTarget() && !c->GetTarget()->IsNPC())) {
		c->Message(Chat::White, "You must target an NPC");
		return;
	}

	for (auto &iter : entity_list.GetNPCList()) {
		auto        npc  = iter.second;
		std::string name = npc->GetName();

		if (name.find("Proximity") != std::string::npos) {
			npc->Depop();
		}
	}

	NPC *npc = c->GetTarget()->CastToNPC();

	std::vector<FindPerson_Point> points;

	FindPerson_Point p{};

	if (npc->IsProximitySet()) {
		glm::vec4 position;
		position.w = npc->GetHeading();
		position.x = npc->GetProximityMinX();
		position.y = npc->GetProximityMinY();
		position.z = npc->GetZ();

		position.x = npc->GetProximityMinX();
		position.y = npc->GetProximityMinY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = npc->GetProximityMinX();
		position.y = npc->GetProximityMaxY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = npc->GetProximityMaxX();
		position.y = npc->GetProximityMinY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = npc->GetProximityMaxX();
		position.y = npc->GetProximityMaxY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		p.x = npc->GetProximityMinX();
		p.y = npc->GetProximityMinY();
		p.z = npc->GetZ();
		points.push_back(p);

		p.x = npc->GetProximityMinX();
		p.y = npc->GetProximityMaxY();
		points.push_back(p);

		p.x = npc->GetProximityMaxX();
		p.y = npc->GetProximityMaxY();
		points.push_back(p);

		p.x = npc->GetProximityMaxX();
		p.y = npc->GetProximityMinY();
		points.push_back(p);

		p.x = npc->GetProximityMinX();
		p.y = npc->GetProximityMinY();
		points.push_back(p);
	}

	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		c->SendPathPacket(points);
	}
}

