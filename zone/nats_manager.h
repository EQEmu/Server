#ifndef _NATS_H
#define _NATS_H
#include "nats.h"
#include "event_codes.h"
#include "entity.h"
#include "mob.h"

#include "../common/opcodemgr.h"
#include "../common/global_define.h"
#include "../common/types.h"

class NatsManager
{
public:
	NatsManager();
	~NatsManager();
	void Process();
	void Unregister();
	void ZoneSubscribe(const char * zonename);
	void Load();
	void DailyGain(int account_id, int character_id, const char * identity, int levels_gained = 0, int experience_gained = 0, int money_earned = 0);
	void OnChannelMessageEvent(uint32 entity_id, ChannelMessage_Struct * cm);
	void OnEntityEvent(const EmuOpcode op, uint32 entity_id, uint32 target_id);
	void OnSpawnEvent(const EmuOpcode op, uint32 entity_id, Spawn_Struct * spawn);
	void OnWearChangeEvent(uint32 entity_id, WearChange_Struct * wc);
	void OnDeleteSpawnEvent(uint32 entity_id, DeleteSpawn_Struct * ds);
	void OnHPEvent(const EmuOpcode op, uint32 entity_id, uint32 cur_hp, uint32 max_hp);
	void OnDamageEvent(uint32 entity_id, CombatDamage_Struct * cd);
	void OnClientUpdateEvent(uint32 entity_id, PlayerPositionUpdateServer_Struct * spu);
	void OnAnimationEvent(uint32 entity_id, Animation_Struct * anim);
	void OnDeathEvent(Death_Struct * d);	
	void SendAdminMessage(std::string adminMessage);	
protected:
	bool connect();
	Timer nats_timer;
	bool isEntitySubscribed(const uint16 ID);
	bool isEntityEventAllEnabled = true;
	natsConnection *conn = NULL;
	natsStatus      s;
	natsOptions *opts = NULL;
	std::string subscribedZonename;
	//global zone subscriptions
	natsSubscription *zoneSub = NULL;
	natsSubscription *channelMessageSub = NULL;
	natsSubscription *commandMessageSub = NULL;
	//zone specific subscriptions
	natsSubscription *zoneChannelMessageSub = NULL;
	natsSubscription *zoneCommandMessageSub = NULL;
	natsSubscription *zoneEntityEventSubscribeAllSub = NULL;
	natsSubscription *zoneEntityEventSubscribeSub = NULL;
	natsSubscription *zoneEntityEventListSub = NULL;
	natsSubscription *zoneEntityEventSub = NULL;
};

#endif