#pragma once

#include "tcp_connection.h"
#include "../event/timer.h"
#include "servertalk_common.h"
#include "packet.h"
#ifdef ENABLE_SECURITY
#include <sodium.h>
#endif

namespace EQ
{
	namespace Net
	{
		class ServertalkClient
		{
		public:
			ServertalkClient(const std::string &addr, int port, bool ipv6, const std::string &identifier, const std::string &credentials);
			~ServertalkClient();

			void Send(uint16_t opcode, EQ::Net::Packet &p);
			void SendPacket(ServerPacket *p);
			void OnConnect(std::function<void(ServertalkClient*)> cb) { m_on_connect_cb = cb; }
			void OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			void OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			bool Connected() const { return m_connecting != true; }

			std::shared_ptr<EQ::Net::TCPConnection> Handle() { return m_connection; }
		private:
			void Connect();
			void ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length);
			void SendHello();
			void InternalSend(ServertalkPacketType type, EQ::Net::Packet &p);
			void ProcessReadBuffer();
			void ProcessHello(EQ::Net::Packet &p);
			void ProcessMessage(EQ::Net::Packet &p);
			void SendHandshake() { SendHandshake(false); }
			void SendHandshake(bool downgrade);

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
			std::unordered_map<uint16_t, std::function<void(uint16_t, EQ::Net::Packet&)>> m_message_callbacks;
			std::function<void(uint16_t, EQ::Net::Packet&)> m_message_callback;
			std::function<void(ServertalkClient*)> m_on_connect_cb;

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
