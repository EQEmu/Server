#include "../common/debug.h"
#include "../common/logsys.h"
#include "../common/logtypes.h"
#include "../common/md5.h"
#include "../common/EmuTCPConnection.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/servertalk.h"
#include "remote_call.h"
#include "remote_call_subscribe.h"
#include "worldserver.h"

std::map<std::string, RemoteCallHandler> remote_call_methods;
extern WorldServer worldserver;

void RemoteCallResponse(const std::string &connection_id, const std::string &request_id, const std::map<std::string, std::string> &res, const std::string &error) {
	uint32 sz = (uint32)(connection_id.size() + request_id.size() + error.size() + 3 + 16);
	auto iter = res.begin();
	while(iter != res.end()) {
		sz += (uint32)iter->first.size() + (uint32)iter->second.size() + 10;
		++iter;
	}

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCallResponse, sz);
	pack->WriteUInt32((uint32)request_id.size());
	pack->WriteString(request_id.c_str());
	pack->WriteUInt32((uint32)connection_id.size());
	pack->WriteString(connection_id.c_str());
	pack->WriteUInt32((uint32)error.size());
	pack->WriteString(error.c_str());
	pack->WriteUInt32((uint32)res.size());
	iter = res.begin();
	while(iter != res.end()) {
		pack->WriteUInt32((uint32)iter->first.size());
		pack->WriteString(iter->first.c_str());
		pack->WriteUInt32((uint32)iter->second.size());
		pack->WriteString(iter->second.c_str());
		++iter;
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void RemoteCall(const std::string &connection_id, const std::string &method, const std::vector<std::string> &params) {
	uint32 sz = (uint32)(connection_id.size() + method.size() + 14);
	auto iter = params.begin();
	while(iter != params.end()) {
		sz += (uint32)iter->size() + 5;
		++iter;
	}

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCallToClient, sz);
	pack->WriteUInt32((uint32)connection_id.size());
	pack->WriteString(connection_id.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32((uint32)params.size());
	iter = params.begin();
	while(iter != params.end()) {
		pack->WriteUInt32((uint32)iter->size());
		pack->WriteString(iter->c_str());
		++iter;
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void register_remote_call_handlers() {
	remote_call_methods["Zone.Subscribe"] = handle_rc_subscribe;
	remote_call_methods["Zone.Unsubscribe"] = handle_rc_unsubscribe;
}

void handle_rc_subscribe(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	if(params.size() != 1) {
		error = "Missing event to subscribe to";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	if(RemoteCallSubscriptionHandler::Instance()->Subscribe(connection_id, params[0])) {
		res["status"] = "subscribed";
	} else {
		res["status"] = "failed to subscribe";
	}
	
	RemoteCallResponse(connection_id, request_id, res, error);
}

void handle_rc_unsubscribe(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	if(params.size() != 1) {
		error = "Missing event to unsubscribe from";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	if(RemoteCallSubscriptionHandler::Instance()->Subscribe(connection_id, params[0])) {
		res["status"] = "unsubscribed";
	}
	else {
		res["status"] = "failed to unsubscribe";
	}

	RemoteCallResponse(connection_id, request_id, res, error);
}
