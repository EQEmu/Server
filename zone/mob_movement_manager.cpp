#include "mob_movement_manager.h"
#include "client.h"
#include "mob.h"
#include "../common/eq_packet_structs.h"
#include "../common/misc_functions.h"

#include <vector>
#include <map>
#include <stdlib.h>

extern double frame_time;

struct RotateCommand
{
	RotateCommand() {
		rotate_to = 0.0;
		rotate_to_speed = 0.0;
		rotate_dir = 1.0;
		active = false;
		started = false;
	}

	double rotate_to;
	double rotate_to_speed;
	double rotate_dir;
	bool active;
	bool started;
};

struct MovementCommand
{
	MovementCommand() {
		move_to_x = 0.0;
		move_to_y = 0.0;
		move_to_z = 0.0;
		move_to_speed = 0.0;
		last_sent_short_distance = 0.0;
		last_sent_medium_distance = 0.0;
		last_sent_long_distance = 0.0;
		active = false;
	}

	double move_to_x;
	double move_to_y;
	double move_to_z;
	double move_to_speed;
	double last_sent_short_distance;
	double last_sent_medium_distance;
	double last_sent_long_distance;
	bool active;
};

struct MovementStats
{
	MovementStats() {
		TotalSent = 0ULL;
		TotalSentMovement = 0ULL;
		TotalSentPosition = 0ULL;
		TotalSentHeading = 0ULL;
	}

	uint64_t TotalSent;
	uint64_t TotalSentMovement;
	uint64_t TotalSentPosition;
	uint64_t TotalSentHeading;
};

struct MobMovementEntry
{
	RotateCommand RotateCommand;
	MovementCommand MoveCommand;
};

struct MobMovementManager::Implementation
{
	std::map<Mob*, MobMovementEntry> Entries;
	std::vector<Client*> Clients;
	MovementStats Stats;
};

MobMovementManager::MobMovementManager()
{
	_impl.reset(new Implementation());
}

void MobMovementManager::ProcessRotateCommand(Mob *m, RotateCommand &cmd)
{
	if (m->IsEngaged()) {
		auto to = FixHeading(cmd.rotate_to);
		m->SetHeading(to);
		SendCommandToAllClients(m, 0.0, 0.0, 0.0, 0.0, 0);
		cmd.active = false;
		return;
	}

	if (!cmd.started) {
		cmd.started = true;

		SendCommandToAllClients(m, 0.0, 0.0, 0.0, cmd.rotate_dir * cmd.rotate_to_speed, 0);
	}

	auto from = FixHeading(m->GetHeading());
	auto to = FixHeading(cmd.rotate_to);
	float dist = 0.0;
	if (cmd.rotate_dir > 0.0) {
		if (to > from) {
			dist = to - from;
		}
		else {
			dist = 512.0 - from + to;
		}
	}
	else {
		if (from > to) {
			dist = from - to;
		}
		else {
			dist = (512.0 - to) + from;
		}
	}
	
	auto td = cmd.rotate_to_speed * 19.0 * frame_time;
	
	if (td >= dist) {
		m->SetHeading(to);
		SendCommandToAllClients(m, 0.0, 0.0, 0.0, 0.0, 0);
		cmd.active = false;
	}
	
	from += td * cmd.rotate_dir;
	m->SetHeading(FixHeading(from));
}

void MobMovementManager::SendCommandToAllClients(Mob *m, float dx, float dy, float dz, float dh, int anim)
{
	EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct *spu = (PlayerPositionUpdateServer_Struct*)outapp.pBuffer;
	FillCommandStruct(spu, m, dx, dy, dz, dh, anim);

	for (auto &c : _impl->Clients) {
		_impl->Stats.TotalSent++;

		if (anim != 0) {
			_impl->Stats.TotalSentMovement++;
		}
		else if (dh != 0) {
			_impl->Stats.TotalSentHeading++;
		}
		else {
			_impl->Stats.TotalSentPosition++;
		}

		c->QueuePacket(&outapp);
	}
}

void MobMovementManager::FillCommandStruct(PlayerPositionUpdateServer_Struct *spu, Mob *m, float dx, float dy, float dz, float dh, int anim)
{
	memset(spu, 0x00, sizeof(PlayerPositionUpdateServer_Struct));
	spu->spawn_id = m->GetID();
	spu->x_pos = FloatToEQ19(m->GetX());
	spu->y_pos = FloatToEQ19(m->GetY());
	spu->z_pos = FloatToEQ19(m->GetZ());
	spu->heading = FloatToEQ12(m->GetHeading());
	spu->delta_x = FloatToEQ13(dx);
	spu->delta_y = FloatToEQ13(dy);
	spu->delta_z = FloatToEQ13(dz);
	spu->delta_heading = FloatToEQ10(dh);
	spu->animation = anim;
}

float MobMovementManager::FixHeading(float in)
{
	auto h = in;
	while (h > 512.0) {
		h -= 512.0;
	}

	while (h < 0.0) {
		h += 512.0;
	}
	
	return h;
}

MobMovementManager::~MobMovementManager()
{
}

void MobMovementManager::Process()
{
	for (auto &iter : _impl->Entries) {
		auto &ent = iter.second;

		if (ent.RotateCommand.active) {
			ProcessRotateCommand(iter.first, ent.RotateCommand);
		}
		else if (ent.MoveCommand.active) {

		}
	}
}

void MobMovementManager::AddMob(Mob *m)
{
	_impl->Entries.insert(std::make_pair(m, MobMovementEntry()));
}

void MobMovementManager::RemoveMob(Mob *m)
{
	_impl->Entries.erase(m);
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

void MobMovementManager::RotateTo(Mob *who, float to, float speed)
{
	auto iter = _impl->Entries.find(who);
	auto &ent = (*iter);

	auto from = FixHeading(who->GetHeading());
	to = FixHeading(to);

	ent.second.RotateCommand.active = true;
	ent.second.RotateCommand.started = false;
	ent.second.RotateCommand.rotate_to = to;
	ent.second.RotateCommand.rotate_to_speed = speed;

	double pdist = 0.0;
	double ndist = 0.0;
	if (to > from) {
		pdist = to - from;
	}
	else {
		pdist = 512.0 - from + to;
	}

	if (from > to) {
		ndist = from - to;
	}
	else {
		ndist = (512.0 - to) + from;
	}

	if (pdist <= ndist) {
		ent.second.RotateCommand.rotate_dir = 1.0;
	}
	else {
		ent.second.RotateCommand.rotate_dir = -1.0;
	}
}

void MobMovementManager::Teleport(Mob *who, float x, float y, float z, float heading)
{
}

void MobMovementManager::NavigateTo(Mob *who, float x, float y, float z, float speed)
{
	
}

void MobMovementManager::StopNavigation(Mob *who) {
	
}
