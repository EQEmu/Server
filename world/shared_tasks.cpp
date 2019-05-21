#include "shared_tasks.h"
#include "clientlist.h"
#include "cliententry.h"
#include "zonelist.h"

#include <algorithm>

extern ClientList client_list;
extern ZSList zoneserver_list;

void SharedTaskManager::HandleTaskRequest(ServerPacket *pack)
{
	if (!pack)
		return;

	/*
	 * Things done in zone:
	 * Verified we were requesting a shared task
	 * Verified leader has a slot available
	 * Verified leader met level reqs
	 * Verified repeatable or not completed (not doing that here?)
	 * Verified leader doesn't have a lock out
	 * Verified the group/raid met min/max player counts
	 */

	char tmp_str[64] = { 0 };
	int task_id = pack->ReadUInt32();
	int npc_id = pack->ReadUInt32();
	pack->ReadString(tmp_str);
	std::string leader_name = tmp_str;
	int player_count = pack->ReadUInt32();
	std::vector<std::string> players;
	for (int i = 0; i < player_count; ++i) {
		pack->ReadString(tmp_str);
		players.push_back(tmp_str);
	}

	// check if the task exist, we only load shared tasks in world, so we know the type is correct if found
	auto it = task_information.find(task_id);
	if (it == task_information.end()) { // not loaded! bad id or not shared task
		auto pc = client_list.FindCharacter(leader_name.c_str());
		if (pc) {
			// failure TODO: appropriate message
			auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 8);
			pack->WriteUInt32(0); // string ID or just generic fail message
			pack->WriteUInt32(npc_id);
			pack->WriteString(leader_name.c_str());
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		} // oh well
		return;
	}

	int id = GetNextID();
	auto ret = tasks.insert({id, {id, task_id}});
	if (!ret.second) {
		auto pc = client_list.FindCharacter(leader_name.c_str());
		if (pc) {
			// failure TODO: appropriate message
			auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 8);
			pack->WriteUInt32(0); // string ID or just generic fail message
			pack->WriteUInt32(npc_id);
			pack->WriteString(leader_name.c_str());
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		} // oh well
		return;
	}

	auto cle_leader = client_list.FindCharacter(leader_name.c_str());
	if (cle_leader == nullptr) {// something went wrong
		tasks.erase(ret.first);
		return;
	}

	auto &task = ret.first->second;
	task.AddMember(leader_name, cle_leader, true);

	if (players.empty()) {
		// send instant success to leader
		SerializeBuffer buf(10);
		buf.WriteInt32(id);				// shared task's ID
		buf.WriteInt32(task_id);		// ID of the task's data
		buf.WriteInt32(npc_id);			// NPC we're requesting from
		buf.WriteString(leader_name);	// leader's name
		buf.WriteInt32(0); // member list minus leader

		auto pack = new ServerPacket(ServerOP_TaskGrant, buf);
		zoneserver_list.SendPacket(cle_leader->zone(), cle_leader->instance(), pack);
		safe_delete(pack);

		task.SetCLESharedTasks();
		return;
	}

	for (auto &&name : players) {
		// look up CLEs by name, tell them we need to know if they can be added
		auto cle = client_list.FindCharacter(name.c_str());
		if (cle) {
			// make sure we don't have a shared task already
			if (!cle->HasFreeSharedTaskSlot()) {
				// failure TODO: appropriate message
				auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 8);
				pack->WriteUInt32(0); // string ID or just generic fail message
				pack->WriteUInt32(npc_id);
				pack->WriteString(leader_name.c_str());
				zoneserver_list.SendPacket(cle_leader->zone(), cle_leader->instance(), pack);
				safe_delete(pack);
				tasks.erase(ret.first);
				return;
			}

			// make sure our level is right
			if (!AppropriateLevel(task_id, cle->level())) {
				// failure TODO: appropriate message
				auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 8);
				pack->WriteUInt32(0); // string ID or just generic fail message
				pack->WriteUInt32(npc_id);
				pack->WriteString(leader_name.c_str());
				zoneserver_list.SendPacket(cle_leader->zone(), cle_leader->instance(), pack);
				safe_delete(pack);
				tasks.erase(ret.first);
				return;
			}

			// check our lock out timer
			int expires = cle->GetTaskLockoutExpire(task_id);
			if ((expires - Timer::GetCurrentTime()) >= 0) {
				// failure TODO: appropriate message, we need to send the timestamp here
				auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 8);
				pack->WriteUInt32(0); // string ID or just generic fail message
				pack->WriteUInt32(npc_id);
				pack->WriteString(leader_name.c_str());
				zoneserver_list.SendPacket(cle_leader->zone(), cle_leader->instance(), pack);
				safe_delete(pack);
				tasks.erase(ret.first);
				return;
			}

			// we're good, add to task
			task.AddMember(name, cle);
		}
	}

	// fire off to zone we're done!
	SerializeBuffer buf(10 + 10 * players.size());
	buf.WriteInt32(id);				// shared task's ID
	buf.WriteInt32(task_id);		// ID of the task's data
	buf.WriteInt32(npc_id);			// NPC we're requesting from
	buf.WriteString(leader_name);	// leader's name
	task.SerializeMembers(buf, false);	// everyone but leader

	auto reply = new ServerPacket(ServerOP_TaskGrant, buf);
	zoneserver_list.SendPacket(cle_leader->zone(), cle_leader->instance(), reply);
	safe_delete(reply);

	task.SetCLESharedTasks();
	return;
}

