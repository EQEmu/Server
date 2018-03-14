#include "entity.h"
#include "mob.h"
#include "client.h"
#include "doors.h"
#include "corpse.h"
#include "object.h"
#include "event_codes.h"
#include "nats.h"
#include "zone_config.h"
#include "nats_manager.h"
//#include "guild_mgr.h" //used for database, map error
#include "npc.h"

#include "../common/opcodemgr.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"
#ifndef PROTO_H
#define PROTO_H
#undef new //needed for linux compile
#include "../common/message.pb.h"
#endif
#include <google/protobuf/arena.h>

google::protobuf::Arena the_arena;

const ZoneConfig *zoneConfig;

NatsManager::NatsManager()
{
	//new timers, object initialization

	zoneConfig = ZoneConfig::get();
}

NatsManager::~NatsManager()
{
	nats_timer.Disable();
	// Destroy all our objects to avoid report of memory leak
	natsConnection_Destroy(conn);
	natsOptions_Destroy(opts);

	// To silence reports of memory still in used with valgrind
	nats_Close();
}

void NatsManager::Process()
{
	if (!connect()) 
		return;

	natsMsg *msg = NULL;
	std::string pubMessage;

	s = NATS_OK;
	for (int count = 0; (s == NATS_OK) && count < 5; count++)
	{
		s = natsSubscription_NextMsg(&msg, zoneCommandMessageSub, 1);
		if (s != NATS_OK)
			break;

		eqproto::CommandMessage* message = google::protobuf::Arena::CreateMessage<eqproto::CommandMessage>(&the_arena);
		if (!message->ParseFromString(natsMsg_GetData(msg))) {
			Log(Logs::General, Logs::NATS, "zone.%s.command_message.in: failed to parse", subscribedZoneName.c_str());
			natsMsg_Destroy(msg);
			continue;
		}
		GetCommandMessage(message, natsMsg_GetReply(msg));
		natsMsg_Destroy(msg);
	}

	s = NATS_OK;
	for (int count = 0; (s == NATS_OK) && count < 5; count++)
	{
		s = natsSubscription_NextMsg(&msg, zoneInstanceCommandMessageSub, 1);
		if (s != NATS_OK)
			break;

		eqproto::CommandMessage* message = google::protobuf::Arena::CreateMessage<eqproto::CommandMessage>(&the_arena);
		if (!message->ParseFromString(natsMsg_GetData(msg))) {
			Log(Logs::General, Logs::NATS, "zone.%s.%dcommand_message.in: failed to parse", subscribedZoneName.c_str(), subscribedZoneInstance);
			natsMsg_Destroy(msg);
			continue;
		}
		GetCommandMessage(message, natsMsg_GetReply(msg));
		natsMsg_Destroy(msg);
	}

	s = NATS_OK;
	for (int count = 0; (s == NATS_OK) && count < 5; count++)
	{
		s = natsSubscription_NextMsg(&msg, zoneChannelMessageSub, 1);
		if (s != NATS_OK)
			break;

		eqproto::ChannelMessage* message = google::protobuf::Arena::CreateMessage<eqproto::ChannelMessage>(&the_arena);
		if (!message->ParseFromString(natsMsg_GetData(msg))) {
			Log(Logs::General, Logs::NATS, "zone.%s.channel_message.in: failed to parse", subscribedZoneName.c_str());
			natsMsg_Destroy(msg);
			continue;
		}
		GetChannelMessage(message, natsMsg_GetReply(msg));
		natsMsg_Destroy(msg);
	}

	s = NATS_OK;
	for (int count = 0; (s == NATS_OK) && count < 5; count++)
	{
		s = natsSubscription_NextMsg(&msg, zoneInstanceChannelMessageSub, 1);
		if (s != NATS_OK)
			break;

		eqproto::ChannelMessage* message = google::protobuf::Arena::CreateMessage<eqproto::ChannelMessage>(&the_arena);
		if (!message->ParseFromString(natsMsg_GetData(msg))) {
			Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.in: failed to parse", subscribedZoneName.c_str(), subscribedZoneInstance);
			natsMsg_Destroy(msg);
			continue;
		}
		GetChannelMessage(message, natsMsg_GetReply(msg));
		natsMsg_Destroy(msg);
	}
}

// GetChannelMessage is when a 3rd party app sends a channel message via NATS.
void NatsManager::GetChannelMessage(eqproto::ChannelMessage* message, const char* reply) {
	if (!connect())
		return;

	if (message->from_entity_id() < 1) {
		message->set_result("from_entity_id must be set to send zone channel messages.");
		SendChannelMessage(message, reply);
		return;
	}


	auto mob = entity_list.GetMobID(message->from_entity_id());
	if (!mob) {
		message->set_result("from_entity_id not found");
		SendChannelMessage(message, reply);
		return;
	}

	if (message->distance() > 0)
		entity_list.MessageClose(mob, message->skip_sender(), message->distance(), message->number(), message->message().c_str());
	else
		mob->Message(message->number(), message->message().c_str());

	message->set_result("1");
	SendChannelMessage(message, reply);
	return;
}


