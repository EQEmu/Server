#ifndef NATS_H
#define NATS_H

#include "nats.h"

#include "world_config.h"
#include "../common/global_define.h"
#include "../common/types.h"
#include "../common/timer.h"
#ifndef PROTO_H
#define PROTO_H
#include "../common/proto/message.pb.h"
#endif
#include "../common/servertalk.h"

class NatsManager
{
public:
	NatsManager();
	~NatsManager();

	void Process();
	void OnChannelMessage(ServerChannelMessage_Struct * msg);
	void OnEmoteMessage(ServerEmoteMessage_Struct * msg);
	void SendAdminMessage(std::string adminMessage);
	void ChannelMessageEvent(eqproto::ChannelMessage* message);
	void CommandMessageEvent(eqproto::CommandMessage* message, const char* reply);
	void SendChannelMessage(eqproto::ChannelMessage* message);
	void Save();
	void Load();
protected:
	bool connect();
	Timer nats_timer;
	natsConnection *conn = NULL;
	natsStatus      s;
	natsStatistics *stats = NULL;
	natsOptions *opts = NULL;
	natsSubscription *channelMessageSub = NULL;
	natsSubscription *commandMessageSub = NULL;
	natsSubscription *adminMessageSub = NULL;
};

#endif