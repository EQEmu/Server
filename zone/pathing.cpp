#include "../common/global_define.h"

#include "client.h"
#include "doors.h"
#include "water_map.h"
#include "zone.h"
#include <math.h>

glm::vec3 Mob::UpdatePath(float ToX, float ToY, float ToZ, float Speed, bool &WaypointChanged)
{
	WaypointChanged = false;

	glm::vec3 from(GetX(), GetY(), GetZ());
	glm::vec3 to(ToX, ToY, ToZ);

	if (Speed <= 0)
		return to;

	if (to == from)
		return to;

	if (!m_pathing_route.Active() || !m_pathing_route.DestinationValid(to)) {
		m_pathing_route = zone->pathing.FindRoute(from, to);

		auto &nodes = m_pathing_route.GetNodesEdit();
		auto &last_node = nodes[nodes.size() - 1];
		auto dist = DistanceSquared(glm::vec4(last_node.position, 1.0f), glm::vec4(ToX, ToY, ToZ, 0.0f));
		if (dist > 10000.0f) {
			auto flag_temp = last_node.flag;
			last_node.flag = NavigationPolyFlagPortal;

			PathfindingNode end;
			end.position.x = ToX;
			end.position.y = ToY;
			end.position.z = ToZ;
			end.flag = flag_temp;
			nodes.push_back(end);
		}
		else if (dist > 100.0f) {
			PathfindingNode end;
			end.position.x = ToX;
			end.position.y = ToY;
			end.position.z = ToZ;
			end.flag = NavigationPolyFlagNormal;
			nodes.push_back(end);
		}
	}

	m_pathing_route.CalcCurrentNode(from, WaypointChanged);
	auto &current = m_pathing_route.GetCurrentNode();

	//We reached a wp and rolled over to a new wp dest and it was a portal wp so we portal to the next spot
	if (WaypointChanged) {
		if (m_pathing_route.GetPreviousNodeFlag() & NavigationPolyFlagPortal) {
			Teleport(current.position);
		}

		TrySnapToMap();
	}

	return current.position;
}

void Mob::TrySnapToMap() {
	bool snap = true;
	if (IsNPC()) {
		auto npc = CastToNPC();
		if (npc->GetFlyMode() == 1 || npc->GetFlyMode() == 2) {
			snap = false;
		}
	}
	
	if (snap && zone->HasMap()) {
		if (!RuleB(Watermap, CheckForWaterWhenMoving) || 
			!zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(glm::vec3(m_Position)))) {
			glm::vec3 dest(m_Position);
			float newz = zone->zonemap->FindBestZ(dest, nullptr) + 3.5f;
			m_Position.z = newz;
		}
	}
}