// SendChannelMessage will send a channel message to NATS
void NatsManager::SendChannelMessage(eqproto::ChannelMessage* message, const char* reply) {
	if (!connect())
		return;

	std::string pubMessage;
	if (!message->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "world.channel_message.out: failed to serialize message to string");
		return;
	}

	if (reply)
		s = natsConnection_Publish(conn, reply, (const void*)pubMessage.c_str(), pubMessage.length());	
	else
		s = natsConnection_Publish(conn, StringFormat("zone.%s.%d.channel_message.out", subscribedZoneName.c_str(), subscribedZoneInstance).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	

	if (s != NATS_OK) {
		Log(Logs::General, Logs::NATS, "world.channel_message.out failed: %s", nats_GetLastError(&s));
		return;
	}

	if (reply) 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.in: %s (%s)", subscribedZoneName.c_str(), subscribedZoneInstance, message->message().c_str(), message->result().c_str());
	else 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.out: %s", subscribedZoneName.c_str(), subscribedZoneInstance, message->message().c_str());
}


// GetCommandMessage is used to process a command message
void NatsManager::GetCommandMessage(eqproto::CommandMessage* message, const char* reply) {
	if (!connect())
		return;

	if (message->command().compare("npctypespawn") == 0) {
		if (message->params_size() < 2) {
			message->set_result("Format: npctypespawn <x> <y> <z> <h> <npctypeid> <factionid>");
			SendCommandMessage(message, reply);
			return;
		}

		float x = atof(message->params(0).c_str());
		float y = atof(message->params(1).c_str());
		float z = atof(message->params(2).c_str());
		float h = atof(message->params(3).c_str());
		auto position = glm::vec4(x, y, z, h);

		uint32 npctypeid = atoi(message->params(4).c_str());
		uint32 factionid = atoi(message->params(5).c_str());
		const NPCType* tmp = 0;
		if (!(tmp = database.LoadNPCTypesData(npctypeid))) {
			message->set_result(StringFormat("NPC Type %i not found", npctypeid));
			SendCommandMessage(message, reply);
			return;
		}

		//tmp->fixedZ = 1;

		auto npc = new NPC(tmp, 0, position, FlyMode3);
		if (!npc) {
			message->set_result("failed to instantiate npc");
			SendCommandMessage(message, reply);
			return;
		}

		if (factionid > 0)
			npc->SetNPCFactionID(factionid);
		npc->AddLootTable();
		entity_list.AddNPC(npc);
		message->set_result(StringFormat("%u", npc->GetID()));
		SendCommandMessage(message, reply);
		return;
	}

	if (message->command().compare("spawn") == 0) {
		if (message->params_size() < 5) {
			message->set_result("Format: spawn <x> <y> <z> <h> <name> <race> <level> <material> <hp> <gender> <class> <priweapon> <secweapon> <merchantid> <bodytype> - spawns a npc those parameters.");
			SendCommandMessage(message, reply);
			return;
		}

		float x = atof(message->params(0).c_str());
		float y = atof(message->params(1).c_str());
		float z = atof(message->params(2).c_str());
		float h = atof(message->params(3).c_str());
		auto position = glm::vec4(x, y, z, h);

		std::string argumentString;
		for (int i = 4; i < message->params_size(); i++) {
			argumentString.append(StringFormat(" %s", message->params(i).c_str()));
		}

		NPC* npc = NPC::SpawnNPC(argumentString.c_str(), position, NULL);
		if (!npc) {
			message->set_result("Format: spawn <x> <y> <z> <h> <name> <race> <level> <material> <hp> <gender> <class> <priweapon> <secweapon> <merchantid> <bodytype> - spawns a npc those parameters.");
			SendCommandMessage(message, reply);
			return;
		}

		message->set_result(StringFormat("%u", npc->GetID()));
		SendCommandMessage(message, reply);
		return;
	}

	if (message->command().compare("moveto") == 0) {
		if (message->params_size() < 5) {
			message->set_result("Usage: moveto <entityid> <x> <y> <z> <h>.");
			SendCommandMessage(message, reply);
			return;
		}

		uint16 entityid = atoi(message->params(0).c_str());
		float x = atof(message->params(1).c_str());
		float y = atof(message->params(2).c_str());
		float z = atof(message->params(3).c_str());
		float h = atof(message->params(4).c_str());
		auto position = glm::vec4(x, y, z, h);

		auto npc = entity_list.GetNPCByID(entityid);
		if (!npc) {
			message->set_result("Invalid entity ID passed, or not an npc, etc");
			SendCommandMessage(message, reply);
			return;
		}

		npc->MoveTo(position, true);
		message->set_result("1");
		SendCommandMessage(message, reply);
		return;
	}

	if (message->command().compare("attack") == 0) {
		if (message->params_size() < 3) {
			message->set_result("Usage: attack <entityid> <targetentityid> <hateamount>.");
			SendCommandMessage(message, reply);
			return;
		}
		uint16 entityID = atoi(message->params(0).c_str());
		uint16 targetEntityID = atoi(message->params(1).c_str());
		uint32 hateAmount = atoi(message->params(2).c_str());

		auto npc = entity_list.GetNPCByID(entityID);
		if (!npc) {
			message->set_result("Invalid entity ID passed, or not an npc, etc");
			SendCommandMessage(message, reply);
			return;
		}

		auto mob = entity_list.GetMobID(targetEntityID);
		if (!mob) {
			message->set_result("Invalid target entitiy ID passed, or not a mob, etc");
			SendCommandMessage(message, reply);
			return;
		}
		npc->AddToHateList(mob, hateAmount);
		message->set_result("1");
		SendCommandMessage(message, reply);
		return;
	}

	if (message->command().compare("entitylist") == 0) {
		std::string entityPayload;
		if (message->params_size() < 1) {
			message->set_result("Usage: entitylist <type>. Types: npc, client, mob, mercenary, corpse, door, object");
			SendCommandMessage(message, reply);
			return;
		}

		eqproto::Entities* entities = google::protobuf::Arena::CreateMessage<eqproto::Entities>(&the_arena);
		if (message->params(0).compare("npc") == 0) {

			auto ents = entity_list.GetNPCList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				auto ent = entry.second;
				auto entity = entities->add_entities();
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);			
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		else if (message->params(0).compare("client") == 0) {
			auto ents = entity_list.GetClientList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				auto ent = entry.second;
				auto entity = entities->add_entities();
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		else if (message->params(0).compare("mob") == 0) {
			auto ents = entity_list.GetMobList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				auto ent = entry.second;
				auto entity = entities->add_entities();
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		else if (message->params(0).compare("mercenary") == 0) {
			auto ents = entity_list.GetMercList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				auto ent = entry.second;
				auto entity = entities->add_entities();
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		else if (message->params(0).compare("corpse") == 0) {
			auto ents = entity_list.GetCorpseList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				Corpse* ent = entry.second;
				auto entity = entities->add_entities();				
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		else if (message->params(0).compare("door") == 0) {
			auto ents = entity_list.GetDoorsList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				Doors* ent = entry.second;
				
				auto entity = entities->add_entities();
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		else if (message->params(0).compare("object") == 0) {
			auto ents = entity_list.GetObjectList();
			auto it = ents.begin();
			for (const auto &entry : ents) {
				if (entry.second == nullptr)
					continue;
				Object * ent = entry.second;
				auto entity = entities->add_entities();
				EncodeEntity(ent, entity);
			}

			size_t size = entities->ByteSizeLong();
			void *buffer = malloc(size);
			if (!entities->SerializeToArray(buffer, size)) {
				message->set_result("Failed to serialize entitiy result");
				SendCommandMessage(message, reply);
				return;
			}
			message->set_result("1");
			message->set_payload(buffer, size);
			SendCommandMessage(message, reply);
			return;
		}
		message->set_result("Usage: entitylist <type>. Types: npc, client, mob, mercenary, corpse, door, object");
		SendCommandMessage(message, reply);
		return;
	}

	message->set_result("Failed to parse command.");
	SendCommandMessage(message, reply);
	return;
}


// SendCommandMessage will send a channel message to NATS
void NatsManager::SendCommandMessage(eqproto::CommandMessage* message, const char* reply) {
	if (!connect())
		return;

	if (message->result().length() == 0)
		message->set_result("Failed to parse command.");


	std::string pubMessage;
	if (!message->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.command_message.in: failed to serialize to string", subscribedZoneName.c_str(), subscribedZoneInstance);
		return;
	}

	if (reply)
		s = natsConnection_Publish(conn, reply, (const void*)pubMessage.c_str(), pubMessage.length());
	else
		s = natsConnection_Publish(conn, StringFormat("zone.%s.%d.command_message.out", subscribedZoneName.c_str(), subscribedZoneInstance).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	

	if (s != NATS_OK) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.command_message.in: failed: %s", subscribedZoneName.c_str(), subscribedZoneInstance, nats_GetLastError(&s));
		return;
	}

	Log(Logs::General, Logs::NATS, "zone.%s.%d.command_message.in: %s (%s)", subscribedZoneName.c_str(), subscribedZoneInstance, message->command().c_str(), message->result().c_str());
}

//Unregister is called when a zone is being put to sleep or being swapped
void NatsManager::Unregister()
{
	if (!connect()) 
		return;

	if (subscribedZoneName.length() == 0)
		return;

	if (zoneCommandMessageSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneCommandMessageSub);
		zoneCommandMessageSub = NULL;
		if (s != NATS_OK) 
			Log(Logs::General, Logs::NATS, "unsubscribe from zoneCommandMessageSub failed: %s", nats_GetLastError(&s));
	}

	if (zoneInstanceCommandMessageSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneInstanceCommandMessageSub);
		zoneInstanceCommandMessageSub = NULL;
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "unsubscribe from zoneCommandMessageSub failed: %s", nats_GetLastError(&s));
	}

	if (zoneChannelMessageSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneChannelMessageSub);
		zoneChannelMessageSub = NULL;
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "unsubscribe from zoneCommandMessageSub failed: %s", nats_GetLastError(&s));
	}

	if (zoneInstanceChannelMessageSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneInstanceChannelMessageSub);
		zoneInstanceChannelMessageSub = NULL;
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "unsubscribe from zoneCommandMessageSub failed: %s", nats_GetLastError(&s));
	}

	
	SendAdminMessage(StringFormat("%s (%d) unregistered", subscribedZoneName.c_str(), subscribedZoneInstance).c_str());
	subscribedZoneName.clear();
	return;
}

