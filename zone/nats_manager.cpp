#include "entity.h"
#include "mob.h"
//#include "client.h" //map error
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
#include "../common/message.pb.h"
#endif

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
	if (!connect()) return;
	natsMsg *msg = NULL;
	std::string pubMessage;

	s = NATS_OK;
	for (int count = 0; (s == NATS_OK) && count < 5; count++)
	{
		s = natsSubscription_NextMsg(&msg, zoneCommandMessageSub, 1);
		if (s != NATS_OK) break;
		Log(Logs::General, Logs::World_Server, "NATS Got Command Message '%s'", natsMsg_GetData(msg));
		eqproto::CommandMessage message;

		if (!message.ParseFromString(natsMsg_GetData(msg))) {
			Log(Logs::General, Logs::World_Server, "Failed to marshal");
			natsMsg_Destroy(msg);
			continue;
		}
		if (message.command().compare("npctypespawn") == 0) {
			if (message.params_size() < 2) {
				message.set_result("Usage: !npctypespawn <npctypeid> <factionid> <x> <y> <z> <h>.");
			} else {

				uint32 npctypeid = atoi(message.params(0).c_str());
				uint32 factionid = atoi(message.params(1).c_str());
				float x = atof(message.params(2).c_str());
				float y = atof(message.params(3).c_str());
				float z = atof(message.params(4).c_str());
				float h = atof(message.params(5).c_str());
				auto position = glm::vec4(x, y, z, h);
				const NPCType* tmp = 0;		

				/*if (!(tmp = database.LoadNPCTypesData(npctypeid))) {
					message.set_result(StringFormat("NPC Type %i not found", npctypeid));
				} else {
					//tmp->fixedZ = 1;
					
					auto npc = new NPC(tmp, 0, position, FlyMode3);
					if (npc && factionid >0)
						npc->SetNPCFactionID(factionid);
					npc->AddLootTable();
					entity_list.AddNPC(npc);
					message.set_result("Created NPC successfully.");
				}	
				*/
			}
		}

		if (message.command().compare("spawn") == 0) {
			if (message.params_size() < 5) {
				message.set_result("Usage: npctypespawn <x> <y> <z> <h> name race  level material hp gender class priweapon secweapon merchantid bodytype.");
			}
			else {

				float x = atof(message.params(0).c_str());
				float y = atof(message.params(1).c_str());
				float z = atof(message.params(2).c_str());
				float h = atof(message.params(3).c_str());
				auto position = glm::vec4(x, y, z, h);

				std::string argumentString;
				for (int i = 4; i < message.params_size(); i++) {
					argumentString.append(StringFormat(" %s", message.params(i).c_str()));
				}
				
				NPC* npc = NPC::SpawnNPC(argumentString.c_str(), position, NULL);
				if (!npc) {
					message.set_result("Format: #spawn name race level material hp gender class priweapon secweapon merchantid bodytype - spawns a npc those parameters.");
				}
				else {
					message.set_result(StringFormat("%u", npc->GetID()));
				}
			}
		}

		if (message.command().compare("moveto") == 0) {
			if (message.params_size() < 5) {
				message.set_result("Usage: moveto <entityid> <x> <y> <z> <h>.");
			}
			else {
				uint16 entityid = atoi(message.params(0).c_str());
				float x = atof(message.params(1).c_str());
				float y = atof(message.params(2).c_str());
				float z = atof(message.params(3).c_str());
				float h = atof(message.params(4).c_str());
				auto position = glm::vec4(x, y, z, h);

				auto npc = entity_list.GetNPCByID(entityid);
				if (!npc) {
					message.set_result("Invalid entity ID passed, or not an npc, etc");
				}
				else {
					npc->MoveTo(position, true);
					message.set_result("OK");
				}
			}
		}

		if (message.command().compare("attack") == 0) {
			if (message.params_size() < 3) {
				message.set_result("Usage: attack <entityid> <targetentityid> <hateamount>.");
			}
			else {
				uint16 entityID = atoi(message.params(0).c_str());
				uint16 targetEntityID = atoi(message.params(1).c_str());
				uint32 hateAmount = atoi(message.params(2).c_str());
				
				auto npc = entity_list.GetNPCByID(entityID);
				if (!npc) {
					message.set_result("Invalid entity ID passed, or not an npc, etc");
				}
				else {
					auto mob = entity_list.GetMobID(targetEntityID);
					if (!mob) {
						message.set_result("Invalid target entitiy ID passed, or not a mob, etc");
					}
					else {
						npc->AddToHateList(mob, hateAmount);
						message.set_result("OK");
					}					
				}
			}
		}

		if (message.command().compare("entitylist") == 0) {
			std::string entityPayload;
			if (message.params_size() < 1) {
				message.set_result("Usage: entitylist <typeid>.");
			}
			else {
				auto entities = eqproto::Entities();
				if (message.params(0).compare("npc") == 0) {
					auto npcs = entity_list.ListNPCs();
					auto it = npcs.begin();
					for (const auto &entry : npcs) {
						auto entity = entities.add_entities();
						entity->set_id(entry.second->GetID());
						entity->set_type(1);
						entity->set_name(entry.second->GetName());
					}

					if (!entities.SerializeToString(&entityPayload)) {
						message.set_result("Failed to serialized entitiy result");
					}
					else {						
						message.set_payload(entityPayload.c_str());
					}
				}
				/*else if (message.params(0).compare("client") == 0) {
					auto clients = entity_list.ListClients();
					auto it = clients.begin();
					for (const auto &entry : clients) {
						auto entity = entities.add_entities();
						entity->set_id(entry.second->GetID());
						entity->set_type(0);
						entity->set_name(entry.second->GetName());
					}

					if (!entities.SerializeToString(&entityMessage)) {
						message.set_result("Failed to serialized entitiy result");
					}
					else {
						message.set_result(entityMessage.c_str());
					}
				}*/
				else {
					message.set_result("Usage: entitylist <typeid>.");
				}
				
			}
		}

		
		if (message.result().length() < 1) {
			message.set_result("Failed to parse command.");
		}

		if (!message.SerializeToString(&pubMessage)) {
			Log(Logs::General, Logs::World_Server, "NATS Failed to serialize command message to string");
			return;
		}

		s = natsConnection_PublishString(conn, natsMsg_GetReply(msg), pubMessage.c_str());
		if (s != NATS_OK) {
			Log(Logs::General, Logs::World_Server, "NATS Failed to send CommandMessageEvent");
			return;
		}
	}
}