/*
 * This is called once during boot of world
 * We need to load next_id, clean up expired tasks (?), and populate the map
 */
bool SharedTaskManager::LoadSharedTaskState()
{
	// clean up expired tasks ... We may not want to do this if world crashes and has to be restarted
	// May need to wait to do it to cleanly inform existing zones to clean up expired tasks

	// Load existing tasks. We may not want to actually do this here and wait for a client to log in
	// But the crash case may actually dictate we should :P

	// set next_id to highest used ID + 1
	return true;
}

int SharedTaskManager::GetNextID()
{
	next_id++;
	// let's not be extra clever here ...
	while (tasks.count(next_id) != 0)
		next_id++;

	return next_id;
}

/*
 * returns true if the level fits in the task's defined range
 */
bool SharedTaskManager::AppropriateLevel(int id, int level) const
{
	auto it = task_information.find(id);
	// doesn't exist
	if (it == task_information.end())
		return false;

	auto &task = it->second;

	if (task.MinLevel && level < task.MinLevel)
		return false;

	if (task.MaxLevel && level > task.MaxLevel)
		return false;

	return true;
}

/*
 * This will check if any tasks have expired
 */
void SharedTaskManager::Process()
{
}

/*
 * When a player leaves world they will tell us to clean up their pointer
 * This is NOT leaving the shared task, just crashed or something
 */

void SharedTask::MemberLeftGame(ClientListEntry *cle)
{
	auto it = std::find_if(members.begin(), members.end(), [cle](SharedTaskMember &m) { return m.cle == cle; });

	// ahh okay ...
	if (it == members.end())
		return;

	it->cle = nullptr;
}

/*
 * Serializes Members into the SerializeBuffer
 * Starts with count then followed by names null-termed
 * In the future this will include monster mission shit
 */
void SharedTask::SerializeMembers(SerializeBuffer &buf, bool include_leader) const
{
	buf.WriteInt32(include_leader ? members.size() : members.size() - 1);

	for (auto && m : members) {
		if (!include_leader && m.leader)
			continue;

		buf.WriteString(m.name);
		// TODO: live also has monster mission class choice in here
	}
}

/*
 * This sets the CLE's quick look up shared task stuff
 */
void SharedTask::SetCLESharedTasks()
{
	for (auto &&m : members) {
		if (m.cle == nullptr) // shouldn't happen ....
			continue;

		m.cle->SetSharedTask(this);
		m.cle->SetCurrentSharedTaskID(id);
	}
}