void NatsManager::SendEvent(eqproto::OpCode op, uint32 entity_id, std::string pubMessage) {

	eqproto::Event* finalEvent = google::protobuf::Arena::CreateMessage<eqproto::Event>(&the_arena);
	finalEvent->set_payload(pubMessage.c_str());
	finalEvent->set_op(op);
	finalEvent->set_entity_id(entity_id);

	if (!finalEvent->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return;
	}

	s = natsConnection_Publish(conn, StringFormat("zone.%s.%d.entity.%d.event.out", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK)
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to send: %s", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op, nats_GetLastError(&s));

}

void NatsManager::ZoneSubscribe(const char* zonename, uint32 instance) {
	if (strcmp(subscribedZoneName.c_str(), zonename) == 0) 
		return;

	if (!connect()) 
		return;

	Unregister();
	
	subscribedZoneName = std::string(zonename);
	subscribedZoneInstance = instance;

	if (subscribedZoneInstance == 0) {
		s = natsConnection_SubscribeSync(&zoneChannelMessageSub, conn, StringFormat("zone.%s.channel_message.in", subscribedZoneName.c_str()).c_str());
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "zone.%s.channel_message.in: failed to subscribe: %s", subscribedZoneName.c_str(), nats_GetLastError(&s));

		s = natsSubscription_SetPendingLimits(zoneChannelMessageSub, -1, -1);
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "zone.%s.channel_message.in: failed to set pending limits: %s", subscribedZoneName.c_str(), nats_GetLastError(&s));

		s = natsConnection_SubscribeSync(&zoneCommandMessageSub, conn, StringFormat("zone.%s.command_message.in", subscribedZoneName.c_str()).c_str());
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "zone.%s.command_message.in: failed to subscribe: %s", subscribedZoneName.c_str(), nats_GetLastError(&s));

		s = natsSubscription_SetPendingLimits(zoneCommandMessageSub, -1, -1);
		if (s != NATS_OK)
			Log(Logs::General, Logs::NATS, "zone.%s.channel_message.in: failed to set pending limits: %s", subscribedZoneName.c_str(), nats_GetLastError(&s));
	}

	s = natsConnection_SubscribeSync(&zoneInstanceChannelMessageSub, conn, StringFormat("zone.%s.%d.channel_message.in", subscribedZoneName.c_str(), subscribedZoneInstance).c_str());
	if (s != NATS_OK)
		Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.in: failed to subscribe: %s", subscribedZoneName.c_str(), subscribedZoneInstance, nats_GetLastError(&s));
	
	s = natsSubscription_SetPendingLimits(zoneInstanceChannelMessageSub, -1, -1);
	if (s != NATS_OK) 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.in: failed to set pending limits: %s", subscribedZoneName.c_str(), subscribedZoneInstance, nats_GetLastError(&s));
		

	s = natsConnection_SubscribeSync(&zoneInstanceCommandMessageSub, conn, StringFormat("zone.%s.%d.command_message.in", subscribedZoneName.c_str()).c_str());
	if (s != NATS_OK) 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.command_message.in: failed to subscribe: %s", subscribedZoneName.c_str(), subscribedZoneInstance, nats_GetLastError(&s)); 
		
	s = natsSubscription_SetPendingLimits(zoneInstanceCommandMessageSub, -1, -1);
	if (s != NATS_OK) 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.in: failed to set pending limits: %s", subscribedZoneName.c_str(), subscribedZoneInstance, nats_GetLastError(&s));
	
	SendAdminMessage(StringFormat("%s (%d) registered", subscribedZoneName.c_str(), subscribedZoneInstance).c_str());
}


