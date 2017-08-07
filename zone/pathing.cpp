#include "../common/global_define.h"

#include "client.h"
#include "zone.h"
#include "water_map.h"

extern Zone *zone;

void AdjustRoute(std::list<IPathfinder::IPathNode> &nodes, int flymode) {
	if (!zone->HasMap() || !zone->HasWaterMap()) {
		return;
	}

	for (auto &node : nodes) {
		if (flymode == 0 || !zone->watermap->InLiquid(node.pos)) {
			auto best_z = zone->zonemap->FindBestZ(node.pos, nullptr);
			if (best_z != BEST_Z_INVALID) {
				node.pos.z = best_z;
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
		Route = zone->pathing->FindRoute(From, To);
		AdjustRoute(Route, flymode);

		PathingDestination = To;
		WaypointChanged = true;
		NodeReached = false;
		if (Route.empty()) {
			return From;
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
				Route = zone->pathing->FindRoute(From, To);
				AdjustRoute(Route, flymode);

				PathingDestination = To;
				WaypointChanged = true;
				NodeReached = false;

				if (Route.empty()) {
					return From;
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

				if (PathingLoopCount > 5) {
					SendPosition();
				}

				auto front = (*Route.begin()).pos;
				Teleport(front);
				Route.pop_front();

				WaypointChanged = true;
				NodeReached = true;
				PathingLoopCount = 0;

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
				Route = zone->pathing->FindRoute(From, To);
				AdjustRoute(Route, flymode);
				PathingDestination = To;
				WaypointChanged = true;

				if (Route.empty()) {
					return From;
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
}

void Client::SendPathPacket(std::vector<FindPerson_Point> &points) {
	if (points.size() < 2) {
		//empty length packet == not found.
		EQApplicationPacket outapp(OP_FindPersonReply, 0);
		QueuePacket(&outapp);
		return;
	}

	if (points.size() > 36) {
		EQApplicationPacket outapp(OP_FindPersonReply, 0);
		QueuePacket(&outapp);
		return;
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
}