//Unregister is called when a zone is being put to sleep or being swapped
void NatsManager::Unregister()
{
	if (!connect()) return;
	if (zoneCommandMessageSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneCommandMessageSub);
		zoneCommandMessageSub = NULL;
		if (s != NATS_OK) Log(Logs::General, Logs::NATS, "unsubscribe from zoneCommandMessageSub failed: %s", nats_GetLastError(&s));
	}

	if (zoneEntityEventSubscribeAllSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneEntityEventSubscribeAllSub);
		zoneEntityEventSubscribeAllSub = NULL;
		if (s != NATS_OK) Log(Logs::General, Logs::NATS, "unsubscribe from zoneEntityEventSubscribeAllSub failed: %s", nats_GetLastError(&s));
	}
	if (zoneEntityEventSubscribeSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneEntityEventSubscribeSub);
		zoneEntityEventSubscribeSub = NULL;
		if (s != NATS_OK) Log(Logs::General, Logs::NATS, "unsubscribe from zoneEntityEventSubscribeSub failed: %s", nats_GetLastError(&s));
	}

	if (zoneEntityEventListSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneEntityEventListSub);
		zoneEntityEventListSub = NULL;
		if (s != NATS_OK) Log(Logs::General, Logs::NATS, "unsubscribe from zoneEntityEventListSub failed: %s", nats_GetLastError(&s));
	}

	if (zoneEntityEventSub != NULL) {
		s = natsSubscription_Unsubscribe(zoneEntityEventSub);
		zoneEntityEventSub = NULL;
		if (s != NATS_OK) Log(Logs::General, Logs::NATS, "unsubscribe from zoneEntityEventSub failed: %s", nats_GetLastError(&s));
	}

	Log(Logs::General, Logs::NATS, "unsubscribed from %s", subscribedZonename.c_str());	
	subscribedZonename.clear();
	return;
}

