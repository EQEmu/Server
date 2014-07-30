#include "../common/debug.h"
#include "../common/logsys.h"
#include "../common/logtypes.h"
#include "../common/md5.h"
#include "../common/EmuTCPConnection.h"
#include "../common/packet_dump.h"
#include "WorldConfig.h"
#include "clientlist.h"
#include "zonelist.h"
#include "web_interface.h"
#include "remote_call.h"
#include "zoneserver.h"

extern ClientList client_list;
extern ZSList zoneserver_list;
extern WebInterfaceConnection WILink;
std::map<std::string, RemoteCallHandler> remote_call_methods;

void RemoteCallResponse(const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &res, const std::string &error) {
	uint32 sz = connection_id.size() + request_id.size() + error.size() + 3 + 16;
	uint32 res_sz = res.size();
	for(uint32 i = 0; i < res_sz; ++i) {
		sz += res[i].size() + 5;
	}

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCallResponse, sz);
	pack->WriteUInt32((uint32)request_id.size());
	pack->WriteString(request_id.c_str());
	pack->WriteUInt32((uint32)connection_id.size());
	pack->WriteString(connection_id.c_str());
	pack->WriteUInt32((uint32)error.size());
	pack->WriteString(error.c_str());
	pack->WriteUInt32((uint32)res_sz);
	for (uint32 i = 0; i < res_sz; ++i) {
		auto &r = res[i];
		pack->WriteUInt32((uint32)r.size());
		pack->WriteString(r.c_str());
	}

	WILink.SendPacket(pack);
}

void register_remote_call_handlers() {
	remote_call_methods["list_zones"] = handle_rc_list_zones;
	remote_call_methods["get_zone_info"] = handle_rc_get_zone_info;
}

void handle_rc_list_zones(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::vector<uint32> zones;
	zoneserver_list.GetZoneIDList(zones);

	std::vector<std::string> res;
	uint32 sz = (uint32)zones.size();
	for(uint32 i = 0; i < sz; ++i) {
		res.push_back(itoa(zones[i]));
	}
	
	std::string error;
	RemoteCallResponse(connection_id, request_id, res, error);
}

void handle_rc_get_zone_info(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::vector<std::string> res;
	if(params.size() != 1) {
		error = "Expected only one zone_id.";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	ZoneServer *zs = zoneserver_list.FindByID(atoi(params[0].c_str()));
	if(zs == nullptr) {
		error = "Invalid zone";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	res.push_back(zs->IsStaticZone() ? "static" : "dynamic");
	res.push_back(itoa(zs->GetZoneID()));
	res.push_back(itoa(zs->GetInstanceID()));
	res.push_back(zs->GetLaunchName());
	res.push_back(zs->GetLaunchedName());
	res.push_back(zs->GetZoneName());
	res.push_back(zs->GetZoneLongName());
	res.push_back(itoa(zs->GetCPort()));
	res.push_back(itoa(zs->NumPlayers()));
	RemoteCallResponse(connection_id, request_id, res, error);
}
