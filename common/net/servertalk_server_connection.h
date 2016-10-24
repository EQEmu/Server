#pragma once

#include "tcp_connection.h"
#include "servertalk_common.h"
#include "packet.h"
#include <vector>
#include <sodium.h>

namespace EQ
{
	namespace Net
	{
		class ServertalkServer;
		class ServertalkServerConnection
		{
		public:
			ServertalkServerConnection(std::shared_ptr<EQ::Net::TCPConnection> c, ServertalkServer *parent, bool encrypted);
			~ServertalkServerConnection();

			std::string GetIdentifier() const {
				return m_identifier;
			}
		private:
			void OnRead(TCPConnection* c, const unsigned char* data, size_t sz);
			void ProcessReadBuffer();
			void OnDisconnect(TCPConnection* c);
			void SendHello();
			void InternalSend(ServertalkPacketType type, EQ::Net::Packet &p);
			void ProcessHandshake(EQ::Net::Packet &p);
			void ProcessMessage(EQ::Net::Packet &p);

			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			ServertalkServer *m_parent;

			std::vector<char> m_buffer;
			std::string m_identifier;

			bool m_encrypted;
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