void NatsManager::SendAdminMessage(std::string adminMessage) {
	if (!connect()) 
		return;

	eqproto::ChannelMessage* message = google::protobuf::Arena::CreateMessage<eqproto::ChannelMessage>(&the_arena);
	message->set_message(adminMessage.c_str());
	std::string pubMessage;
	if (!message->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "global.admin_message.out: failed to serialize message to string");
		return;
	}

	s = natsConnection_Publish(conn, "global.admin_message.out", (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) {
		Log(Logs::General, Logs::NATS, "global.admin_message.out: failed: %s", nats_GetLastError(&s));
		return;
	}

	Log(Logs::General, Logs::NATS, "global.admin_message.out: %s", adminMessage.c_str());
}


bool NatsManager::connect() {
	auto ncs = natsConnection_Status(conn);

	if (ncs == CONNECTED) 
		return true;

	if (nats_timer.Enabled() && !nats_timer.Check()) 
		return false;

	natsOptions *opts = NULL;
	natsOptions_Create(&opts);
	natsOptions_SetMaxReconnect(opts, 0);
	natsOptions_SetReconnectWait(opts, 0);
	natsOptions_SetAllowReconnect(opts, false);
	//The timeout is going to cause a 100ms delay on all connected clients every X seconds (20s)
	//since this blocks the connection. It can be set lower or higher delay, 
	//but since NATS is a second priority I wanted server impact minimum.
	natsOptions_SetTimeout(opts, 100);
	std::string connection = StringFormat("nats://%s:%d", zoneConfig->NATSHost.c_str(), zoneConfig->NATSPort);
	if (zoneConfig->NATSHost.length() == 0) 
		connection = "nats://localhost:4222";

	natsOptions_SetURL(opts, connection.c_str());
	s = natsConnection_Connect(&conn, opts);
	natsOptions_Destroy(opts);
	if (s != NATS_OK) {
		Log(Logs::General, Logs::NATS, "failed to connect to %s: %s, retrying in 20s", connection.c_str(), nats_GetLastError(&s));
		conn = NULL;
		nats_timer.Enable();
		nats_timer.SetTimer(20000);
		return false;
	}

	Log(Logs::General, Logs::NATS, "connected to %s", connection.c_str());
	nats_timer.Disable();
	return true;
}

