#pragma once

#define EQSM_PAD_LEN 252

namespace EQ
{
	namespace Net
	{
		class DynamicPacket;
		enum ceqs_msg_type : uint32_t
		{
			//Sent by background
			NewConnection,
			ConnectionStateChange,
			PacketRecv,
			UpdateStats,
			//Sent by foreground
			QueuePacket,
			TerminateBackground,
			CloseConnection,
			ResetStats,
			SetPriority
		};

		typedef struct  
		{
			ceqs_msg_type type;
			char padding[EQSM_PAD_LEN];
		} ceqs_msg_t;

		//Sent by background
		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
			int remote_port;
			int state;
			char endpoint[64];
		} ceqs_new_connection_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
			int from;
			int to;
		} ceqs_connection_state_change_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
			EQ::Net::DynamicPacket *packet;
		} ceqs_packet_recv_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
			DaybreakConnectionStats stats;
		} ceqs_update_stats_msg_t;

		//Sent by foreground
		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
			EQ::Net::DynamicPacket *packet;
			bool ack_req;
		} ceqs_queue_packet_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
		} ceqs_close_connection_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
			uint64_t stream_id;
		} ceqs_reset_stats_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
		} ceqs_terminate_msg_t;

		typedef struct
		{
			ceqs_msg_type type;
			EQStreamPriority priority;
		} ceqs_set_priority_msg_t;
	}
}
