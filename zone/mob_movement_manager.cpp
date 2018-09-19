#include "mob_movement_manager.h"
#include "client.h"
#include "mob.h"
#include "../common/timer.h"

#include <vector>
#include <stdlib.h>

struct MobMovementEntry
{
	Mob *m;
	int animation;
	float heading;
	bool dirty;
	double last_sent_time;
	double last_sent_time_long_distance;

	MobMovementEntry(Mob *m) {
		this->m = m;
		animation = 0;
		heading = m->GetHeading();
		dirty = false;
		last_sent_time = 0.0;
		last_sent_time_long_distance = 0.0;
	}
};

struct MobMovementManager::Implementation
{
	std::vector<MobMovementEntry> Entries;
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

	for (auto &ent : _impl->Entries) {
		if (ent.dirty) {
			SendUpdate(ent.m, ent.animation, ent.heading);

			ent.dirty = false;
			ent.last_sent_time = current_time;
			ent.last_sent_time_long_distance = current_time;
		} else if (ent.animation != 0) {
			double diff_short_range = current_time - ent.last_sent_time;
			double diff_long_range = current_time - ent.last_sent_time_long_distance;

			if (diff_short_range >= 2.0) {
				SendUpdateShortDistance(ent.m, ent.animation, ent.heading);
				ent.last_sent_time = current_time;
			}

			if (diff_long_range >= 6.0) {
				SendUpdateLongDistance(ent.m, ent.animation, ent.heading);
				ent.last_sent_time_long_distance = current_time;
			}
		}
	}
}

void MobMovementManager::AddMob(Mob *m)
{
	_impl->Entries.push_back(MobMovementEntry(m));
}

void MobMovementManager::RemoveMob(Mob *m)
{
	auto iter = _impl->Entries.begin();
	while (iter != _impl->Entries.end()) {
		auto &ent = *iter;
		if (ent.m == m) {
			_impl->Entries.erase(iter);
			return;
		}
		++iter;
	}
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
	auto iter = _impl->Entries.begin();
	while (iter != _impl->Entries.end()) {
		auto &ent = *iter;
		
		if (ent.m == who) {
			auto anim = 0;
			auto heading = who->GetHeading();
			
			if (ent.animation != anim || !HeadingEqual(ent.heading, heading)) {
				ent.animation = anim;
				ent.heading = heading;
				ent.dirty = true;
			}

			return;
		}

		++iter;
	}
}

void MobMovementManager::SendPositionUpdate(Mob *who, bool send_to_self)
{
	auto iter = _impl->Entries.begin();
	while (iter != _impl->Entries.end()) {
		auto &ent = *iter;

		if (ent.m == who) {
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

			return;
		}

		++iter;
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
