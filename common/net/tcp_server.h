#pragma once

#include "tcp_connection.h"
#include "../event/event_loop.h"

namespace EQ
{
	namespace Net
	{
		class TCPServer
		{
		public:
			TCPServer();
			TCPServer(EQ::EventLoop *loop);
			~TCPServer();

			void Listen(int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb);
			void Listen(const std::string &addr, int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb);
			void Close();
			void AddClient(uv_tcp_t *c);

		private:
			std::function<void(std::shared_ptr<TCPConnection>)> m_on_new_connection;
			EQ::EventLoop *m_loop;
			uv_tcp_t *m_socket;
		};
	}
}
