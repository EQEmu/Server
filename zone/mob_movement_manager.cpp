#include "mob_movement_manager.h"
#include "client.h"
#include "mob.h"
#include "../common/timer.h"

#include <vector>
#include <map>
#include <stdlib.h>

extern uint64_t frame_time;

struct MovementEntry
{
	int animation;
	float heading;
	bool dirty;
	double last_sent_time;
	double last_sent_time_long_distance;

	MovementEntry(Mob *m) {
		animation = 0;
		heading = m->GetHeading();
		dirty = false;
		last_sent_time = 0.0;
		last_sent_time_long_distance = 0.0;
	}
};

struct MoveToEntry
{
	float x;
	float y;
	float z;
	float speed;
	bool active;

	MoveToEntry() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		speed = 0.0f;
		active = false;
	}
};

struct MobMovementManager::Implementation
{
	std::map<Mob*, MovementEntry> Entries;
	std::map<Mob*, MoveToEntry> MoveEntries;
	std::vector<Client*> Clients;
};

MobMovementManager::MobMovementManager()
{
	_impl.reset(new Implementation());
}

MobMovementManager::~MobMovementManager()
{
}

void MobMovementManager::Process()
{
	double current_time = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;

	for (auto &mov : _impl->MoveEntries) {
		auto &ent = mov.second;

		if (ent.active) {
			ProcessMovement(mov.first, ent.x, ent.y, ent.z, ent.speed);
		}
	}

	for (auto &iter : _impl->Entries) {
		auto &ent = iter.second;
		if (ent.dirty) {
			SendUpdate(iter.first, ent.animation, ent.heading);

			ent.dirty = false;
			ent.last_sent_time = current_time;
			ent.last_sent_time_long_distance = current_time;
		} else if (ent.animation != 0) {
			double diff_short_range = current_time - ent.last_sent_time;
			double diff_long_range = current_time - ent.last_sent_time_long_distance;

			if (diff_short_range >= 2.0) {
				SendUpdateShortDistance(iter.first, ent.animation, ent.heading);
				ent.last_sent_time = current_time;
			}

			if (diff_long_range >= 6.0) {
				SendUpdateLongDistance(iter.first, ent.animation, ent.heading);
				ent.last_sent_time_long_distance = current_time;
			}
		}
	}
}

void MobMovementManager::AddMob(Mob *m)
{
	_impl->Entries.insert(std::make_pair(m, MovementEntry(m)));
	_impl->MoveEntries.insert(std::make_pair(m, MoveToEntry()));
}

void MobMovementManager::RemoveMob(Mob *m)
{
	_impl->Entries.erase(m);
	_impl->MoveEntries.erase(m);
}

void MobMovementManager::AddClient(Client *c)
{
	_impl->Clients.push_back(c);
}

void MobMovementManager::RemoveClient(Client *c)
{
	auto iter = _impl->Clients.begin();
	while (iter != _impl->Clients.end()) {
		if (c == *iter) {
			_impl->Clients.erase(iter);
			return;
		}
	
		++iter;
	}
}

void MobMovementManager::SendPosition(Mob *who)
{
	auto iter = _impl->Entries.find(who);
	auto &ent = iter->second;

	auto anim = 0;
	auto heading = who->GetHeading();
	
	if (ent.animation != anim || !HeadingEqual(ent.heading, heading)) {
		ent.animation = anim;
		ent.heading = heading;
		ent.dirty = true;
	}
}

void MobMovementManager::SendPositionUpdate(Mob *who, bool send_to_self)
{
	auto iter = _impl->Entries.find(who);
	auto &ent = iter->second;

	auto anim = 0;
	auto heading = who->GetHeading();
	if (who->IsMoving()) {
		if (who->IsClient()) {
			anim = who->GetAnimation();
		}
		else {
			anim = who->GetRunAnimSpeed();
		}
	}
	
	if (send_to_self && who->IsClient()) {
		SendUpdateTo(who, who->CastToClient(), anim, heading);
	}
	
	if (ent.animation != anim || !HeadingEqual(ent.heading, heading)) {
		ent.animation = anim;
		ent.heading = heading;
		ent.dirty = true;
	}
}

void MobMovementManager::NavigateTo(Mob *who, float x, float y, float z, float speed)
{
	auto iter = _impl->MoveEntries.find(who);
	auto &ent = iter->second;

	ent.x = x;
	ent.y = y;
	ent.z = z;
	ent.speed = speed;
	ent.active = true;
}

void MobMovementManager::StopNavigation(Mob *who) {
	auto iter = _impl->MoveEntries.find(who);
	auto &ent = iter->second;
	ent.active = false;

	SendPosition(who);
}

void MobMovementManager::Dump(Mob *m, Client *to)
{
	{
		auto iter = _impl->Entries.find(m);
		auto &ent = iter->second;

		to->Message(0, "Packet: anim=%d, heading=%.2f, dirty=%s, last_sent_time=%.2f, last_sent_time_long_dist=%.2f", 
			ent.animation, 
			ent.heading, 
			ent.dirty ? "true" : "false", 
			ent.last_sent_time, 
			ent.last_sent_time_long_distance);
	}

	{
		auto iter = _impl->MoveEntries.find(m);
		auto &ent = iter->second;

		to->Message(0, "Movement: speed=%.2f, x=%.2f, y=%.2f, z=%.2f, active=%s", 
			ent.speed, 
			ent.x, 
			ent.y, 
			ent.z, 
			ent.active ? "true" : "false");
	}
}