void NatsManager::Load()
{	
	if (!connect()) 
		return;	
	return;
}

bool NatsManager::isEntitySubscribed(const uint16 ID) {
	if (!connect()) 
		return false;
	return true;
}

void NatsManager::EncodeEntity(Entity * entity, eqproto::Entity * out)
{	
	out->set_id(entity->GetID());
	out->set_name(entity->GetName());
	out->set_spawn_timestamp(entity->GetSpawnTimeStamp());

	if (entity->IsMob()) {
		out->set_type(eqproto::EntityType::Mob);
		Mob* mob = entity->CastToMob();
		out->set_hp(mob->GetHP());
		out->set_level(mob->GetLevel());
		eqproto::Position* pos = google::protobuf::Arena::CreateMessage<eqproto::Position>(&the_arena);
		pos->set_x(mob->GetX());
		pos->set_y(mob->GetY());
		pos->set_z(mob->GetZ());
		pos->set_h(mob->GetHeading());
		out->set_allocated_position(pos);

		out->set_race(mob->GetRace());
		out->set_class_(mob->GetClass());
		out->set_max_hp(mob->GetMaxHP());
		//TODO: SpecialAbility
		//TODO: AuraInfo
		out->set_lastname(mob->GetLastName());
		out->set_gender(static_cast<eqproto::GenderType>(mob->GetGender()));
		out->set_bodytype(static_cast<eqproto::BodyType>(mob->GetBodyType()));
		out->set_deity(static_cast<eqproto::DeityType>(mob->GetDeity()));
		out->set_npctype_id(mob->GetNPCTypeID());
		out->set_size(mob->GetSize());
		out->set_runspeed(mob->GetRunspeed());
		//TODO: out->set_light(mob->GetLight)
		out->set_texture(mob->GetTexture());
		out->set_helmtexture(mob->GetHelmTexture());
		out->set_ac(mob->GetAC());
		out->set_atk(mob->GetATK());
		out->set_str(mob->GetSTR());
		out->set_sta(mob->GetSTA());
		out->set_dex(mob->GetDEX());
		out->set_agi(mob->GetAGI());
		out->set_int_(mob->GetINT());
		out->set_wis(mob->GetWIS());
		out->set_cha(mob->GetCHA());
		out->set_haircolor(mob->GetHairColor());
		out->set_beardcolor(mob->GetBeardColor());
		out->set_eyecolor1(mob->GetEyeColor1());
		out->set_eyecolor2(mob->GetEyeColor2());
		
		if (mob->IsClient()) {
			out->set_type(eqproto::EntityType::Client);
			Client *client = entity->CastToClient();
		}
		if (mob->IsNPC()) {
			out->set_type(eqproto::EntityType::NPC);
			NPC *npc = entity->CastToNPC();

			if (npc->IsMerc()) {
				out->set_type(eqproto::EntityType::Mercenary);
			}
		}
		if (mob->IsCorpse()) {
			out->set_type(eqproto::EntityType::Corpse);

		}
		if (mob->IsBeacon()) {
			out->set_type(eqproto::EntityType::Beacon);
		}
		if (mob->IsEncounter()) {
			out->set_type(eqproto::EntityType::Encounter);
		}
	}

	if (entity->IsDoor()) {
		out->set_type(eqproto::EntityType::Door);
		Doors * door = entity->CastToDoors();
		eqproto::Position* pos = google::protobuf::Arena::CreateMessage<eqproto::Position>(&the_arena);

		auto door_pos = door->GetPosition();
		pos->set_x(door_pos.x);
		pos->set_y(door_pos.y);
		pos->set_z(door_pos.z);
		pos->set_h(door_pos.w);
		out->set_allocated_position(pos);
		
		out->set_door_db_id(door->GetDoorDBID());
		out->set_door_id(door->GetDoorID());
		out->set_door_name(door->GetName());
		out->set_door_incline(door->GetIncline());
		out->set_door_opentype(door->GetOpenType());
		out->set_door_guild_id(door->GetGuildID());
		out->set_door_lockpick(door->GetLockpick());
		out->set_door_keyitem(door->GetKeyItem());
		out->set_door_nokeyring(door->GetNoKeyring());
		out->set_door_trigger_door_id(door->GetTriggerDoorID());
		out->set_door_trigger_type(door->GetTriggerType());
		out->set_door_param(door->GetDoorParam());
		out->set_door_invert_state(door->GetInvertState());
		out->set_door_disable_timer(door->GetDisableTimer());
		out->set_door_is_open(door->IsDoorOpen());
		//TODO: expose door destination properties
		//out->set_door_destination_zone(door->)
		out->set_door_is_ldon(door->IsLDoNDoor());
		out->set_door_client_version_mask(door->GetClientVersionMask());

	}

	if (entity->IsObject()) {
		out->set_type(eqproto::EntityType::Object);
	}

	if (entity->IsTrap()) {
		out->set_type(eqproto::EntityType::Trap);
	}
}