void NatsManager::ZoneSubscribe(const char* zonename) {
	if (strcmp(subscribedZonename.c_str(), zonename) == 0) return;
	if (!connect()) return;
	Unregister();
	
	subscribedZonename = std::string(zonename);
	
	s = natsConnection_SubscribeSync(&zoneChannelMessageSub, conn,  StringFormat("zone.%s.channel_message", subscribedZonename.c_str()).c_str());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to subscribe to zoneChannelMessageSub %s", nats_GetLastError(&s));
	s = natsSubscription_SetPendingLimits(zoneChannelMessageSub, -1, -1);
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to set pending limits to zoneChannelMessageSub %s", nats_GetLastError(&s));

	s = natsConnection_SubscribeSync(&zoneCommandMessageSub, conn, StringFormat("zone.%s.command_message", subscribedZonename.c_str()).c_str());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to subscribe to zoneCommandMessageSub %s", nats_GetLastError(&s));
	s = natsSubscription_SetPendingLimits(zoneCommandMessageSub, -1, -1);
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to set pending limits to zoneCommandMessageSub %s", nats_GetLastError(&s));

	s = natsConnection_SubscribeSync(&zoneEntityEventSubscribeAllSub, conn, StringFormat("zone.%s.entity.event_subscribe.all", subscribedZonename.c_str()).c_str());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to subscribe to zoneEntityEventSubscribeAllSub %s", nats_GetLastError(&s));
	s = natsSubscription_SetPendingLimits(zoneEntityEventSubscribeAllSub, -1, -1);
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to set pending limits to zoneEntityEventSubscribeAllSub %s", nats_GetLastError(&s));

	s = natsConnection_SubscribeSync(&zoneEntityEventSubscribeAllSub, conn, StringFormat("zone.%s.entity.event_subscribe.all", subscribedZonename.c_str()).c_str());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to subscribe to zoneEntityEventSubscribeAllSub %s", nats_GetLastError(&s));
	s = natsSubscription_SetPendingLimits(zoneEntityEventSubscribeAllSub, -1, -1);
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "failed to set pending limits to zoneEntityEventSubscribeAllSub %s", nats_GetLastError(&s));

	Log(Logs::General, Logs::NATS, "subscribed to %s", subscribedZonename.c_str());
}


void NatsManager::SendAdminMessage(std::string adminMessage) {
	if (!connect()) return;

	eqproto::ChannelMessage message;
	message.set_message(adminMessage.c_str());
	std::string pubMessage;
	if (!message.SerializeToString(&pubMessage)) {
		Log(Logs::General, Logs::NATS, "Failed to serialize message to string");
		return;
	}
	s = natsConnection_PublishString(conn, "NATS AdminMessage", pubMessage.c_str());
	if (s != NATS_OK) {
		Log(Logs::General, Logs::NATS, "Failed to SendAdminMessage");
	}
	Log(Logs::General, Logs::NATS, "AdminMessage: %s", adminMessage.c_str());
}


bool NatsManager::connect() {
	auto ncs = natsConnection_Status(conn);
	if (ncs == CONNECTED) return true;
	if (nats_timer.Enabled() && !nats_timer.Check()) return false;
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
	if (zoneConfig->NATSHost.length() == 0) connection = "nats://localhost:4222";
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
	if (!connect()) return;	
	return;
}

/*
void NatsManager::OnEntityEvent(const EmuOpcode op, Entity *ent, Entity *target) {
	if (ent == NULL) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(ent->GetID())) {
		return;
	}


	if (!conn) {
		Log(Logs::General, Logs::NATS, "OnChannelMessage failed, no connection to NATS");
		return;
	}

	eqproto::EntityEvent event;
	event.set_op(eqproto::OpCode(op));
	eqproto::Entity entity;
	entity.set_id(ent->GetID());
	entity.set_name(ent->GetName());

	if (ent->IsClient()) {
		entity.set_type(1);
	}
	else if (ent->IsNPC()) {
		entity.set_type(2);
	}

	auto position = eqproto::Position();
	if (ent->IsMob()) {
		auto mob = ent->CastToMob();
		entity.set_hp(mob->GetHP());
		entity.set_level(mob->GetLevel());
		entity.set_name(mob->GetName());
		position.set_x(mob->GetX());
		position.set_y(mob->GetY());
		position.set_z(mob->GetZ());
		position.set_h(mob->GetHeading());
		entity.set_race(mob->GetRace());
		entity.set_class_(mob->GetClass());		
	}

	auto targetEntity = eqproto::Entity();
	auto targetPosition = eqproto::Position();
	if (target != NULL && target->IsMob()) {
		if (target->IsClient()) {
			targetEntity.set_type(1);
		}
		else if (target->IsNPC()) {
			targetEntity.set_type(2);
		}
		auto mob = target->CastToMob();
		targetEntity.set_hp(mob->GetHP());
		targetEntity.set_level(mob->GetLevel());
		targetEntity.set_name(mob->GetName());
		targetPosition.set_x(mob->GetX());
		targetPosition.set_y(mob->GetY());
		targetPosition.set_z(mob->GetZ());
		targetPosition.set_h(mob->GetHeading());
		targetEntity.set_race(mob->GetRace());
		targetEntity.set_class_(mob->GetClass());
	}
	
	entity.set_allocated_position(&position);
	targetEntity.set_allocated_position(&targetPosition);
	event.set_allocated_entity(&entity);
	event.set_allocated_target(&targetEntity);

	
	std::string pubMessage;
	bool isSerialized = event.SerializeToString(&pubMessage);	
	if (!isSerialized) Log(Logs::General, Logs::NATS, "Failed to serialize message to string");
	Log(Logs::General, Logs::NATS, "Event: %d", op);
	
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), ent->GetID()).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
	entity.release_name();
	targetEntity.release_name();
	entity.release_position();
	targetEntity.release_position();
	event.release_entity();
	event.release_target();
	return;
}*/

