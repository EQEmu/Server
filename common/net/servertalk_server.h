#pragma once

#include "tcp_server.h"
#include "servertalk_server_connection.h"
#include <vector>
#include <map>

#ifdef ENABLE_SECURITY
#include <sodium.h>
#endif

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
#ifdef ENABLE_SECURITY
				encrypted = true;
				allow_downgrade = true;
#else
				encrypted = false;
				allow_downgrade = true;
#endif
				ipv6 = false;
			}
		};

		class ServertalkServer
		{
		public:
			ServertalkServer();
			~ServertalkServer();

			void Listen(const ServertalkServerOptions& opts);
			void OnConnectionIdentified(const std::string &type, std::function<void(std::shared_ptr<ServertalkServerConnection>)> cb);
			void OnConnectionRemoved(const std::string &type, std::function<void(std::shared_ptr<ServertalkServerConnection>)> cb);

		private:
			void ConnectionDisconnected(ServertalkServerConnection *conn);
			void ConnectionIdentified(ServertalkServerConnection *conn);
			bool CheckCredentials(const std::string &credentials);

			std::unique_ptr<EQ::Net::TCPServer> m_server;
			std::vector<std::shared_ptr<ServertalkServerConnection>> m_unident_connections;
			std::map<std::string, std::vector<std::shared_ptr<ServertalkServerConnection>>> m_ident_connections;

			std::map<std::string, std::function<void(std::shared_ptr<ServertalkServerConnection>)>> m_on_ident;
			std::map<std::string, std::function<void(std::shared_ptr<ServertalkServerConnection>)>> m_on_disc;
			bool m_encrypted;
			bool m_allow_downgrade;
			std::string m_credentials;

			friend class ServertalkServerConnection;
		};
	}
}
