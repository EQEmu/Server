#include "../common/global_define.h"
#include "../common/event/background_task.h"

#include "client.h"
#include "zone.h"
#include "water_map.h"

extern Zone *zone;

void AdjustRoute(std::list<IPathfinder::IPathNode> &nodes, int flymode, float offset) {
	if (!zone->HasMap() || !zone->HasWaterMap()) {
		return;
	}
	
	for (auto &node : nodes) {
		if (flymode == 0 || !zone->watermap->InLiquid(node.pos)) {
			auto best_z = zone->zonemap->FindBestZ(node.pos, nullptr);
			if (best_z != BEST_Z_INVALID) {
				node.pos.z = best_z + offset;
			}
		}
	}
}

glm::vec3 Mob::UpdatePath(float ToX, float ToY, float ToZ, float Speed, bool &WaypointChanged, bool &NodeReached)
{
	glm::vec3 To(ToX, ToY, ToZ);
	if (Speed <= 0) {
		return To;
	}

	glm::vec3 From(GetX(), GetY(), GetZ());

	if (DistanceSquared(To, From) < 1.0f) {
		WaypointChanged = false;
		NodeReached = true;
		Route.clear();
		return To;
	}

	if (Route.empty()) {
		bool partial = false;
		bool stuck = false;
		Route = zone->pathing->FindRoute(From, To, partial, stuck);
		AdjustRoute(Route, flymode, GetZOffset());

		PathingDestination = To;
		WaypointChanged = true;
		NodeReached = false;
		if (stuck) {
			return HandleStuckPath(To, From);
		}

		if (Route.empty()) {
			return To;
		}
		else {
			return (*Route.begin()).pos;
		}
	}
	else {
		if (PathRecalcTimer->Check()) {
			bool SameDestination = DistanceSquared(To, PathingDestination) < 100.0f;
			if (!SameDestination) {
				//We had a route but our target position moved too much
				bool partial = false;
				bool stuck = false;
				Route = zone->pathing->FindRoute(From, To, partial, stuck);
				AdjustRoute(Route, flymode, GetZOffset());

				PathingDestination = To;
				WaypointChanged = true;
				NodeReached = false;

				if (stuck) {
					return HandleStuckPath(To, From);
				}

				if (Route.empty()) {
					return To;
				}
				else {
					return (*Route.begin()).pos;
				}
			}
		}

		if (!IsRooted()) {
			bool AtPrevNode = DistanceSquared(From, PathingLastPosition) < 1.0f;
			if (AtPrevNode) {
				PathingLoopCount++;
				auto front = (*Route.begin()).pos;

				if (PathingLoopCount > 5) {
					Teleport(front);
					SendPosition();
					Route.pop_front();

					WaypointChanged = true;
					NodeReached = true;
					PathingLoopCount = 0;
				}

				return front;
			}
			else {
				PathingLastPosition = From;
				PathingLoopCount = 0;
			}
		}
		else {
			PathingLastPosition = From;
			PathingLoopCount = 0;
		}

		bool AtNextNode = false;
		if (flymode == 1) {
			AtNextNode = DistanceSquared(From, (*Route.begin()).pos) < 4.0f;
		}
		else {
			float z_dist = From.z - (*Route.begin()).pos.z;
			z_dist *= z_dist;
			AtNextNode = DistanceSquaredNoZ(From, (*Route.begin()).pos) < 4.0f && z_dist < 25.0f;
		}
		
		if (AtNextNode) {
			WaypointChanged = false;
			NodeReached = true;

			Route.pop_front();

			if (Route.empty()) {
				bool partial = false;
				bool stuck = false;
				Route = zone->pathing->FindRoute(From, To, partial, stuck);
				AdjustRoute(Route, flymode, GetZOffset());
				PathingDestination = To;
				WaypointChanged = true;

				if (stuck) {
					return HandleStuckPath(To, From);
				} 
				
				if(Route.empty()) {
					return To;
				}
				else {
					return (*Route.begin()).pos;
				}
			}
			else {
				auto node = *Route.begin();
				if (node.teleport) {
					Route.pop_front();

					if (Route.empty()) {
						return To;
					}

					auto nextNode = *Route.begin();

					Teleport(nextNode.pos);

					Route.pop_front();

					if (Route.empty()) {
						return To;
					}

					return (*Route.begin()).pos;
				}

				return node.pos;
			}
		}
		else {
			WaypointChanged = false;
			NodeReached = false;
			return (*Route.begin()).pos;
		}
	}

	return To;
}

glm::vec3 Mob::HandleStuckPath(const glm::vec3 &To, const glm::vec3 &From)
{
	bool partial = false;
	bool stuck = false;
	auto r = zone->pathing->FindRoute(To, From, partial, stuck);
	Route.clear();
	
	if (r.size() < 1) {
	Teleport(To);
	return To;
	}

	auto iter = r.rbegin();
	auto final_node = (*iter);
	Teleport(final_node.pos);

	if (r.size() < 2) {
		return final_node.pos;
	}
	else {
		iter++;
		return (*iter).pos;
	}
}

void CullPoints(std::vector<FindPerson_Point> &points) {
	if (!zone->HasMap()) {
		return;
	}

	size_t i = 0;
	for (; i < points.size(); ++i) {
		auto &p = points[i];

		for (;;) {
			if (i + 2 >= points.size()) {
				return;
			}

			if (points.size() < 36) {
				return;
			}

			auto &p1 = points[i + 1];
			auto &p2 = points[i + 2];

			if (zone->zonemap->CheckLoS(glm::vec3(p.x, p.y, p.z), glm::vec3(p2.x, p2.y, p2.z))) {
				points.erase(points.begin() + i + 1);
				Log(Logs::General, Logs::Status, "Culled find path point %u, connecting %u->%u instead.", i + 1, i, i + 2);
			}
			else {
				break;
			}
		}
	}
}

void Client::SendPathPacket(const std::vector<FindPerson_Point> &points) {
	EQ::BackgroundTask task([](EQEmu::Any &data) {
		auto &points = EQEmu::any_cast<std::vector<FindPerson_Point>&>(data);
		CullPoints(points);
	}, [this](EQEmu::Any &data) {
		auto &points = EQEmu::any_cast<std::vector<FindPerson_Point>&>(data);

		if (points.size() < 2) {
			if (Admin() > 10) {
				Message(MT_System, "Too few points");
			}

			EQApplicationPacket outapp(OP_FindPersonReply, 0);
			QueuePacket(&outapp);
			return;
		}

		if (points.size() > 36) {
			if (Admin() > 10) {
				Message(MT_System, "Too many points %u", points.size());
			}

			EQApplicationPacket outapp(OP_FindPersonReply, 0);
			QueuePacket(&outapp);
			return;
		}

		if (Admin() > 10) {
			Message(MT_System, "Total points %u", points.size());
		}

		int len = sizeof(FindPersonResult_Struct) + (points.size() + 1) * sizeof(FindPerson_Point);
		auto outapp = new EQApplicationPacket(OP_FindPersonReply, len);
		FindPersonResult_Struct* fpr = (FindPersonResult_Struct*)outapp->pBuffer;

		std::vector<FindPerson_Point>::iterator cur, end;
		cur = points.begin();
		end = points.end();
		unsigned int r;
		for (r = 0; cur != end; ++cur, r++) {
			fpr->path[r] = *cur;

		}
		//put the last element into the destination field
		--cur;
		fpr->path[r] = *cur;
		fpr->dest = *cur;

		FastQueuePacket(&outapp);
	}, points);
}