bool MobMovementManager::HeadingEqual(float a, float b)
{
	const float eps = 0.0001f;
	return abs(a - b) < eps;
}

void MobMovementManager::SendUpdateTo(Mob *who, Client *c, int anim, float heading)
{
	EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp.pBuffer;
	if (anim == 0) {
		who->MakeSpawnUpdateNoDelta(spu);
	}
	else {
		who->MakeSpawnUpdate(spu);
	}

	c->QueuePacket(&outapp);
}

void MobMovementManager::SendUpdate(Mob *who, int anim, float heading)
{
	EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp.pBuffer;
	if (anim == 0) {
		who->MakeSpawnUpdateNoDelta(spu);
	}
	else {
		who->MakeSpawnUpdate(spu);
	}
	
	for (auto &c : _impl->Clients) {
		if (c != who) {
			c->QueuePacket(&outapp);
		}
		else if (c->IsAIControlled()) {
			c->QueuePacket(&outapp);
		}
	}
}

void MobMovementManager::SendUpdateShortDistance(Mob *who, int anim, float heading)
{
	EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp.pBuffer;
	if (anim == 0) {
		who->MakeSpawnUpdateNoDelta(spu);
	}
	else {
		who->MakeSpawnUpdate(spu);
	}

	for (auto &c : _impl->Clients) {
		if (c != who) {
			auto dist = who->CalculateDistance(c->GetX(), c->GetY(), c->GetZ());
			if (dist <= 1000.0f) {
				c->QueuePacket(&outapp);
			}
		}
		else if (c->IsAIControlled()) {
			auto dist = who->CalculateDistance(c->GetX(), c->GetY(), c->GetZ());
			if (dist <= 1000.0f) {
				c->QueuePacket(&outapp);
			}
		}
	}
}

void MobMovementManager::SendUpdateLongDistance(Mob *who, int anim, float heading)
{
	EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp.pBuffer;
	if (anim == 0) {
		who->MakeSpawnUpdateNoDelta(spu);
	}
	else {
		who->MakeSpawnUpdate(spu);
	}

	for (auto &c : _impl->Clients) {
		if (c != who) {
			auto dist = who->CalculateDistance(c->GetX(), c->GetY(), c->GetZ());
			if (dist > 1000.0f) {
				c->QueuePacket(&outapp);
			}
		}
		else if (c->IsAIControlled()) {
			auto dist = who->CalculateDistance(c->GetX(), c->GetY(), c->GetZ());
			if (dist > 1000.0f) {
				c->QueuePacket(&outapp);
			}
		}
	}
}

void MobMovementManager::ProcessMovement(Mob *who, float x, float y, float z, float speed)
{
	if (who->GetID() == 0) {
		return;
	}

	if (speed <= 0) {
		who->SetCurrentSpeed(0);
		return;
	}

	if (IsPositionEqual(x, y, z, who->GetX(), who->GetY(), who->GetZ())) {
		StopNavigation(who);
		return;
	}

	bool WaypointChanged = false;
	bool NodeReached = false;
	glm::vec3 Goal = who->UpdatePath(
		x, y, z, speed, WaypointChanged, NodeReached
	);

	if (WaypointChanged || NodeReached) {
		entity_list.OpenDoorsNear(who);
	}

	who->SetCurrentSpeed(static_cast<int>(speed));
	who->SetRunAnimSpeed(speed);

#ifdef BOTS
	if (who->IsClient() || who->IsBot())
#else
	if (who->IsClient())
#endif
	{
		who->SetAnimation(speed * 0.55f);
	}

	auto &p = who->GetPosition();

	//Setup Vectors
	glm::vec3 tar(Goal.x, Goal.y, Goal.z);
	glm::vec3 pos(p.x, p.y, p.z);
	double len = glm::distance(pos, tar);
	if (len == 0) {
		return;
	}

	glm::vec3 dir = tar - pos;
	glm::vec3 ndir = glm::normalize(dir);
	double time_since_last = static_cast<double>(frame_time) / 1000.0;
	double distance_moved = time_since_last * speed * 0.4f * 1.4f;

	if (distance_moved > len) {
		who->Teleport(Goal);

		if (who->IsNPC()) {
			entity_list.ProcessMove(who->CastToNPC(), Goal.x, Goal.y, Goal.z);
		}

		who->TryFixZ();
		return;
	}
	else {
		glm::vec3 npos = pos + (ndir * static_cast<float>(distance_moved));
		who->Teleport(npos);

		if (who->IsNPC()) {
			entity_list.ProcessMove(who->CastToNPC(), npos.x, npos.y, npos.z);
		}
	}

	who->SetHeading(who->CalculateHeadingToTarget(Goal.x, Goal.y));
	SendPositionUpdate(who, false);

	who->TryFixZ();

	who->SetMoving(true);

	if (who->IsClient()) {
		who->SendPositionUpdate();
		who->CastToClient()->ResetPositionTimer();
	}
	else {
		who->SendPositionUpdate();
		who->SetAppearance(eaStanding, false);
	}
}