bool NatsManager::isEntitySubscribed(const uint16 ID) {
	if (!connect()) return false;
	return false;
}


void NatsManager::OnDeathEvent(Death_Struct* d) {
	if (!connect()) return;
	if (d == NULL) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(d->spawn_id)) return;

	std::string pubMessage;
	auto event = eqproto::DeathEvent();

	event.set_spawn_id(d->spawn_id);
	event.set_killer_id(d->killer_id);
	event.set_bind_zone_id(d->bindzoneid);
	event.set_spell_id(d->spell_id);
	event.set_attack_skill_id(d->attack_skill);
	event.set_damage(d->damage);

	
	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }	

	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_Death);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), d->spawn_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}


void NatsManager::OnChannelMessageEvent(uint32 entity_id, ChannelMessage_Struct* cm) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::ChannelMessageEvent();

	event.set_target_name(cm->targetname);
	event.set_sender(cm->sender);
	event.set_language(cm->language);
	event.set_chan_num(cm->chan_num);
	event.set_cm_unknown4(*cm->cm_unknown4);
	event.set_skill_in_language(cm->skill_in_language);
	event.set_message(cm->message);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_ChannelMessage);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}

void NatsManager::OnEntityEvent(const EmuOpcode op, uint32 entity_id, uint32 target_id) {
	if (!connect()) return;
	if (entity_id == 0) return;

	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::EntityEvent();

	event.set_entity_id(entity_id);
	event.set_target_id(target_id);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	if (op == OP_Camp) finalEvent.set_op(eqproto::OP_Camp);
	else if (op == OP_Assist) finalEvent.set_op(eqproto::OP_Assist);
	else { Log(Logs::General, Logs::NATS, "unhandled op type passed: %i", op); return; }
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}


