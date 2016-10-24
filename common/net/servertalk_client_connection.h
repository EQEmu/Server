#pragma once

#include "tcp_connection.h"
#include "../event/timer.h"
#include "servertalk_common.h"
#include "packet.h"
#include <sodium.h>

namespace EQ
{
	namespace Net
	{
		class ServertalkClient
		{
		public:
			ServertalkClient(const std::string &addr, int port, bool ipv6, const std::string &identifier, const std::string &credentials);
			~ServertalkClient();

		private:
			void Connect();
			void ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length);
			void SendHello();
			void InternalSend(ServertalkPacketType type, EQ::Net::Packet &p);
			void ProcessReadBuffer();
			void ProcessHello(EQ::Net::Packet &p);
			void ProcessMessage(EQ::Net::Packet &p);
			void SendHandshake();

			std::unique_ptr<EQ::Timer> m_timer;

			std::string m_addr;
			std::string m_identifier;
			std::string m_credentials;
			bool m_connecting;
			int m_port;
			bool m_ipv6;
			bool m_encrypted;
			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			std::vector<char> m_buffer;

#ifdef ENABLE_SECURITY
			unsigned char m_public_key_ours[crypto_box_PUBLICKEYBYTES];
			unsigned char m_private_key_ours[crypto_box_SECRETKEYBYTES];
			unsigned char m_nonce_ours[crypto_box_NONCEBYTES];

			unsigned char m_public_key_theirs[crypto_box_PUBLICKEYBYTES];
			unsigned char m_nonce_theirs[crypto_box_NONCEBYTES];

			unsigned char m_shared_key[crypto_box_BEFORENMBYTES];
#endif
		};
	}
}
