#include "../common/debug.h"
#include "../common/logsys.h"
#include "../common/logtypes.h"
#include "../common/md5.h"
#include "../common/EmuTCPConnection.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/servertalk.h"
#include "../common/web_interface_utils.h"
#include "remote_call.h"
#include "remote_call_subscribe.h"
#include "worldserver.h"
#include "zone.h"
#include "entity.h"
#include "npc.h"
#include "mob.h"
#include "client.h"
#include <string>

std::map<std::string, RemoteCallHandler> remote_call_methods;
extern WorldServer worldserver;
extern Zone* zone;

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

/* Zone */
void register_remote_call_handlers() {
	remote_call_methods["Zone.Subscribe"] = handle_rc_subscribe;
	remote_call_methods["Zone.Unsubscribe"] = handle_rc_unsubscribe;
	remote_call_methods["Zone.Get.Initial.Entity.Positions"] = handle_rc_get_initial_entity_positions;
	remote_call_methods["Zone.Move.Entity"] = handle_rc_move_entity; 
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

void handle_rc_get_initial_entity_positions(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	std::list<NPC*> npc_list;
	entity_list.GetNPCList(npc_list);
	for(std::list<NPC*>::iterator itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
		NPC* npc = *itr;
		res["zone_id"] = std::to_string((long)zone->GetZoneID());
		res["instance_id"] = std::to_string((long)zone->GetInstanceID());
		res["ent_id"] = std::to_string((long)npc->GetID());
		res["type"] = "NPC";
		res["name"] = npc->GetName();
		res["x"] = std::to_string((double)npc->GetX());
		res["y"] = std::to_string((double)npc->GetY());
		res["z"] = std::to_string((double)npc->GetZ());
		res["h"] = std::to_string((double)npc->GetHeading());
		RemoteCallResponse(connection_id, request_id, res, error);
	}
	std::list<Client*> client_list;
	entity_list.GetClientList(client_list);
	for (std::list<Client*>::iterator itr = client_list.begin(); itr != client_list.end(); ++itr) {
		Client* c = *itr;
		res["zone_id"] = itoa(zone->GetZoneID());
		res["instance_id"] = itoa(zone->GetInstanceID());
		res["ent_id"] = itoa(c->GetID());
		res["type"] = "Client";
		res["name"] = c->GetCleanName();
		res["x"] = itoa(c->GetX());
		res["y"] = itoa(c->GetY());
		res["z"] = itoa(c->GetZ());
		res["h"] = itoa(c->GetHeading());
		RemoteCallResponse(connection_id, request_id, res, error); 
	}
}

void handle_rc_move_entity(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	if (params.size() != 1) {
		error = "Missing function data";
		RemoteCallResponse(connection_id, request_id, res, error);
		return;

	}

	printf("params 0 = %s\n", params[0].c_str());
	printf("params 1 = %s\n", params[1].c_str());
	printf("params 2 = %s\n", params[2].c_str());
	printf("params 3 = %s\n", params[3].c_str());
	return;
	auto arg_v = explode_string(params[0].c_str(), ':');
	/*
	0 = Ent ID
	1 = X
	2 = Y
	3 = Z
	4 = H
	*/
	Mob *ent = entity_list.GetMob(atoi(arg_v[0].c_str()));
	if (ent){
		if (ent->IsClient()){
			ent->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), atoi(arg_v[1].c_str()), atoi(arg_v[2].c_str()), ent->GetGroundZ(atoi(arg_v[1].c_str()), atoi(arg_v[2].c_str())), ent->GetHeading());

		}
		else{
			ent->GMMove(atoi(arg_v[1].c_str()), atoi(arg_v[2].c_str()), ent->GetGroundZ(atoi(arg_v[1].c_str()), atoi(arg_v[2].c_str())), ent->GetHeading());
		}
	}
}