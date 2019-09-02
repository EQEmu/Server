#include "servertalk_server_connection.h"
#include "servertalk_server.h"
#include "../eqemu_logsys.h"
#include "../eqemu_logsys_fmt.h"
#include "../util/uuid.h"

EQ::Net::ServertalkServerConnection::ServertalkServerConnection(std::shared_ptr<EQ::Net::TCPConnection> c, EQ::Net::ServertalkServer *parent, bool encrypted, bool allow_downgrade)
{
	m_connection = c;
	m_parent = parent;
	m_encrypted = encrypted;
	m_allow_downgrade = allow_downgrade;
	m_uuid = EQ::Util::UUID::Generate().ToString();
	m_connection->OnRead(std::bind(&ServertalkServerConnection::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection->OnDisconnect(std::bind(&ServertalkServerConnection::OnDisconnect, this, std::placeholders::_1));
	m_connection->Start();
}

EQ::Net::ServertalkServerConnection::~ServertalkServerConnection()
{
}

void EQ::Net::ServertalkServerConnection::Send(uint16_t opcode, EQ::Net::Packet & p)
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

void EQ::Net::ServertalkServerConnection::SendPacket(ServerPacket *p)
{
	EQ::Net::DynamicPacket pout;
	if (p->pBuffer) {
		pout.PutData(0, p->pBuffer, p->size);
	}
	Send(p->opcode, pout);
}

void EQ::Net::ServertalkServerConnection::OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callbacks.insert(std::make_pair(opcode, cb));
}

void EQ::Net::ServertalkServerConnection::OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callback = cb;
}

void EQ::Net::ServertalkServerConnection::OnRead(TCPConnection *c, const unsigned char *data, size_t sz)
{
	m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + sz);
	ProcessReadBuffer();
}

