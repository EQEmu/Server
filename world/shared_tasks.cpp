#include "shared_tasks.h"
#include "clientlist.h"
#include "cliententry.h"
#include "zonelist.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

void SharedTaskManager::HandleTaskRequest(ServerPacket *pack)
{
	if (!pack)
		return;

	char tmp_str[64] = { 0 };
	int task_id = pack->ReadUInt32();
	pack->ReadString(tmp_str);
	std::string leader_name = tmp_str;
	int missing_count = pack->ReadUInt32();
	std::vector<std::string> missing_players;
	for (int i = 0; i < missing_count; ++i) {
		pack->ReadString(tmp_str);
		missing_players.push_back(tmp_str);
	}

	int id = GetNextID();
	auto ret = tasks.insert({id, {id, task_id}});
	if (!ret.second) {
		auto pc = client_list.FindCharacter(leader_name.c_str());
		if (pc) {
			auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 4);
			pack->WriteUInt32(0); // string ID or just generic fail message
			pack->WriteString(leader_name.c_str());
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		} // oh well
	}

	auto &task = ret.first->second;
	task.AddMember(leader_name, true);

	if (missing_players.empty()) {
		// send instant success to leader
		auto pc = client_list.FindCharacter(leader_name.c_str());
		if (pc) {
			SerializeBuffer buf(10);
			buf.WriteInt32(id);				// task's ID
			buf.WriteString(leader_name);	// leader's name

			auto pack = new ServerPacket(ServerOP_TaskGrant, buf);
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		} else { // well fuck
			tasks.erase(ret.first);
		}
		return;
	}

	task.SetMissingCount(missing_count);
	for (auto &&name : missing_players) {
		// look up CLEs by name, tell them we need to know if they can be added
		auto pc = client_list.FindCharacter(name.c_str());
		if (pc) {
			SerializeBuffer buf(10);
			buf.WriteInt32(id);
			buf.WriteInt32(task_id);
			buf.WriteString(name);

			auto pack = new ServerPacket(ServerOP_TaskRequest, buf);
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		} else { // asked for a toon we couldn't find ABORT!
			auto pc = client_list.FindCharacter(leader_name.c_str());
			if (pc) {
				auto pack = new ServerPacket(ServerOP_TaskReject, leader_name.size() + 1 + 4);
				pack->WriteUInt32(0); // string ID or just generic fail message
				pack->WriteString(leader_name.c_str());
				zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
				safe_delete(pack);
			} // oh well
			tasks.erase(ret.first);
			break;
		}
	}
}

void SharedTaskManager::HandleTaskRequestReply(ServerPacket *pack)
{
	if (!pack)
		return;

	int id = pack->ReadUInt32();

	char name[64] = { 0 };
	pack->ReadString(name);

	int status = pack->ReadUInt32();

	auto it = tasks.find(id);
	if (it == tasks.end()) {
		// task already errored and no longer existed, we can ignore
		return;
	}

	auto &task = it->second;

	if (status != TASKJOINOOZ_CAN) {
		// TODO: forward to leader
		return;
	}

	if (!task.DecrementMissingCount())
		tasks.erase(it);
}

bool SharedTask::DecrementMissingCount()
{
	--missing_count;
	if (missing_count == 0) {
		auto pc = client_list.FindCharacter(leader_name.c_str());
		if (pc) {
			SerializeBuffer buf(10);
			buf.WriteInt32(id);				// task's ID
			buf.WriteString(leader_name);	// leader's name

			auto pack = new ServerPacket(ServerOP_TaskGrant, buf);
			zoneserver_list.SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		} else {
			return false; // error, please clean us up
		}
	}
	return true;
}

