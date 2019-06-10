#pragma once

#include "eq_packet_structs.h"
#include "net/servertalk_client_connection.h"
#include <functional>

namespace EQ
{
	class WorldConnection
	{
	public:
		typedef std::function<void()> OnConnectedHandler;
		typedef std::function<void(uint16, const EQ::Net::Packet&)> OnMessageHandler;
		typedef std::function<void(const std::string&, const std::string&, const std::string&, const EQ::Net::Packet&)> OnRoutedMessageHandler;

		WorldConnection(const std::string &type);
		virtual ~WorldConnection();
	
		void SendPacket(ServerPacket* pack);
		std::string GetIP() const;
		uint16 GetPort() const;
		bool Connected() const;

		void SetOnConnectedHandler(OnConnectedHandler handler) {
			m_on_connected = handler;
		};

		void SetOnMessageHandler(OnMessageHandler handler) {
			m_on_message = handler;
		};

		void SetOnRoutedMessageHandler(OnRoutedMessageHandler handler) {
			m_on_routed_message = handler;
		}

		void RouteMessage(const std::string &filter, const std::string &id, const EQ::Net::Packet& payload);
		
	protected:
		OnConnectedHandler m_on_connected;
		OnMessageHandler m_on_message;
		OnRoutedMessageHandler m_on_routed_message;
		std::unique_ptr<EQ::Net::ServertalkClient> m_connection;

	private:
		void _HandleMessage(uint16 opcode, const EQ::Net::Packet& p);
		void _HandleRoutedMessage(uint16 opcode, const EQ::Net::Packet& p);
	};
}