void NatsManager::OnDeathEvent(Death_Struct* d) {
	if (!connect()) 
		return;
	
	if (d == NULL) 
		return;

	if (!isEntityEventAllEnabled && !isEntitySubscribed(d->spawn_id)) 
		return;

	auto op = eqproto::OP_Death;
	std::string pubMessage;
	eqproto::DeathEvent* event = google::protobuf::Arena::CreateMessage<eqproto::DeathEvent>(&the_arena);

	event->set_spawn_id(d->spawn_id);
	event->set_killer_id(d->killer_id);
	event->set_bind_zone_id(d->bindzoneid);
	event->set_spell_id(d->spell_id);
	event->set_attack_skill_id(d->attack_skill);
	event->set_damage(d->damage);

	if (!event->SerializeToString(&pubMessage)) { 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, d->spawn_id, op);
		return; 
	}

	SendEvent(op, d->spawn_id, pubMessage);
	SendEvent(op, d->killer_id, pubMessage);
}


void NatsManager::OnChannelMessageEvent(uint32 entity_id, ChannelMessage_Struct* cm) {
	if (!connect()) 
		return;

	if (entity_id == 0) 
		return;

	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;
	eqproto::ChannelMessageEvent* event = google::protobuf::Arena::CreateMessage<eqproto::ChannelMessageEvent>(&the_arena);
	
	event->set_target_name(cm->targetname);
	event->set_sender(cm->sender);
	event->set_language(cm->language);	
	event->set_number((eqproto::MessageType)cm->chan_num);
	event->set_cm_unknown4(*cm->cm_unknown4);
	event->set_skill_in_language(cm->skill_in_language);
	event->set_message(cm->message);

	if (!event->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.channel_message.out: failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance);
		return; 
	}

}

void NatsManager::OnSpecialMessageEvent(uint32 entity_id, SpecialMesg_Struct* sm) {	
	if (!connect())
		return;
	if (entity_id == 0)
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id))
		return;

	auto op = eqproto::OP_SpecialMesg;

	std::string pubMessage;
	eqproto::SpecialMessageEvent* event = google::protobuf::Arena::CreateMessage<eqproto::SpecialMessageEvent>(&the_arena);
	event->set_header(sm->header);
	event->set_number(static_cast<eqproto::MessageType>(sm->msg_type));
	event->set_target_spawn_id(sm->target_spawn_id);
	event->set_sayer(sm->sayer);
	event->set_unknown12(*sm->unknown12);
	event->set_message(sm->message);
	
	if (!event->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return;
	}

	SendEvent(op, entity_id, pubMessage);
}

void NatsManager::OnEntityEvent(const EmuOpcode op, uint32 entity_id, uint32 target_id) {
	if (!connect()) 
		return;

	if (entity_id == 0) 
		return;

	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;

	eqproto::EntityEvent* event = google::protobuf::Arena::CreateMessage<eqproto::EntityEvent>(&the_arena);
	event->set_entity_id(entity_id);
	event->set_target_id(target_id);

	if (!event->SerializeToString(&pubMessage)) { 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return;
	}
	
	eqproto::Event* finalEvent = google::protobuf::Arena::CreateMessage<eqproto::Event>(&the_arena);
	finalEvent->set_payload(pubMessage.c_str());


	auto eqop = eqproto::OP_Camp;
	if (op == OP_Camp) {
		eqop = eqproto::OP_Camp;
	}
	else if (op == OP_Assist) {
		eqop = eqproto::OP_Assist;
	}
	else { 
		Log(Logs::General, Logs::NATS, "unhandled op type passed: %i", op); 
		return; 
	}
	SendEvent(eqop, entity_id, pubMessage);	
	SendEvent(eqop, target_id, pubMessage);
}


