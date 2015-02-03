#include "../common/global_define.h"
#include "../common/emu_tcp_connection.h"
#include "../common/eqemu_logsys.h"
#include "../common/md5.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "../common/servertalk.h"
#include "../common/web_interface_utils.h"
#include "client.h"
#include "entity.h"
#include "mob.h"
#include "npc.h"
#include "quest_parser_collection.h"
#include "remote_call.h"
#include "remote_call_subscribe.h"
#include "worldserver.h"
#include "zone.h"
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

/* Zone: register_remote_call_handlers */
void register_remote_call_handlers() {
	remote_call_methods["Zone.Subscribe"] = handle_rc_subscribe;
	remote_call_methods["Zone.Unsubscribe"] = handle_rc_unsubscribe;
	remote_call_methods["Zone.GetInitialEntityPositions"] = handle_rc_get_initial_entity_positions;
	remote_call_methods["Zone.MoveEntity"] = handle_rc_move_entity;
	remote_call_methods["Zone.GetEntityAttributes"] = handle_rc_get_entity_attributes;
	remote_call_methods["Zone.SetEntityAttribute"] = handle_rc_set_entity_attribute;
	remote_call_methods["Zone.Action"] = handle_rc_zone_action;
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
		res["ent_id"] = std::to_string((long)npc->GetID());
		res["race_id"] = std::to_string((long)npc->GetRace());
		res["class_id"] = std::to_string((long)npc->GetClass()); 
		res["type"] = "NPC";
		res["name"] = npc->GetCleanName();
		res["x"] = std::to_string((double)npc->GetX());
		res["y"] = std::to_string((double)npc->GetY());
		res["z"] = std::to_string((double)npc->GetZ());
		res["h"] = std::to_string((double)npc->GetHeading());
		res["aggro_range"] = std::to_string((double)npc->GetAggroRange()); 
		res["gender"] = std::to_string((double)npc->GetGender());
		RemoteCallResponse(connection_id, request_id, res, error);
	}
	std::list<Client*> client_list;
	entity_list.GetClientList(client_list);
	for (std::list<Client*>::iterator itr = client_list.begin(); itr != client_list.end(); ++itr) {
		Client* c = *itr;
		res["ent_id"] = itoa(c->GetID());
		res["race_id"] = std::to_string((long)c->GetRace());
		res["class_id"] = std::to_string((long)c->GetClass());
		res["type"] = "Client";
		res["name"] = c->GetCleanName(); 
		res["x"] = itoa(c->GetX());
		res["y"] = itoa(c->GetY());
		res["z"] = itoa(c->GetZ());
		res["h"] = itoa(c->GetHeading());
		RemoteCallResponse(connection_id, request_id, res, error); 
	}
	std::list<Corpse*> corpse_list;
	entity_list.GetCorpseList(corpse_list);
	for (std::list<Corpse*>::iterator itr = corpse_list.begin(); itr != corpse_list.end(); ++itr) {
		Corpse* c = *itr;
		res["ent_id"] = itoa(c->GetID());
		res["race_id"] = std::to_string((long)c->GetRace());
		res["class_id"] = std::to_string((long)c->GetClass());
		res["type"] = "Corpse";
		res["name"] = c->GetCleanName();
		res["x"] = itoa(c->GetX());
		res["y"] = itoa(c->GetY());
		res["z"] = itoa(c->GetZ());
		res["h"] = itoa(c->GetHeading());
		RemoteCallResponse(connection_id, request_id, res, error);
	}
	std::list<Doors*> door_list;
	entity_list.GetDoorsList(door_list);
	for (std::list<Doors*>::iterator itr = door_list.begin(); itr != door_list.end(); ++itr) {
		Doors* c = *itr;
		res["ent_id"] = itoa(c->GetEntityID());
		res["type"] = "Door";
		res["name"] = c->GetDoorName();
		res["x"] = itoa(c->GetPosition().x);
		res["y"] = itoa(c->GetPosition().y);
		res["z"] = itoa(c->GetPosition().z);
		res["h"] = itoa(c->GetPosition().w);
		RemoteCallResponse(connection_id, request_id, res, error);
	}
	std::list<Object*> object_list;
	entity_list.GetObjectList(object_list);
	for (std::list<Object*>::iterator itr = object_list.begin(); itr != object_list.end(); ++itr) {
		Object* c = *itr;
		res["ent_id"] = itoa(c->GetID());
		res["type"] = "Object";
		res["name"] = c->GetModelName();
		res["x"] = itoa(c->GetX());
		res["y"] = itoa(c->GetY());
		res["z"] = itoa(c->GetZ());
		res["h"] = itoa(c->GetHeadingData());
		RemoteCallResponse(connection_id, request_id, res, error);
	}
}

