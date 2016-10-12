#pragma once

#include "tcp_server.h"
#include "packet.h"
#include "../event/timer.h"
#include "../event/event_loop.h"

namespace EQ
{
	namespace Net {
		class RelayLink
		{
		public:
			RelayLink(const std::string &addr, int port, const std::string &identifier);
			~RelayLink();

		private:
			void Connect();
			void ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length);
			void SendIdentifier();
			void SendInternal(const EQ::Net::Packet &p);

			std::unique_ptr<EQ::Timer> m_timer;
			std::string m_addr; 
			std::string m_identifier;
			int m_port;
			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			bool m_established;
			bool m_connecting;
		};
	}
}