void NatsManager::OnSpawnEvent(const EmuOpcode op, uint32 entity_id, Spawn_Struct *spawn) {
	if (!connect()) 
		return;

	if (entity_id == 0) 
		return;

	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;

	eqproto::SpawnEvent* event = google::protobuf::Arena::CreateMessage<eqproto::SpawnEvent>(&the_arena);

	event->set_unknown0000(spawn->unknown0000);
	event->set_gm(spawn->gm);
	event->set_unknown0003(spawn->unknown0003);
	event->set_aaitle(spawn->aaitle);
	event->set_unknown0004(spawn->unknown0004);
	event->set_anon(spawn->anon);
	event->set_face(spawn->face);
	event->set_name(spawn->name);
	event->set_deity(spawn->deity);
	event->set_unknown0073(spawn->unknown0073);
	event->set_size(spawn->size);
	event->set_unknown0079(spawn->unknown0079);
	event->set_npc(spawn->NPC);
	event->set_invis(spawn->invis);
	event->set_haircolor(spawn->haircolor);
	event->set_curhp(spawn->curHp);
	event->set_max_hp(spawn->max_hp);
	event->set_findable(spawn->findable);
	event->set_unknown0089(*spawn->unknown0089);
	event->set_deltaheading(spawn->deltaHeading);
	event->set_x(spawn->x);
	event->set_padding0054(spawn->padding0054);
	event->set_y(spawn->y);
	event->set_animation(spawn->animation);
	event->set_padding0058(spawn->padding0058);
	event->set_z(spawn->z);
	event->set_deltay(spawn->deltaY);
	event->set_deltax(spawn->deltaX);
	event->set_heading(spawn->heading);
	event->set_padding0066(spawn->padding0066);
	event->set_deltaz(spawn->deltaZ);
	event->set_padding0070(spawn->padding0070);
	event->set_eyecolor1(spawn->eyecolor1);
	event->set_unknown0115(*spawn->unknown0115);
	event->set_standstate(spawn->StandState);
	event->set_drakkin_heritage(spawn->drakkin_heritage);
	event->set_drakkin_tattoo(spawn->drakkin_tattoo);
	event->set_drakkin_details(spawn->drakkin_details);
	event->set_showhelm(spawn->showhelm);
	event->set_unknown0140(*spawn->unknown0140);
	event->set_is_npc(spawn->is_npc);
	event->set_hairstyle(spawn->hairstyle);
	event->set_beard(spawn->beard);
	event->set_unknown0147(*spawn->unknown0147);
	event->set_level(spawn->level);
	event->set_playerstate(spawn->PlayerState);
	event->set_beardcolor(spawn->beardcolor);
	event->set_suffix(spawn->suffix);
	event->set_petownerid(spawn->petOwnerId);
	event->set_guildrank(spawn->guildrank);
	event->set_unknown0194(*spawn->unknown0194);
	
	/*auto texture = eqproto::Texture();
	texture.set_elitemodel(spawn->equipment.Arms.EliteModel);
	texture.set_herosforgemodel(spawn->equipment.Arms.HerosForgeModel);
	texture.set_material(spawn->equipment.Arms.Material);
	texture.set_unknown1(spawn->equipment.Arms.Unknown1);
	texture.set_unknown2(spawn->equipment.Arms.Unknown2);
	event->set_allocated_equipment(textureProfile);*/
	event->set_runspeed(spawn->runspeed);
	event->set_afk(spawn->afk);
	event->set_guildid(spawn->guildID);
	event->set_title(spawn->title);
	event->set_unknown0274(spawn->unknown0274);
	event->set_set_to_0xff(*spawn->set_to_0xFF);
	event->set_helm(spawn->helm);
	event->set_race(spawn->race);
	event->set_unknown0288(spawn->unknown0288);
	event->set_lastname(spawn->lastName);
	event->set_walkspeed(spawn->walkspeed);
	event->set_unknown0328(spawn->unknown0328);
	event->set_is_pet(spawn->is_pet);
	event->set_light(spawn->light);
	event->set_class_(spawn->class_);
	event->set_eyecolor2(spawn->eyecolor2);
	event->set_flymode(spawn->flymode);
	event->set_gender(spawn->gender);
	event->set_bodytype(spawn->bodytype);
	event->set_unknown0336(*spawn->unknown0336);
	event->set_equip_chest2(spawn->equip_chest2);
	event->set_mount_color(spawn->mount_color);
	event->set_spawnid(spawn->spawnId);
	event->set_ismercenary(spawn->IsMercenary);
	//event->set_equipment_tint(spawn->equipment_tint);
	event->set_lfg(spawn->lfg);
	event->set_destructibleobject(spawn->DestructibleObject);
	event->set_destructiblemodel(spawn->DestructibleModel);
	event->set_destructiblename2(spawn->DestructibleName2);
	event->set_destructiblestring(spawn->DestructibleString);
	event->set_destructibleappearance(spawn->DestructibleAppearance);
	event->set_destructibleunk1(spawn->DestructibleUnk1);
	event->set_destructibleid1(spawn->DestructibleID1);
	event->set_destructibleid2(spawn->DestructibleID2);
	event->set_destructibleid3(spawn->DestructibleID3);
	event->set_destructibleid4(spawn->DestructibleID4);
	event->set_destructibleunk2(spawn->DestructibleUnk2);
	event->set_destructibleunk3(spawn->DestructibleUnk3);
	event->set_destructibleunk4(spawn->DestructibleUnk4);
	event->set_destructibleunk5(spawn->DestructibleUnk5);
	event->set_destructibleunk6(spawn->DestructibleUnk6);
	event->set_destructibleunk7(spawn->DestructibleUnk7);
	event->set_destructibleunk8(spawn->DestructibleUnk8);
	event->set_destructibleunk9(spawn->DestructibleUnk9);
	event->set_targetable_with_hotkey(spawn->targetable_with_hotkey);
	event->set_show_name(spawn->show_name);

	if (!event->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return;
	}

	auto eqop = eqproto::OP_ZoneEntry;
	if (op == OP_ZoneEntry)
		eqop = eqproto::OP_ZoneEntry;
	else if (op == OP_NewSpawn)
		eqop = eqproto::OP_NewSpawn;
	else {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) unhandled opcode passed", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return;
	}
	SendEvent(eqop, entity_id, pubMessage);
}