void handle_rc_move_entity(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res; 
	if (params.size() != 5) {
		error = "Missing function data";
		std::cout << error << "\n" << std::endl;
		RemoteCallResponse(connection_id, request_id, res, error);
		return; 
	} 
	/* 0 = Ent ID, 1 = X, 2 = Y, 3 = Z, 4 = H */
	Mob *ent = entity_list.GetMob(atoi(params[0].c_str()));
	if (ent){
		if (ent->IsClient()){
			ent->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), atoi(params[1].c_str()), atoi(params[2].c_str()), ent->GetGroundZ(atoi(params[1].c_str()), atoi(params[2].c_str())), ent->GetHeading());
		}
		else{
			ent->GMMove(atoi(params[1].c_str()), atoi(params[2].c_str()), ent->GetGroundZ(atoi(params[1].c_str()), atoi(params[2].c_str())), ent->GetHeading()); 
		}
	}
}

void handle_rc_zone_action(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	/* Zone Reload Functions */
	if (params[0] == "Repop"){ zone->Repop(); } 
	if (params[0] == "ReloadQuests"){ parse->ReloadQuests(); }

	/* Zone Visuals Functions */
	if (params[0] == "ZoneSky"){ 
		for (int z = 0; z < 4; z++) {
			zone->newzone_data.fog_red[z] = atoi(params[1].c_str());
			zone->newzone_data.fog_green[z] = atoi(params[2].c_str());
			zone->newzone_data.fog_blue[z] = atoi(params[3].c_str());
			zone->newzone_data.sky = 0; 
		}
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(0, outapp);  
		safe_delete(outapp);
	}
	if (params[0] == "ZoneFogDensity"){
		zone->newzone_data.fog_density = atof(params[1].c_str());
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);
	}
	if (params[0] == "ZoneFogClip"){
		for (int z = 0; z < 4; z++) {
			zone->newzone_data.fog_minclip[z] = atoi(params[1].c_str());
			zone->newzone_data.fog_maxclip[z] = atoi(params[2].c_str());
			zone->newzone_data.sky = 0; 
		}
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);
	}
	if (params[0] == "ZoneClip"){
		zone->newzone_data.minclip = atoi(params[1].c_str());
		zone->newzone_data.maxclip = atoi(params[2].c_str());
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);
	} 
	if (params[0] == "ZoneSaveHeaders"){ zone->SaveZoneCFG(); }
	
	if (params[0] == "Kill"){
		Mob *ent = entity_list.GetMob(atoi(params[1].c_str()));
		if (ent){ ent->Kill(); }
	}
}

/* Server -> Client */
void handle_rc_get_entity_attributes(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	if (params.size() != 1) {
		error = "Missing function data";
		std::cout << error << "\n" << std::endl;
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	Mob *ent = entity_list.GetMob(atoi(params[0].c_str()));
	if (ent){
		res["ent_id"] = itoa(ent->GetID());
		res["clean_name"] = ent->GetCleanName();
		res["name"] = ent->GetName();
		res["race"] = itoa(ent->GetRace());
		res["class"] = itoa(ent->GetClass());
		res["size"] = itoa(ent->GetSize());
		res["texture"] = itoa(ent->GetTexture());
		res["gender"] = itoa(ent->GetGender()); 
		res["weapon_1"] = itoa(ent->GetEquipmentMaterial(7));
		res["weapon_2"] = itoa(ent->GetEquipmentMaterial(8));
		res["heading"] = itoa(ent->GetHeading());
		RemoteCallResponse(connection_id, request_id, res, error);
	}
}

/* Client -> Server :: Zone.SetEntityAttribute */
void handle_rc_set_entity_attribute(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params) {
	std::string error;
	std::map<std::string, std::string> res;

	if (params.size() != 3) {
		error = "Missing function data";
		std::cout << error << "\n" << std::endl;
		RemoteCallResponse(connection_id, request_id, res, error);
		return;
	}

	Mob *ent = entity_list.GetMob(atoi(params[0].c_str()));
	if (ent){
		if (params[1] == "race"){ ent->SendIllusionPacket(atoi(params[2].c_str())); }
		if (params[1] == "appearance_effect"){ ent->SendAppearanceEffect(atoi(params[2].c_str()), 0, 0, 0, 0); }
		if (params[1] == "size"){ ent->ChangeSize(atoi(params[2].c_str())); }
		if (params[1] == "texture"){ ent->SendIllusionPacket(0, 0xFF, atoi(params[2].c_str()), 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0); }
		if (params[1] == "gender"){ ent->SendIllusionPacket(0, atoi(params[2].c_str()), 0xFF, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0); }
		if (params[1] == "weapon_1"){ ent->WearChange(7, atoi(params[2].c_str()), 0); }
		if (params[1] == "weapon_2"){ ent->WearChange(8, atoi(params[2].c_str()), 0); }
		if (params[1] == "heading"){ ent->GMMove(ent->GetX(), ent->GetY(), ent->GetZ(), atoi(params[2].c_str()), true); }
	}
}