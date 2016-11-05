#include "tcp_connection.h"
#include "../event/event_loop.h"

void on_close_handle(uv_handle_t* handle) {
	delete handle;
}

EQ::Net::TCPConnection::TCPConnection(uv_tcp_t *socket)
{
	m_socket = socket;
	m_socket->data = this;
}

EQ::Net::TCPConnection::~TCPConnection() {
	Disconnect();
}

void EQ::Net::TCPConnection::Connect(const std::string &addr, int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb)
{
	struct EQTCPConnectBaton
	{
		uv_tcp_t *socket;
		std::function<void(std::shared_ptr<EQ::Net::TCPConnection>)> cb;
	};

	auto loop = EQ::EventLoop::Get().Handle();
	uv_tcp_t *socket = new uv_tcp_t;
	memset(socket, 0, sizeof(uv_tcp_t));
	uv_tcp_init(loop, socket);

	sockaddr_storage iaddr;
	if (ipv6) {
		uv_ip6_addr(addr.c_str(), port, (sockaddr_in6*)&iaddr);
	}
	else {
		uv_ip4_addr(addr.c_str(), port, (sockaddr_in*)&iaddr);
	}

	uv_connect_t *connect = new uv_connect_t;
	memset(connect, 0, sizeof(uv_connect_t));

	EQTCPConnectBaton *baton = new EQTCPConnectBaton;
	baton->cb = cb;
	baton->socket = socket;
	connect->data = baton;
	uv_tcp_connect(connect, socket, (sockaddr*)&iaddr,
		[](uv_connect_t* req, int status) {
		EQTCPConnectBaton *baton = (EQTCPConnectBaton*)req->data;
		auto socket = baton->socket;
		auto cb = baton->cb;

		delete baton;

		if (status < 0) {
			uv_close((uv_handle_t*)socket, on_close_handle);
			delete req;
			cb(nullptr);
		}
		else {
			delete req;
			std::shared_ptr<EQ::Net::TCPConnection> connection(new EQ::Net::TCPConnection(socket));
			cb(connection);
		}
	});
}

void EQ::Net::TCPConnection::Start() {
	uv_read_start((uv_stream_t*)m_socket, [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		buf->base = new char[suggested_size];
		buf->len = suggested_size;
	}, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {

		TCPConnection *connection = (TCPConnection*)stream->data;

		if (nread > 0) {
			connection->Read(buf->base, nread);

			if (buf->base) {
				delete[] buf->base;
			}
		}
		else if (nread == UV_EOF) {
			if (buf->base) {
				delete[] buf->base;
			}
		}
		else if (nread < 0) {
			connection->Disconnect();

			if (buf->base) {
				delete[] buf->base;
			}
		}
	});
}

void EQ::Net::TCPConnection::OnRead(std::function<void(TCPConnection*, const unsigned char*, size_t)> cb)
{
	m_on_read_cb = cb;
}

void EQ::Net::TCPConnection::OnDisconnect(std::function<void(TCPConnection*)> cb)
{
	m_on_disconnect_cb = cb;
}

void EQ::Net::TCPConnection::Disconnect()
{
	if (m_socket) {
		m_socket->data = this;
		uv_close((uv_handle_t*)m_socket, [](uv_handle_t* handle) {
			TCPConnection *connection = (TCPConnection*)handle->data;

			if (connection->m_on_disconnect_cb) {
				connection->m_on_disconnect_cb(connection);
			}

			delete handle;
		});
		m_socket = nullptr;
	}
}

void EQ::Net::TCPConnection::Read(const char *data, size_t count)
{
	if (m_on_read_cb) {
		m_on_read_cb(this, (unsigned char*)data, count);
	}
}

void EQ::Net::TCPConnection::Write(const char *data, size_t count)
{
	if (!m_socket) {
		return;
	}

	struct WriteBaton
	{
		TCPConnection *connection;
		char *buffer;
	};

	WriteBaton *baton = new WriteBaton;
	baton->connection = this;
	baton->buffer = new char[count];;

	uv_write_t *write_req = new uv_write_t;
	memset(write_req, 0, sizeof(uv_write_t));
	write_req->data = baton;
	uv_buf_t send_buffers[1];

	memcpy(baton->buffer, data, count);
	send_buffers[0] = uv_buf_init(baton->buffer, count);

	uv_write(write_req, (uv_stream_t*)m_socket, send_buffers, 1, [](uv_write_t* req, int status) {
		WriteBaton *baton = (WriteBaton*)req->data;
		delete[] baton->buffer;
		delete req;

		if (status < 0) {
			baton->connection->Disconnect();
		}

		delete baton;
	});
}

std::string EQ::Net::TCPConnection::LocalIP() const
{
	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getsockname(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		uv_ip4_name((const sockaddr_in*)&addr, endpoint, 64);
	}
	else if (addr.ss_family == AF_INET6) {
		uv_ip6_name((const sockaddr_in6*)&addr, endpoint, 64);
	}

	return endpoint;
}

int EQ::Net::TCPConnection::LocalPort() const
{
	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getsockname(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		sockaddr_in *s = (sockaddr_in*)&addr;
		return ntohs(s->sin_port);
	}
	else if (addr.ss_family == AF_INET6) {
		sockaddr_in6 *s = (sockaddr_in6*)&addr;
		return ntohs(s->sin6_port);
	}

	return 0;
}

std::string EQ::Net::TCPConnection::RemoteIP() const
{
	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getpeername(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		uv_ip4_name((const sockaddr_in*)&addr, endpoint, 64);
	}
	else if(addr.ss_family == AF_INET6) {
		uv_ip6_name((const sockaddr_in6*)&addr, endpoint, 64);
	}

	return endpoint;
}

int EQ::Net::TCPConnection::RemotePort() const
{
	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getpeername(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		sockaddr_in *s = (sockaddr_in*)&addr;
		return ntohs(s->sin_port);
	}
	else if (addr.ss_family == AF_INET6) {
		sockaddr_in6 *s = (sockaddr_in6*)&addr;
		return ntohs(s->sin6_port);
	}

	return 0;
}