void NatsManager::OnSpawnEvent(const EmuOpcode op, uint32 entity_id, Spawn_Struct *spawn) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::SpawnEvent();

	event.set_unknown0000(spawn->unknown0000);
	event.set_gm(spawn->gm);
	event.set_unknown0003(spawn->unknown0003);
	event.set_aaitle(spawn->aaitle);
	event.set_unknown0004(spawn->unknown0004);
	event.set_anon(spawn->anon);
	event.set_face(spawn->face);
	event.set_name(spawn->name);
	event.set_deity(spawn->deity);
	event.set_unknown0073(spawn->unknown0073);
	event.set_size(spawn->size);
	event.set_unknown0079(spawn->unknown0079);
	event.set_npc(spawn->NPC);
	event.set_invis(spawn->invis);
	event.set_haircolor(spawn->haircolor);
	event.set_curhp(spawn->curHp);
	event.set_max_hp(spawn->max_hp);
	event.set_findable(spawn->findable);
	event.set_unknown0089(*spawn->unknown0089);
	event.set_deltaheading(spawn->deltaHeading);
	event.set_x(spawn->x);
	event.set_padding0054(spawn->padding0054);
	event.set_y(spawn->y);
	event.set_animation(spawn->animation);
	event.set_padding0058(spawn->padding0058);
	event.set_z(spawn->z);
	event.set_deltay(spawn->deltaY);
	event.set_deltax(spawn->deltaX);
	event.set_heading(spawn->heading);
	event.set_padding0066(spawn->padding0066);
	event.set_deltaz(spawn->deltaZ);
	event.set_padding0070(spawn->padding0070);
	event.set_eyecolor1(spawn->eyecolor1);
	event.set_unknown0115(*spawn->unknown0115);
	event.set_standstate(spawn->StandState);
	event.set_drakkin_heritage(spawn->drakkin_heritage);
	event.set_drakkin_tattoo(spawn->drakkin_tattoo);
	event.set_drakkin_details(spawn->drakkin_details);
	event.set_showhelm(spawn->showhelm);
	event.set_unknown0140(*spawn->unknown0140);
	event.set_is_npc(spawn->is_npc);
	event.set_hairstyle(spawn->hairstyle);
	event.set_beard(spawn->beard);
	event.set_unknown0147(*spawn->unknown0147);
	event.set_level(spawn->level);
	event.set_playerstate(spawn->PlayerState);
	event.set_beardcolor(spawn->beardcolor);
	event.set_suffix(spawn->suffix);
	event.set_petownerid(spawn->petOwnerId);
	event.set_guildrank(spawn->guildrank);
	event.set_unknown0194(*spawn->unknown0194);
	
	/*auto texture = eqproto::Texture();
	texture.set_elitemodel(spawn->equipment.Arms.EliteModel);
	texture.set_herosforgemodel(spawn->equipment.Arms.HerosForgeModel);
	texture.set_material(spawn->equipment.Arms.Material);
	texture.set_unknown1(spawn->equipment.Arms.Unknown1);
	texture.set_unknown2(spawn->equipment.Arms.Unknown2);
	event.set_allocated_equipment(textureProfile);*/
	event.set_runspeed(spawn->runspeed);
	event.set_afk(spawn->afk);
	event.set_guildid(spawn->guildID);
	event.set_title(spawn->title);
	event.set_unknown0274(spawn->unknown0274);
	event.set_set_to_0xff(*spawn->set_to_0xFF);
	event.set_helm(spawn->helm);
	event.set_race(spawn->race);
	event.set_unknown0288(spawn->unknown0288);
	event.set_lastname(spawn->lastName);
	event.set_walkspeed(spawn->walkspeed);
	event.set_unknown0328(spawn->unknown0328);
	event.set_is_pet(spawn->is_pet);
	event.set_light(spawn->light);
	event.set_class_(spawn->class_);
	event.set_eyecolor2(spawn->eyecolor2);
	event.set_flymode(spawn->flymode);
	event.set_gender(spawn->gender);
	event.set_bodytype(spawn->bodytype);
	event.set_unknown0336(*spawn->unknown0336);
	event.set_equip_chest2(spawn->equip_chest2);
	event.set_mount_color(spawn->mount_color);
	event.set_spawnid(spawn->spawnId);
	event.set_ismercenary(spawn->IsMercenary);
	//event.set_equipment_tint(spawn->equipment_tint);
	event.set_lfg(spawn->lfg);
	event.set_destructibleobject(spawn->DestructibleObject);
	event.set_destructiblemodel(spawn->DestructibleModel);
	event.set_destructiblename2(spawn->DestructibleName2);
	event.set_destructiblestring(spawn->DestructibleString);
	event.set_destructibleappearance(spawn->DestructibleAppearance);
	event.set_destructibleunk1(spawn->DestructibleUnk1);
	event.set_destructibleid1(spawn->DestructibleID1);
	event.set_destructibleid2(spawn->DestructibleID2);
	event.set_destructibleid3(spawn->DestructibleID3);
	event.set_destructibleid4(spawn->DestructibleID4);
	event.set_destructibleunk2(spawn->DestructibleUnk2);
	event.set_destructibleunk3(spawn->DestructibleUnk3);
	event.set_destructibleunk4(spawn->DestructibleUnk4);
	event.set_destructibleunk5(spawn->DestructibleUnk5);
	event.set_destructibleunk6(spawn->DestructibleUnk6);
	event.set_destructibleunk7(spawn->DestructibleUnk7);
	event.set_destructibleunk8(spawn->DestructibleUnk8);
	event.set_destructibleunk9(spawn->DestructibleUnk9);
	event.set_targetable_with_hotkey(spawn->targetable_with_hotkey);
	event.set_show_name(spawn->show_name);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }	
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	if (op == OP_ZoneEntry) finalEvent.set_op(eqproto::OP_ZoneEntry);
	else if (op == OP_NewSpawn) finalEvent.set_op(eqproto::OP_NewSpawn);
	else { Log(Logs::General, Logs::NATS, "unhandled op type passed: %i", op); return; }
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}


