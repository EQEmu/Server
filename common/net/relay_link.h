#pragma once

#include "tcp_server.h"
#include "packet.h"
#include "../types.h"
#include "../event/timer.h"
#include "../event/event_loop.h"
#include <map>
#include <queue>

namespace EQ
{
	namespace Net {
		class RelayLink
		{
		public:
			RelayLink(const std::string &addr, int port, const std::string &identifier, const std::string &password);
			~RelayLink();

			void OnMessageType(uint16 opcode, std::function<void(const EQ::Net::Packet &p)> cb);
			void SendPacket(uint16 opcode, const EQ::Net::Packet &p);
			bool Connected() const { return m_connection != nullptr; }
			std::string GetIP() const { return m_addr; }
			uint16 GetPort() const { return m_port; }
		private:
			void Connect();
			void ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length);
			void ProcessPacket(const EQ::Net::Packet &p);
			void ProcessBuffer();
			void ProcessQueue();
			void SendIdentifier();
			void SendInternal(const EQ::Net::Packet &p);
			void SendPassword();
			void OnAuthFailed(const EQ::Net::Packet &p);

			std::unique_ptr<EQ::Timer> m_timer;
			std::string m_addr; 
			std::string m_identifier;
			std::string m_password;
			int m_port;
			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			bool m_established;
			bool m_connecting;
			std::vector<char> m_data_buffer;
			std::map<uint16, std::function<void(const EQ::Net::Packet &p)>> m_opcode_dispatch;
			std::queue<EQ::Net::WritablePacket> m_packet_queue;
		};
	}
}
