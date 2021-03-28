#include "servertalk_client_connection.h"
#include "dns.h"
#include "../eqemu_logsys.h"

EQ::Net::ServertalkClient::ServertalkClient(const std::string &addr, int port, bool ipv6, const std::string &identifier, const std::string &credentials)
	: m_timer(std::make_unique<EQ::Timer>(100, true, std::bind(&EQ::Net::ServertalkClient::Connect, this)))
{
	m_port = port;
	m_ipv6 = ipv6;
	m_identifier = identifier.empty() ? "Unknown" : identifier;
	m_credentials = credentials;
	m_connecting = false;
	DNSLookup(addr, port, false, [this](const std::string &address) {
		m_addr = address;
	});
}

EQ::Net::ServertalkClient::~ServertalkClient()
{
}

void EQ::Net::ServertalkClient::Send(uint16_t opcode, EQ::Net::Packet &p)
{
	EQ::Net::DynamicPacket out;
#ifdef ENABLE_SECURITY
	if (m_encrypted) {
		if (p.Length() == 0) {
			p.PutUInt8(0, 0);
		}

		out.PutUInt32(0, p.Length() + crypto_secretbox_MACBYTES);
		out.PutUInt16(4, opcode);

		std::unique_ptr<unsigned char[]> cipher(new unsigned char[p.Length() + crypto_secretbox_MACBYTES]);

		crypto_box_easy_afternm(&cipher[0], (unsigned char*)p.Data(), p.Length(), m_nonce_ours, m_shared_key);
		(*(uint64_t*)&m_nonce_ours[0])++;
		out.PutData(6, &cipher[0], p.Length() + crypto_secretbox_MACBYTES);
	}
	else {
		out.PutUInt32(0, p.Length());
		out.PutUInt16(4, opcode);
		out.PutPacket(6, p);
	}
#else
	out.PutUInt32(0, p.Length());
	out.PutUInt16(4, opcode);
	out.PutPacket(6, p);
#endif
	InternalSend(ServertalkMessage, out);
}

void EQ::Net::ServertalkClient::SendPacket(ServerPacket *p)
{
	EQ::Net::DynamicPacket pout;
	if (p->pBuffer) {
		pout.PutData(0, p->pBuffer, p->size);
	}
	Send(p->opcode, pout);
}

void EQ::Net::ServertalkClient::OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callbacks.insert(std::make_pair(opcode, cb));
}

void EQ::Net::ServertalkClient::OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callback = cb;
}

void EQ::Net::ServertalkClient::Connect()
{
	if (m_addr.length() == 0 || m_port == 0 || m_connection || m_connecting) {
		return;
	}

	m_connecting = true;
	EQ::Net::TCPConnection::Connect(m_addr, m_port, false, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		if (connection == nullptr) {
			LogF(Logs::General, Logs::TCPConnection, "Error connecting to {0}:{1}, attempting to reconnect...", m_addr, m_port);
			m_connecting = false;
			return;
		}

		LogF(Logs::General, Logs::TCPConnection, "Connected to {0}:{1}", m_addr, m_port);
		m_connection = connection;
		m_connection->OnDisconnect([this](EQ::Net::TCPConnection *c) {
			LogF(Logs::General, Logs::TCPConnection, "Connection lost to {0}:{1}, attempting to reconnect...", m_addr, m_port);
			m_encrypted = false;
			m_connection.reset();
		});

		m_connection->OnRead(std::bind(&EQ::Net::ServertalkClient::ProcessData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_connection->Start();

		SendHello();
		m_connecting = false;
	});
}

void EQ::Net::ServertalkClient::ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length)
{
	m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + length);
	ProcessReadBuffer();
}

void EQ::Net::ServertalkClient::SendHello()
{
	EQ::Net::DynamicPacket p;
	InternalSend(ServertalkClientHello, p);
}

void EQ::Net::ServertalkClient::InternalSend(ServertalkPacketType type, EQ::Net::Packet &p)
{
	if (!m_connection)
		return;

	EQ::Net::DynamicPacket out;
	out.PutUInt32(0, (uint32_t)p.Length());
	out.PutUInt8(4, (uint8_t)type);
	if (p.Length() > 0) {
		out.PutPacket(5, p);
	}

	m_connection->Write((const char*)out.Data(), out.Length());
}