void NatsManager::OnWearChangeEvent(uint32 entity_id, WearChange_Struct *wc) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::WearChangeEvent();


	event.set_spawn_id(wc->spawn_id);
	event.set_material(wc->material);
	event.set_unknown06(wc->unknown06);
	event.set_elite_material(wc->elite_material);
	event.set_hero_forge_model(wc->hero_forge_model);
	event.set_unknown18(wc->unknown18);
	//event.set_color(wc->color); //tint
	event.set_wear_slot_id(wc->wear_slot_id);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_WearChange);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}

void NatsManager::OnDeleteSpawnEvent(uint32 entity_id, DeleteSpawn_Struct *ds) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::DeleteSpawnEvent();


	event.set_spawn_id(ds->spawn_id);
	event.set_decay(ds->Decay);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_DeleteSpawn);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}

void NatsManager::OnHPEvent(const EmuOpcode op, uint32 entity_id, uint32 cur_hp, uint32 max_hp) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;
	if (cur_hp == max_hp) return;
	std::string pubMessage;
	auto event = eqproto::HPEvent();

	event.set_spawn_id(entity_id);
	event.set_cur_hp(cur_hp);
	event.set_max_hp(max_hp);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	if (op == OP_MobHealth) finalEvent.set_op(eqproto::OP_MobHealth);
	else if (op == OP_HPUpdate) finalEvent.set_op(eqproto::OP_HPUpdate);
	else { Log(Logs::General, Logs::NATS, "unhandled op type passed: %i", op); return; }
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}

void NatsManager::OnDamageEvent(uint32 entity_id, CombatDamage_Struct *cd) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;
	std::string pubMessage;
	auto event = eqproto::DamageEvent();

	event.set_target(cd->target);
	event.set_source(cd->source);
	event.set_type(cd->type);
	event.set_spellid(cd->spellid);
	event.set_damage(cd->damage);
	event.set_force(cd->force);
	event.set_meleepush_xy(cd->hit_heading);
	event.set_meleepush_z(cd->hit_pitch);
	
	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_Damage);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}

void NatsManager::OnClientUpdateEvent(uint32 entity_id, PlayerPositionUpdateServer_Struct * spu) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::PlayerPositionUpdateEvent();
	
	event.set_spawn_id(spu->spawn_id);
	event.set_delta_heading(spu->delta_heading);
	event.set_x_pos(spu->x_pos);
	event.set_padding0002(spu->padding0002);
	event.set_y_pos(spu->y_pos);
	event.set_animation(spu->animation);
	event.set_padding0006(spu->padding0006);
	event.set_z_pos(spu->z_pos);
	event.set_delta_y(spu->delta_y);
	event.set_delta_x(spu->delta_x);
	event.set_heading(spu->heading);
	event.set_padding0014(spu->padding0014);
	event.set_delta_z(spu->delta_z);
	event.set_padding0018(spu->padding0018);

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_ClientUpdate);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}


void NatsManager::OnAnimationEvent(uint32 entity_id, Animation_Struct *anim) {
	if (!connect()) return;
	if (entity_id == 0) return;
	if (!isEntityEventAllEnabled && !isEntitySubscribed(entity_id)) return;

	std::string pubMessage;
	auto event = eqproto::AnimationEvent();

	event.set_spawnid(anim->spawnid);
	event.set_speed(anim->speed);
	event.set_action(anim->action);	

	if (!event.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	auto finalEvent = eqproto::Event();
	finalEvent.set_payload(pubMessage.c_str());
	finalEvent.set_op(eqproto::OP_Animation);
	if (!finalEvent.SerializeToString(&pubMessage)) { Log(Logs::General, Logs::NATS, "Failed to serialize message to string"); return; }
	s = natsConnection_Publish(conn, StringFormat("zone.%s.entity.event.%d", subscribedZonename.c_str(), entity_id).c_str(), (const void*)pubMessage.c_str(), pubMessage.length());
	if (s != NATS_OK) Log(Logs::General, Logs::NATS, "Failed to send EntityEvent");
}