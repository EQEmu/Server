#include "../common/global_define.h"

#include "client.h"
#include "zone.h"

extern Zone *zone;

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
		PathingDestination = To;
		WaypointChanged = true;
		NodeReached = false;
		return (*Route.begin()).pos;
	}
	else {
		if (PathRecalcTimer->Check()) {
			bool SameDestination = DistanceSquared(To, PathingDestination) < 4.0f;
			if (!SameDestination) {
				//We had a route but our target position moved too much
				Route = zone->pathing->FindRoute(From, To);
				PathingDestination = To;
				WaypointChanged = true;
				NodeReached = false;
				return (*Route.begin()).pos;
			}
		}

		bool AtNextNode = DistanceSquared(From, (*Route.begin()).pos) < 4.0f;
		if (AtNextNode) {
			WaypointChanged = false;
			NodeReached = true;

			Route.pop_front();

			if (Route.empty()) {
				Route = zone->pathing->FindRoute(From, To);
				PathingDestination = To;
				WaypointChanged = true;
				return (*Route.begin()).pos;
			}
			else {
				auto node = *Route.begin();
				if (node.teleport) {
					//If is identity node then is teleport node.
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
