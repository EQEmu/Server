#ifndef _NATS_H
#define _NATS_H

#include "nats.h"
//#include "event_codes.h"
//#include "entity.h"
//#include "mob.h"
#include "../common/opcodemgr.h"
//#include "../common/global_define.h"
//#include "../common/types.h"
#undef New //Needed for MSVC compile
#undef Move //Needed for linux compile
#ifndef PROTO_H
#define PROTO_H
#include "../common/message.pb.h"
#endif

class NatsManager
{
public:
	NatsManager();
	~NatsManager();
	void Process();
	void Unregister();
	void ZoneSubscribe(const char * zonename, uint32 instance);
	void Load();
	void GetChannelMessage(eqproto::ChannelMessage* message, const char* reply = nullptr);
	void SendChannelMessage(eqproto::ChannelMessage* message, const char* reply = nullptr);
	void GetCommandMessage(eqproto::CommandMessage* message, const char* reply = nullptr);
	void SendCommandMessage(eqproto::CommandMessage* message, const char* reply = nullptr);
	void SendAdminMessage(std::string adminMessage, int min_status = 200);
	void SendEvent(eqproto::OpCode op, uint32 entity_id, void * buffer, size_t size);

	void OnAlternateAdvancementStats(uint32 entity_id, AltAdvStats_Struct * aas);
	void OnAlternateAdvancementAction(uint32 entity_id, UseAA_Struct * uaas);
	void OnAlternateAdvancementActionRequest(uint32 entity_id, AA_Action* action);
	void OnAnimationEvent(uint32 entity_id, Animation_Struct * anim);
	void OnChannelMessageEvent(uint32 entity_id, ChannelMessage_Struct * cm);
	void OnClientUpdateEvent(uint32 entity_id, PlayerPositionUpdateServer_Struct * spu);
	void OnDamageEvent(uint32 entity_id, CombatDamage_Struct * cd);
	void OnDeathEvent(Death_Struct * d);
	void OnDeleteSpawnEvent(uint32 entity_id, DeleteSpawn_Struct * ds);
	void OnEntityEvent(const EmuOpcode op, uint32 entity_id, uint32 target_id);
	void OnHPEvent(const EmuOpcode op, uint32 entity_id, uint32 cur_hp, uint32 max_hp);
	void OnSpawnEvent(const EmuOpcode op, uint32 entity_id, Spawn_Struct * spawn);
	void OnSpecialMessageEvent(uint32 entity_id, SpecialMesg_Struct *sm);
	void OnWearChangeEvent(uint32 entity_id, WearChange_Struct * wc);
	void OnZoneCompleteEvent(uint32 entity_id);
	void OnNewZoneEvent(uint32 entity_id, NewZone_Struct * nz);
protected:
	bool connect();
	Timer nats_timer;
	bool isEntitySubscribed(const uint16 ID);
	bool isEntityEventAllEnabled = true;
	natsConnection *conn = NULL;
	natsStatus      s;
	natsOptions *opts = NULL;
	std::string subscribedZoneName;
	uint32 subscribedZoneInstance;
	natsSubscription *zoneChannelMessageSub = NULL;
	natsSubscription *zoneInstanceChannelMessageSub = NULL;
	natsSubscription *zoneCommandMessageSub = NULL;
	natsSubscription *zoneInstanceCommandMessageSub = NULL;

	void EncodeEntity(Entity *entity, eqproto::Entity *out);
};

#endif