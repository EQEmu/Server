#pragma once

#include "tcp_connection.h"
#include "servertalk_common.h"
#include "packet.h"
#include <vector>
#ifdef ENABLE_SECURITY
#include <sodium.h>
#endif

namespace EQ
{
	namespace Net
	{
		class ServertalkServer;
		class ServertalkServerConnection
		{
		public:
			ServertalkServerConnection(std::shared_ptr<EQ::Net::TCPConnection> c, ServertalkServer *parent, bool encrypted, bool allow_downgrade);
			~ServertalkServerConnection();

			void Send(uint16_t opcode, EQ::Net::Packet &p);
			void SendPacket(ServerPacket *p);
			void OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			void OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb);

			std::string GetIdentifier() const { return m_identifier; }
			std::shared_ptr<EQ::Net::TCPConnection> Handle() { return m_connection; }
			std::string GetUUID() const { return m_uuid; }
		private:
			void OnRead(TCPConnection* c, const unsigned char* data, size_t sz);
			void ProcessReadBuffer();
			void OnDisconnect(TCPConnection* c);
			void SendHello();
			void InternalSend(ServertalkPacketType type, EQ::Net::Packet &p);
			void ProcessHandshake(EQ::Net::Packet &p) { ProcessHandshake(p, false); }
			void ProcessHandshake(EQ::Net::Packet &p, bool security_downgrade);
			void ProcessMessage(EQ::Net::Packet &p);

			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			ServertalkServer *m_parent;

			std::vector<char> m_buffer;
			std::unordered_map<uint16_t, std::function<void(uint16_t, EQ::Net::Packet&)>> m_message_callbacks;
			std::function<void(uint16_t, EQ::Net::Packet&)> m_message_callback;
			std::string m_identifier;
			std::string m_uuid;

			bool m_encrypted;
			bool m_allow_downgrade;
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