void EQ::Net::ServertalkServerConnection::ProcessReadBuffer()
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
			case ServertalkClientHello:
			{
				SendHello();
			}
			break;
			case ServertalkClientHandshake:
				ProcessHandshake(p);
				break;
			case ServertalkMessage:
				ProcessMessage(p);
				break;
			}
		}
		else {
			EQ::Net::StaticPacket p(&m_buffer[current + 5], length);
			switch (type) {
			case ServertalkClientHello:
			{
				SendHello();
			}
			break;
			case ServertalkClientHandshake:
				ProcessHandshake(p);
				break;
			case ServertalkClientDowngradeSecurityHandshake:
				ProcessHandshake(p, true);
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

void EQ::Net::ServertalkServerConnection::OnDisconnect(TCPConnection *c)
{
	m_parent->ConnectionDisconnected(this);
}

void EQ::Net::ServertalkServerConnection::SendHello()
{
	EQ::Net::DynamicPacket hello;

#ifdef ENABLE_SECURITY
	memset(m_public_key_ours, 0, crypto_box_PUBLICKEYBYTES);
	memset(m_public_key_theirs, 0, crypto_box_PUBLICKEYBYTES);
	memset(m_private_key_ours, 0, crypto_box_SECRETKEYBYTES);
	memset(m_nonce_ours, 0, crypto_box_NONCEBYTES);
	memset(m_nonce_theirs, 0, crypto_box_NONCEBYTES);

	if (m_encrypted) {
		hello.PutInt8(0, 1);

		crypto_box_keypair(m_public_key_ours, m_private_key_ours);
		randombytes_buf(m_nonce_ours, crypto_box_NONCEBYTES);

		hello.PutData(1, m_public_key_ours, crypto_box_PUBLICKEYBYTES);
		hello.PutData(1 + crypto_box_PUBLICKEYBYTES, m_nonce_ours, crypto_box_NONCEBYTES);
	}
	else {
		hello.PutInt8(0, 0);
	}
#else
	hello.PutInt8(0, 0);
#endif

	InternalSend(ServertalkServerHello, hello);
}

void EQ::Net::ServertalkServerConnection::InternalSend(ServertalkPacketType type, EQ::Net::Packet &p)
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

void EQ::Net::ServertalkServerConnection::ProcessHandshake(EQ::Net::Packet &p, bool downgrade_security)
{
#ifdef ENABLE_SECURITY
	if (downgrade_security && m_allow_downgrade && m_encrypted) {
		LogF(Logs::General, Logs::TCP_Connection, "Downgraded encrypted connection to plaintext because otherside didn't support encryption {0}:{1}", 
			m_connection->RemoteIP(), m_connection->RemotePort());
		m_encrypted = false;
	}

	if (m_encrypted) {
		try {
			if (p.Length() > (crypto_box_PUBLICKEYBYTES + crypto_box_NONCEBYTES)) {
				memcpy(m_public_key_theirs, (char*)p.Data(), crypto_box_PUBLICKEYBYTES);
				memcpy(m_nonce_theirs, (char*)p.Data() + crypto_box_PUBLICKEYBYTES, crypto_box_NONCEBYTES);
			
				crypto_box_beforenm(m_shared_key, m_public_key_theirs, m_private_key_ours);

				size_t cipher_len = p.Length() - crypto_box_PUBLICKEYBYTES - crypto_box_NONCEBYTES;
				size_t message_len = cipher_len - crypto_secretbox_MACBYTES;
				std::unique_ptr<unsigned char[]> decrypted_text(new unsigned char[message_len]);

				if (crypto_box_open_easy_afternm(&decrypted_text[0], (unsigned char*)p.Data() + crypto_box_PUBLICKEYBYTES + crypto_box_NONCEBYTES, cipher_len, m_nonce_theirs, m_shared_key))
				{
					LogF(Logs::General, Logs::Error, "Error decrypting handshake from client, dropping connection.");
					m_connection->Disconnect();
					return;
				}
				
				m_identifier = (const char*)&decrypted_text[0];
				std::string credentials = (const char*)&decrypted_text[0] + (m_identifier.length() + 1);

				if (!m_parent->CheckCredentials(credentials)) {
					LogF(Logs::General, Logs::Error, "Got incoming connection with invalid credentials during handshake, dropping connection.");
					m_connection->Disconnect();
					return;
				}

				m_parent->ConnectionIdentified(this);
				(*(uint64_t*)&m_nonce_theirs[0])++;
			}
		}
		catch (std::exception &ex) {
			LogF(Logs::General, Logs::Error, "Error parsing handshake from client: {0}", ex.what());
			m_connection->Disconnect();
		}
	}
	else {
		try {
			m_identifier = p.GetCString(0);
			auto credentials = p.GetCString(m_identifier.length() + 1);

			if (!m_parent->CheckCredentials(credentials)) {
				LogF(Logs::General, Logs::Error, "Got incoming connection with invalid credentials during handshake, dropping connection.");
				m_connection->Disconnect();
				return;
			}

			m_parent->ConnectionIdentified(this);
		}
		catch (std::exception &ex) {
			LogF(Logs::General, Logs::Error, "Error parsing handshake from client: {0}", ex.what());
			m_connection->Disconnect();
		}
	}
#else
	try {
		m_identifier = p.GetCString(0);
		auto credentials = p.GetCString(m_identifier.length() + 1);

		if (!m_parent->CheckCredentials(credentials)) {
			LogF(Logs::General, Logs::Error, "Got incoming connection with invalid credentials during handshake, dropping connection.");
			m_connection->Disconnect();
			return;
		}

		m_parent->ConnectionIdentified(this);
	}
	catch (std::exception &ex) {
		LogF(Logs::General, Logs::Error, "Error parsing handshake from client: {0}", ex.what());
		m_connection->Disconnect();
	}
#endif
}

void EQ::Net::ServertalkServerConnection::ProcessMessage(EQ::Net::Packet &p)
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
					LogF(Logs::General, Logs::Error, "Error decrypting message from client");
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
		LogF(Logs::General, Logs::Error, "Error parsing message from client: {0}", ex.what());
	}
}