void NatsManager::OnWearChangeEvent(uint32 entity_id, WearChange_Struct *wc) {
	if (!connect()) 
		return;
	if (entity_id == 0) 
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	auto op = eqproto::OP_WearChange;
	std::string pubMessage;
	eqproto::WearChangeEvent* event = google::protobuf::Arena::CreateMessage<eqproto::WearChangeEvent>(&the_arena);
	event->set_spawn_id(wc->spawn_id);
	event->set_material(wc->material);
	event->set_unknown06(wc->unknown06);
	event->set_elite_material(wc->elite_material);
	event->set_hero_forge_model(wc->hero_forge_model);
	event->set_unknown18(wc->unknown18);
	//event->set_color(wc->color); //tint
	event->set_wear_slot_id(wc->wear_slot_id);

	if (!event->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);		
		return; 
	}

	SendEvent(op, entity_id, pubMessage);	
}

void NatsManager::OnDeleteSpawnEvent(uint32 entity_id, DeleteSpawn_Struct *ds) {
	if (!connect()) 
		return;
	if (entity_id == 0) 
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;
	auto op = eqproto::OP_DeleteSpawn;
	eqproto::DeleteSpawnEvent* event = google::protobuf::Arena::CreateMessage<eqproto::DeleteSpawnEvent>(&the_arena);

	event->set_spawn_id(ds->spawn_id);
	event->set_decay(ds->Decay);

	if (!event->SerializeToString(&pubMessage)) { 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return; 
	}
	SendEvent(op, entity_id, pubMessage);
}

void NatsManager::OnHPEvent(const EmuOpcode op, uint32 entity_id, uint32 cur_hp, uint32 max_hp) {
	if (!connect()) 
		return;
	if (entity_id == 0) 
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;
	if (cur_hp == max_hp) 
		return;
	std::string pubMessage;


	eqproto::HPEvent* event = google::protobuf::Arena::CreateMessage<eqproto::HPEvent>(&the_arena);
	event->set_spawn_id(entity_id);
	event->set_cur_hp(cur_hp);
	event->set_max_hp(max_hp);

	if (!event->SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return;
	}
	auto eqop = eqproto::OP_MobHealth;
	if (op == OP_MobHealth)
		eqop = eqproto::OP_MobHealth;
	else if (op == OP_HPUpdate)
		eqop = eqproto::OP_HPUpdate;
	else { 
		Log(Logs::General, Logs::NATS, "unhandled op type passed: %i", op); 
		return; 
	}
	SendEvent(eqop, entity_id, pubMessage);
}

void NatsManager::OnDamageEvent(uint32 entity_id, CombatDamage_Struct *cd) {
	if (!connect()) 
		return;
	if (entity_id == 0) 
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;

	eqproto::DamageEvent* event = google::protobuf::Arena::CreateMessage<eqproto::DamageEvent>(&the_arena);
	event->set_target(cd->target);
	event->set_source(cd->source);
	event->set_type(cd->type);
	event->set_spellid(cd->spellid);
	event->set_damage(cd->damage);
	event->set_force(cd->force);
	event->set_meleepush_xy(cd->hit_heading);
	event->set_meleepush_z(cd->hit_pitch);

	auto op = eqproto::OP_Damage;
	
	if (!event->SerializeToString(&pubMessage)) { 
		Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); 
		return; 
	}

	SendEvent(op, entity_id, pubMessage);
}

void NatsManager::OnClientUpdateEvent(uint32 entity_id, PlayerPositionUpdateServer_Struct * spu) {

	auto op = eqproto::OP_ClientUpdate;
	if (!connect()) 
		return;
	if (entity_id == 0) 
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;

	eqproto::PlayerPositionUpdateEvent* event = google::protobuf::Arena::CreateMessage<eqproto::PlayerPositionUpdateEvent>(&the_arena);
	event->set_spawn_id(spu->spawn_id);
	event->set_delta_heading(spu->delta_heading);
	event->set_x_pos(spu->x_pos);
	event->set_padding0002(spu->padding0002);
	event->set_y_pos(spu->y_pos);
	event->set_animation(spu->animation);
	event->set_padding0006(spu->padding0006);
	event->set_z_pos(spu->z_pos);
	event->set_delta_y(spu->delta_y);
	event->set_delta_x(spu->delta_x);
	event->set_heading(spu->heading);
	event->set_padding0014(spu->padding0014);
	event->set_delta_z(spu->delta_z);
	event->set_padding0018(spu->padding0018);
	if (!event->SerializeToString(&pubMessage)) { 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return; 
	}

	SendEvent(op, entity_id, pubMessage);
}


void NatsManager::OnAnimationEvent(uint32 entity_id, Animation_Struct *anim) {
	if (!connect()) 
		return;
	if (entity_id == 0) 
		return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) 
		return;

	std::string pubMessage;

	auto op = eqproto::OP_Animation;

	eqproto::AnimationEvent* event = google::protobuf::Arena::CreateMessage<eqproto::AnimationEvent>(&the_arena);
	event->set_spawnid(anim->spawnid);
	event->set_speed(anim->speed);
	event->set_action(anim->action);	

	if (!event->SerializeToString(&pubMessage)) { 
		Log(Logs::General, Logs::NATS, "zone.%s.%d.entity.%d.event.out: (OP: %d) failed to serialize message to string", subscribedZoneName.c_str(), subscribedZoneInstance, entity_id, op);
		return; 
	}
	SendEvent(op, entity_id, pubMessage);
}