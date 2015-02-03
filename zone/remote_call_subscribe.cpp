#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/md5.h"
#include "../common/emu_tcp_connection.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/servertalk.h"
#include "remote_call_subscribe.h"
#include "remote_call.h"
#include "worldserver.h"
#include "zone.h"

extern WorldServer worldserver;
extern Zone* zone;

RemoteCallSubscriptionHandler* RemoteCallSubscriptionHandler::_instance = nullptr;

RemoteCallSubscriptionHandler::RemoteCallSubscriptionHandler() {
}

RemoteCallSubscriptionHandler::~RemoteCallSubscriptionHandler() {
}

RemoteCallSubscriptionHandler *RemoteCallSubscriptionHandler::Instance()
{
	if(!_instance) {
		_instance = new RemoteCallSubscriptionHandler();
	}

	return _instance;
}

bool RemoteCallSubscriptionHandler::Subscribe(std::string connection_id, std::string event_name) {
	if(registered_events.count(event_name) == 0) {
		std::vector<std::string> r;
		r.push_back(connection_id);
		registered_events[event_name] = r;

		if(connection_ids.count(connection_id) == 0) {
			connection_ids[connection_id] = 1;
		} else {
			int count = connection_ids[connection_id];
			connection_ids[connection_id] = count + 1;
		}

		return true;
	} else {
		std::vector<std::string>& r = registered_events[event_name];

		size_t sz = r.size();
		for(size_t i = 0; i < sz; ++i) {
			if(connection_id.compare(r[i]) == 0) {
				return false;
			}
		}

		r.push_back(connection_id);
		registered_events[event_name] = r;

		if(connection_ids.count(connection_id) == 0) {
			connection_ids[connection_id] = 1;
		}
		else {
			int count = connection_ids[connection_id];
			connection_ids[connection_id] = count + 1;
		}

		return true;
	}
}

bool RemoteCallSubscriptionHandler::Unsubscribe(std::string connection_id, std::string event_name) {
	if(registered_events.count(event_name) == 0) {
		return false;
	}

	std::vector<std::string>& r = registered_events[event_name];
	auto iter = r.begin();
	while(iter != r.end()) {
		if(iter->compare(connection_id) == 0) {
			r.erase(iter);
			registered_events[event_name] = r;

			int count = connection_ids[connection_id];
			connection_ids[connection_id] = count - 1;
			return true;
		}
	}

	return false;
}

void RemoteCallSubscriptionHandler::OnEvent(std::string method, std::vector<std::string> &params) {
	if(registered_events.count(method) == 0) {
		return;
	}

	std::string func = "On." + method;
	std::vector<std::string> &conns = registered_events[method];
	if(conns.size() > 0) {
		auto iter = conns.begin();
		while(iter != conns.end()) {
			RemoteCall((*iter), func, params);
			++iter;
		}
	}
}

bool RemoteCallSubscriptionHandler::IsSubscribed(std::string method){
	if (registered_events.count(method) == 0) {
		return false; 
	}
	return true; 
}

void RemoteCallSubscriptionHandler::Process() {
	//create a check for all these connection ids packet
	uint32 sz = 12;
	auto iter = connection_ids.begin();
	while(iter != connection_ids.end()) {
		sz += (uint32)iter->first.size();
		sz += 5;
		++iter;
	}

	if (!zone)
		return;

	ServerPacket *pack = new ServerPacket(ServerOP_WIClientSession, sz);
	pack->WriteUInt32((uint32)zone->GetZoneID());
	pack->WriteUInt32((uint32)zone->GetInstanceID());
	pack->WriteUInt32((uint32)connection_ids.size());

	iter = connection_ids.begin();
	while(iter != connection_ids.end()) {
		pack->WriteUInt32((uint32)iter->first.size());
		pack->WriteString(iter->first.c_str());
		++iter;
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void RemoteCallSubscriptionHandler::ClearConnection(std::string connection_id) {
	if(connection_ids.count(connection_id) != 0) {
		connection_ids.erase(connection_id);
	}

	auto iter = registered_events.begin();
	while(iter != registered_events.end()) {
		auto &conns = iter->second;
		auto conn_iter = conns.begin();
		while(conn_iter != conns.end()) {
			if(conn_iter->compare(connection_id) == 0) {
				conns.erase(conn_iter);
				registered_events[iter->first] = conns;
				break;
			}
			++conn_iter;
		}

		++iter;
	}
}

void RemoteCallSubscriptionHandler::ClearAllConnections() {
	registered_events.clear();
	connection_ids.clear();
}