void EQ::Net::ServertalkClient::ProcessReadBuffer()
{
	size_t current = 0;
	size_t total = m_buffer.size();

	while (current < total) {
		auto left = total - current;

		/*
		//header:
		//uint32 length;
		//uint8 type;
		*/
		size_t length = 0;
		uint8_t type = 0;
		if (left < 5) {
			break;
		}

		length = *(uint32_t*)&m_buffer[current];
		type = *(uint8_t*)&m_buffer[current + 4];

		if (current + 5 + length > total) {
			break;
		}

		if (length == 0) {
			EQ::Net::DynamicPacket p;
			switch (type) {
			case ServertalkServerHello:
				ProcessHello(p);
				break;
			case ServertalkMessage:
				ProcessMessage(p);
				break;
			}
		}
		else {
			EQ::Net::StaticPacket p(&m_buffer[current + 5], length);
			switch (type) {
			case ServertalkServerHello:
				ProcessHello(p);
				break;
			case ServertalkMessage:
				ProcessMessage(p);
				break;
			}
		}

		current += length + 5;
	}

	if (current == total) {
		m_buffer.clear();
	}
	else {
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + current);
	}
}

void EQ::Net::ServertalkClient::ProcessHello(EQ::Net::Packet &p)
{
#ifdef ENABLE_SECURITY
	memset(m_public_key_ours, 0, crypto_box_PUBLICKEYBYTES);
	memset(m_public_key_theirs, 0, crypto_box_PUBLICKEYBYTES);
	memset(m_private_key_ours, 0, crypto_box_SECRETKEYBYTES);
	memset(m_nonce_ours, 0, crypto_box_NONCEBYTES);
	memset(m_nonce_theirs, 0, crypto_box_NONCEBYTES);
	memset(m_shared_key, 0, crypto_box_BEFORENMBYTES);
	m_encrypted = false;

	try {
		bool enc = p.GetInt8(0) == 1 ? true : false;

		if (enc) {
			if (p.Length() == (1 + crypto_box_PUBLICKEYBYTES + crypto_box_NONCEBYTES)) {
				memcpy(m_public_key_theirs, (char*)p.Data() + 1, crypto_box_PUBLICKEYBYTES);
				memcpy(m_nonce_theirs, (char*)p.Data() + 1 + crypto_box_PUBLICKEYBYTES, crypto_box_NONCEBYTES);
				m_encrypted = true;

				SendHandshake();

				if (m_on_connect_cb) {
					m_on_connect_cb(this);
				}
			}
			else {
				LogError("Could not process hello, size != {0}", 1 + crypto_box_PUBLICKEYBYTES + crypto_box_NONCEBYTES);
			}
		}
		else {
			SendHandshake();

			if (m_on_connect_cb) {
				m_on_connect_cb(this);
			}
		}
	}
	catch (std::exception &ex) {
		LogError("Error parsing hello from server: {0}", ex.what());
		m_connection->Disconnect();

		if (m_on_connect_cb) {
			m_on_connect_cb(nullptr);
		}
	}
#else
	try {
		bool enc = p.GetInt8(0) == 1 ? true : false;

		if (enc) {
			SendHandshake(true);

			if (m_on_connect_cb) {
				m_on_connect_cb(this);
			}
		}
		else {
			SendHandshake();

			if (m_on_connect_cb) {
				m_on_connect_cb(this);
			}
		}
}
	catch (std::exception &ex) {
		LogError("Error parsing hello from server: {0}", ex.what());
		m_connection->Disconnect();

		if (m_on_connect_cb) {
			m_on_connect_cb(nullptr);
		}
	}
#endif
}

