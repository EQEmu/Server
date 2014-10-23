#include <string>

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

	WILink.SendPacket(pack);
	safe_delete(pack);
}

/* World:register_remote_call_handlers */
void register_remote_call_handlers() {
	remote_call_methods["World.ListZones"] = handle_rc_list_zones;
	remote_call_methods["World.GetZoneDetails"] = handle_rc_get_zone_info;
	remote_call_methods["Zone.Subscribe"] = handle_rc_relay;
	remote_call_methods["Zone.Unsubscribe"] = handle_rc_relay;
	remote_call_methods["Zone.GetInitialEntityPositions"] = handle_rc_relay;
	remote_call_methods["Zone.MoveEntity"] = handle_rc_relay;
	remote_call_methods["Zone.Action"] = handle_rc_relay;
	remote_call_methods["World.GetFileContents"] = handle_rc_get_file_contents;
	remote_call_methods["World.SaveFileContents"] = handle_rc_save_file_contents;
}

void handle_rc_list_zones(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::vector<uint32> zones;
	zoneserver_list.GetZoneIDList(zones);

	std::map<std::string, std::string> res;
	uint32 sz = (uint32)zones.size();
	for(uint32 i = 0; i < sz; ++i) {
		res[itoa(i)] = (itoa(zones[i]));
	}
	
	std::string error;
	RemoteCallResponse(connection_id, request_id, res, error);
}

void handle_rc_get_zone_info(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;
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

	res["type"] = zs->IsStaticZone() ? "static" : "dynamic";
	res["zone_id"] = itoa(zs->GetZoneID());
	res["instance_id"] = itoa(zs->GetInstanceID());
	res["launch_name"] = zs->GetLaunchName();
	res["launched_name"] = zs->GetLaunchedName();
	res["short_name"] = zs->GetZoneName();
	res["long_name"] = zs->GetZoneLongName();
	res["port"] = itoa(zs->GetCPort());
	res["player_count"] = itoa(zs->NumPlayers());
	RemoteCallResponse(connection_id, request_id, res, error);
}

void handle_rc_relay(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;
	uint32 zone_id = 0;
	uint32 instance_id = 0;
	ZoneServer *zs = nullptr;
	
	if(params.size() < 2) {
		error = "Missing zone relay params";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}
	
	zone_id = (uint32)atoi(params[0].c_str());
	instance_id = (uint32)atoi(params[1].c_str());
	if(!zone_id && !instance_id) {
		error = "Zone not booted";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}
	
	
	if(instance_id) {
		zs = zoneserver_list.FindByInstanceID(instance_id);
	} else {
		zs = zoneserver_list.FindByZoneID(zone_id);
	}
	
	if(!zs) {
		error = "Zone server not found";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}
	
	uint32 sz = (uint32)(request_id.size() + connection_id.size() + method.size() + 3 + 16);
	uint32 p_sz = (uint32)params.size() - 2;
	for(uint32 i = 0; i < p_sz; ++i) {
		auto &param = params[i + 2];
		sz += (uint32)param.size();
		sz += 5;
	}
	
	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCall, sz);
	pack->WriteUInt32((uint32)request_id.size());
	pack->WriteString(request_id.c_str());
	pack->WriteUInt32((uint32)connection_id.size());
	pack->WriteString(connection_id.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32(p_sz);
	
	for(uint32 i = 0; i < p_sz; ++i) {
		auto &param = params[i + 2];
		pack->WriteUInt32((uint32)param.size());
		pack->WriteString(param.c_str());
	}
	
	zs->SendPacket(pack);
	safe_delete(pack);
}

//TODO: We need to look at potential security concerns on direct file access like this.
void handle_rc_get_file_contents(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;
	if(params.size() != 1) {
		error = "Expected only one filename.";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	FILE *f = fopen(params[0].c_str(), "rb");
	if(!f) {
		error = "File not found: " + params[0];
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	rewind(f);

	char *buffer = new char[sz + 1];
	size_t r = fread(buffer, 1, sz, f);
	if(r != sz) {
		error = "Unable to read file: " + params[0];
		RemoteCallResponse(connection_id, request_id, res, error);
		fclose(f);
		delete[] buffer;
		return;
	}

	fclose(f);
	buffer[sz] = '\0';

	res["quest_text"] = buffer;
	res["file_name"] = params[0];
	delete[] buffer;
	RemoteCallResponse(connection_id, request_id, res, error);
}

void handle_rc_save_file_contents(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;
	if(params.size() != 2) {
		error = "Expected [filename, content]";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	FILE *f = fopen(params[0].c_str(), "wb");
	if(!f) {
		error = "File not found: " + params[0];
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	if(params[1].size() == 0) {
		fclose(f);
		res["status"] = "success";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	size_t r = fwrite(params[1].c_str(), 1, params[1].size(), f);
	fclose(f);

	if(r != params[1].size()) {
		error = "Unable to write file: " + params[0];
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	res["status"] = "success";
	RemoteCallResponse(connection_id, request_id, res, error);
}