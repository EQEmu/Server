#pragma once

#include "tcp_server.h"
#include "servertalk_server_connection.h"
#include <vector>
#include <map>

namespace EQ
{
	namespace Net
	{
		struct ServertalkServerOptions
		{
			int port;
			bool ipv6;
			bool encrypted;
			bool allow_downgrade;
			std::string credentials;

			ServertalkServerOptions() {
				encrypted = false;
				allow_downgrade = true;
				ipv6 = false;
			}
		};

		class ServertalkServer
		{
		public:
			typedef std::function<void(std::shared_ptr<ServertalkServerConnection>)> IdentityCallback;

			ServertalkServer();
			~ServertalkServer();

			void Listen(const ServertalkServerOptions& opts);
			void OnConnectionIdentified(const std::string &type, IdentityCallback cb);
			void OnConnectionRemoved(const std::string &type, IdentityCallback cb);
			void OnConnectionIdentified(IdentityCallback cb);
			void OnConnectionRemoved(IdentityCallback cb);

		private:
			void ConnectionDisconnected(ServertalkServerConnection *conn);
			void ConnectionIdentified(ServertalkServerConnection *conn);
			bool CheckCredentials(const std::string &credentials);

			std::unique_ptr<EQ::Net::TCPServer> m_server;
			std::vector<std::shared_ptr<ServertalkServerConnection>> m_unident_connections;
			std::map<std::string, std::vector<std::shared_ptr<ServertalkServerConnection>>> m_ident_connections;

			std::map<std::string, IdentityCallback> m_on_ident;
			std::map<std::string, IdentityCallback> m_on_disc;
			IdentityCallback m_on_any_ident;
			IdentityCallback m_on_any_disc;
			bool m_encrypted;
			bool m_allow_downgrade;
			std::string m_credentials;

			friend class ServertalkServerConnection;
		};
	}
}