void EQ::Net::ServertalkClient::ProcessMessage(EQ::Net::Packet &p)
{
	try {
		auto length = p.GetUInt32(0);
		auto opcode = p.GetUInt16(4);
		if (length > 0) {
			auto data = p.GetString(6, length);
#ifdef ENABLE_SECURITY
			if (m_encrypted) {
				size_t message_len = length - crypto_secretbox_MACBYTES;
				std::unique_ptr<unsigned char[]> decrypted_text(new unsigned char[message_len]);
				if (crypto_box_open_easy_afternm(&decrypted_text[0], (unsigned char*)&data[0], length, m_nonce_theirs, m_shared_key))
				{
					LogError("Error decrypting message from server");
					(*(uint64_t*)&m_nonce_theirs[0])++;
					return;
				}

				EQ::Net::StaticPacket decrypted_packet(&decrypted_text[0], message_len);

				(*(uint64_t*)&m_nonce_theirs[0])++;

				auto cb = m_message_callbacks.find(opcode);
				if (cb != m_message_callbacks.end()) {
					cb->second(opcode, decrypted_packet);
				}

				if (m_message_callback) {
					m_message_callback(opcode, decrypted_packet);
				}
			}
			else {
				size_t message_len = length;
				EQ::Net::StaticPacket packet(&data[0], message_len);

				auto cb = m_message_callbacks.find(opcode);
				if (cb != m_message_callbacks.end()) {
					cb->second(opcode, packet);
				}

				if (m_message_callback) {
					m_message_callback(opcode, packet);
				}
			}

#else
			size_t message_len = length;
			EQ::Net::StaticPacket packet(&data[0], message_len);

			auto cb = m_message_callbacks.find(opcode);
			if (cb != m_message_callbacks.end()) {
				cb->second(opcode, packet);
			}

			if (m_message_callback) {
				m_message_callback(opcode, packet);
			}
#endif
		}
	}
	catch (std::exception &ex) {
		LogError("Error parsing message from server: {0}", ex.what());
	}
}

void EQ::Net::ServertalkClient::SendHandshake(bool downgrade)
{
	EQ::Net::DynamicPacket handshake;
#ifdef ENABLE_SECURITY
	if (m_encrypted) {
		crypto_box_keypair(m_public_key_ours, m_private_key_ours);
		randombytes_buf(m_nonce_ours, crypto_box_NONCEBYTES);

		crypto_box_beforenm(m_shared_key, m_public_key_theirs, m_private_key_ours);

		handshake.PutData(0, m_public_key_ours, crypto_box_PUBLICKEYBYTES);
		handshake.PutData(crypto_box_PUBLICKEYBYTES, m_nonce_ours, crypto_box_NONCEBYTES);

		memset(m_public_key_ours, 0, crypto_box_PUBLICKEYBYTES);
		memset(m_public_key_theirs, 0, crypto_box_PUBLICKEYBYTES);
		memset(m_private_key_ours, 0, crypto_box_SECRETKEYBYTES);

		size_t cipher_length = m_identifier.length() + 1 + m_credentials.length() + 1 + crypto_secretbox_MACBYTES;
		size_t data_length = m_identifier.length() + 1 + m_credentials.length() + 1;
		
		std::unique_ptr<unsigned char[]> signed_buffer(new unsigned char[cipher_length]);
		std::unique_ptr<unsigned char[]> data_buffer(new unsigned char[data_length]);

		memset(&data_buffer[0], 0, data_length);
		memcpy(&data_buffer[0], m_identifier.c_str(), m_identifier.length());
		memcpy(&data_buffer[1 + m_identifier.length()], m_credentials.c_str(), m_credentials.length());
		
		crypto_box_easy_afternm(&signed_buffer[0], &data_buffer[0], data_length, m_nonce_ours, m_shared_key);

		(*(uint64_t*)&m_nonce_ours[0])++;

		handshake.PutData(crypto_box_PUBLICKEYBYTES + crypto_box_NONCEBYTES, &signed_buffer[0], cipher_length);
	}
	else {
		handshake.PutString(0, m_identifier);
		handshake.PutString(m_identifier.length() + 1, m_credentials);
		handshake.PutUInt8(m_identifier.length() + 1 + m_credentials.length(), 0);
	}
#else
	handshake.PutString(0, m_identifier);
	handshake.PutString(m_identifier.length() + 1, m_credentials);
	handshake.PutUInt8(m_identifier.length() + 1 + m_credentials.length(), 0);
#endif

	if (downgrade) {
		InternalSend(ServertalkClientDowngradeSecurityHandshake, handshake);
	}
	else {
		InternalSend(ServertalkClientHandshake, handshake);
	}
}
