#include "daybreak_connection.h"
#include "../event/event_loop.h"
#include "../eqemu_logsys.h"
#include "crc32.h"
#include <zlib.h>
#include <sstream>

EQ::Net::DaybreakConnectionManager::DaybreakConnectionManager()
{
	m_attached = nullptr;
	memset(&m_timer, 0, sizeof(uv_timer_t));
	memset(&m_socket, 0, sizeof(uv_udp_t));

	Attach(EQ::EventLoop::Get().Handle());
}

EQ::Net::DaybreakConnectionManager::DaybreakConnectionManager(const DaybreakConnectionManagerOptions &opts)
{
	m_attached = nullptr;
	m_options = opts;
	memset(&m_timer, 0, sizeof(uv_timer_t));
	memset(&m_socket, 0, sizeof(uv_udp_t));

	Attach(EQ::EventLoop::Get().Handle());
}

EQ::Net::DaybreakConnectionManager::~DaybreakConnectionManager()
{
	Detach();
}

void EQ::Net::DaybreakConnectionManager::Attach(uv_loop_t *loop)
{
	if (!m_attached) {
		uv_timer_init(loop, &m_timer);
		m_timer.data = this;

		auto update_rate = (uint64_t)(1000.0 / m_options.tic_rate_hertz);

		uv_timer_start(&m_timer, [](uv_timer_t *handle) {
			DaybreakConnectionManager *c = (DaybreakConnectionManager*)handle->data;
			c->Process();
			c->ProcessResend();
		}, update_rate, update_rate);

		uv_udp_init(loop, &m_socket);
		m_socket.data = this;
		struct sockaddr_in recv_addr;
		uv_ip4_addr("0.0.0.0", m_options.port, &recv_addr);
		int rc = uv_udp_bind(&m_socket, (const struct sockaddr *)&recv_addr, UV_UDP_REUSEADDR);

		rc = uv_udp_recv_start(&m_socket,
			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
			buf->base = new char[suggested_size];
			memset(buf->base, 0, suggested_size);
			buf->len = suggested_size;
		},
			[](uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
			DaybreakConnectionManager *c = (DaybreakConnectionManager*)handle->data;
			if (nread < 0 || addr == nullptr) {
				delete[] buf->base;
				return;
			}

			char endpoint[16];
			uv_ip4_name((const sockaddr_in*)addr, endpoint, 16);
			auto port = ntohs(((const sockaddr_in*)addr)->sin_port);

			//raw data in
			if(nread > 0) {
				EQ::Net::StaticPacket wire((void*)buf->base, (uint64_t)nread);
				Log.OutF(Logs::Detail, Logs::Netcode, "Raw Packet in\n{0}", wire.ToString());
			}

			c->ProcessPacket(endpoint, port, buf->base, nread);
			delete[] buf->base;
		});

		m_attached = loop;
	}
}

void EQ::Net::DaybreakConnectionManager::Detach()
{
	if (m_attached) {
		uv_udp_recv_stop(&m_socket);
		uv_timer_stop(&m_timer);
		m_attached = nullptr;
	}
}

void EQ::Net::DaybreakConnectionManager::Connect(const std::string &addr, int port)
{
	//todo dns resolution
	
	auto connection = std::shared_ptr<DaybreakConnection>(new DaybreakConnection(this, addr, port));
	connection->m_self = connection;
	
	if (m_on_new_connection) {
		m_on_new_connection(connection);
	}
	
	m_connections.insert(std::make_pair(std::make_pair(addr, port), connection));
}

void EQ::Net::DaybreakConnectionManager::Process()
{
	auto now = Clock::now();
	auto iter = m_connections.begin();
	while (iter != m_connections.end()) {
		auto connection = iter->second;
		auto status = connection->m_status;

		if (status == StatusDisconnecting) {
			iter = m_connections.erase(iter);
			connection->FlushBuffer();
			connection->ChangeStatus(StatusDisconnected);
			continue;
		}

		if (status == StatusConnecting) {
			auto time_since_last_recv = std::chrono::duration_cast<std::chrono::milliseconds>(now - connection->m_last_recv);
			if ((size_t)time_since_last_recv.count() > m_options.connect_stale_ms) {
				iter = m_connections.erase(iter);
				connection->ChangeStatus(StatusDisconnecting);
				continue;
			}
		} else if (status == StatusConnected) {
			auto time_since_last_recv = std::chrono::duration_cast<std::chrono::milliseconds>(now - connection->m_last_recv);
			if ((size_t)time_since_last_recv.count() > m_options.stale_connection_ms) {
				iter = m_connections.erase(iter);
				connection->ChangeStatus(StatusDisconnecting);
				continue;
			}
		}

		switch (status)
		{
		case StatusConnecting: {
			auto time_since_last_send = std::chrono::duration_cast<std::chrono::milliseconds>(now - connection->m_last_send);
			if ((size_t)time_since_last_send.count() > m_options.connect_delay_ms) {
				connection->SendConnect();
			}
		}
			break;
		case StatusConnected: {
			if (m_options.keepalive_delay_ms != 0) {
				auto time_since_last_send = std::chrono::duration_cast<std::chrono::milliseconds>(now - connection->m_last_send);
				if ((size_t)time_since_last_send.count() > m_options.keepalive_delay_ms) {
					connection->SendKeepAlive();
				}
			}
		}
		case StatusDisconnecting:
			connection->Process();
			break;
		default:
			break;
		}

		iter++;
	}
}

void EQ::Net::DaybreakConnectionManager::ProcessResend()
{
	auto iter = m_connections.begin();
	while (iter != m_connections.end()) {
		auto connection = iter->second;
		auto status = connection->m_status;

		switch (status)
		{
		case StatusConnected:
		case StatusDisconnecting:
			connection->ProcessResend();
			break;
		default:
			break;
		}

		iter++;
	}
}

void EQ::Net::DaybreakConnectionManager::ProcessPacket(const std::string &endpoint, int port, const char *data, uint64_t size)
{
	if (m_options.simulated_in_packet_loss && m_options.simulated_in_packet_loss >= m_rand.Int(0, 100)) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Dropped a packet from {0}:{1} because of simulated packet loss", endpoint, port);
		return;
	}

	if (size < DaybreakHeader::size()) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Packet of size {0} which is less than {1}", size, DaybreakHeader::size());
		return;
	}

	try {
		auto connection = FindConnectionByEndpoint(endpoint, port);
		if (connection) {
			StaticPacket p((void*)data, size);
			connection->ProcessPacket(p);
		}
		else {
			if (data[0] == 0 && data[1] == OP_SessionRequest) {
				StaticPacket p((void*)data, size);
				auto request = p.GetSerialize<DaybreakConnect>(0);

				connection = std::shared_ptr<DaybreakConnection>(new DaybreakConnection(this, request, endpoint, port));
				connection->m_self = connection;

				if (m_on_new_connection) {
					m_on_new_connection(connection);
				}
				m_connections.insert(std::make_pair(std::make_pair(endpoint, port), connection));
				connection->ProcessPacket(p);
			}
			else if(data[1] != OP_OutOfSession) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Recv {0:#x} from {1}:{2} but there is no identified connection from this address.", data[1], endpoint, port);
				SendDisconnect(endpoint, port);
			}
		}
	}
	catch (std::exception &ex) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Error processing packet: {0}", ex.what());
	}
}

std::shared_ptr<EQ::Net::DaybreakConnection> EQ::Net::DaybreakConnectionManager::FindConnectionByEndpoint(std::string addr, int port)
{
	auto p = std::make_pair(addr, port);
	auto iter = m_connections.find(p);
	if (iter != m_connections.end()) {
		return iter->second;
	}

	return nullptr;
}

void EQ::Net::DaybreakConnectionManager::SendDisconnect(const std::string &addr, int port)
{
	DaybreakDisconnect header;
	header.zero = 0;
	header.opcode = OP_SessionDisconnect;
	header.connect_code = 0;
	
	DynamicPacket out;
	out.PutSerialize(0, header);
	
	uv_udp_send_t *send_req = new uv_udp_send_t;
	sockaddr_in send_addr;
	uv_ip4_addr(addr.c_str(), port, &send_addr);
	uv_buf_t send_buffers[1];

	char *data = new char[(size_t)out.Length()];
	memcpy(data, out.Data(), (size_t)out.Length());
	send_buffers[0] = uv_buf_init(data, (unsigned int)out.Length());
	send_req->data = send_buffers[0].base;

	Log.OutF(Logs::Detail, Logs::Netcode, "Sending disconnect to {0}:{1}", addr, port);

	int ret = uv_udp_send(send_req, &m_socket, send_buffers, 1, (sockaddr*)&send_addr,
		[](uv_udp_send_t* req, int status) {
		delete[] (char*)req->data;
		delete req;
	});
}

//new connection made as server
EQ::Net::DaybreakConnection::DaybreakConnection(DaybreakConnectionManager *owner, const DaybreakConnect &connect, const std::string &endpoint, int port)
{
	m_owner = owner;
	m_last_send = Clock::now();
	m_last_recv = Clock::now();
	m_status = StatusConnected;
	m_endpoint = endpoint;
	m_port = port;
	m_connect_code = NetworkToHost(connect.connect_code);
	m_encode_key = m_owner->m_rand.Int(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
	m_max_packet_size = (uint32_t)std::min((uint32_t)owner->m_options.max_packet_size, (uint32_t)NetworkToHost(connect.max_packet_size));
	m_crc_bytes = (uint32_t)owner->m_options.crc_length;
	m_encode_passes[0] = owner->m_options.encode_passes[0];
	m_encode_passes[1] = owner->m_options.encode_passes[1];
	m_hold_time = Clock::now();
	m_buffered_packets_length = 0;
	m_resend_delay = m_owner->m_options.resend_delay_ms;
	m_rolling_ping = 100;
	m_combined.reset(new char[512]);
	m_combined[0] = 0;
	m_combined[1] = OP_Combined;
	m_last_session_stats = Clock::now();
	m_last_stats = Clock::now();
}

//new connection made as client
EQ::Net::DaybreakConnection::DaybreakConnection(DaybreakConnectionManager *owner, const std::string &endpoint, int port)
{
	m_owner = owner;
	m_last_send = Clock::now();
	m_last_recv = Clock::now();
	m_status = StatusConnecting;
	m_endpoint = endpoint;
	m_port = port;
	m_connect_code = m_owner->m_rand.Int(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
	m_encode_key = 0;
	m_max_packet_size = (uint32_t)owner->m_options.max_packet_size;
	m_crc_bytes = 0;
	m_hold_time = Clock::now();
	m_buffered_packets_length = 0;
	m_resend_delay = m_owner->m_options.resend_delay_ms;
	m_rolling_ping = 100;
	m_combined.reset(new char[512]);
	m_combined[0] = 0;
	m_combined[1] = OP_Combined;
	m_last_session_stats = Clock::now();
	m_last_stats = Clock::now();
}

EQ::Net::DaybreakConnection::~DaybreakConnection()
{
}

void EQ::Net::DaybreakConnection::Close()
{
	Log.OutF(Logs::Detail, Logs::Netcode, "Closing daybreak connection, status was {0}", (int)m_status);
	if (m_status == StatusConnected) {
		DaybreakDisconnect disconnect;
		disconnect.zero = 0;
		disconnect.opcode = OP_SessionDisconnect;
		disconnect.connect_code = HostToNetwork(m_connect_code);
		DynamicPacket out;
		out.PutSerialize(0, disconnect);
		InternalSend(out);

		ChangeStatus(StatusDisconnecting);
	}
	else {
		ChangeStatus(StatusDisconnecting);
	}
}

void EQ::Net::DaybreakConnection::QueuePacket(Packet &p)
{
	QueuePacket(p, 0, true);
}

void EQ::Net::DaybreakConnection::QueuePacket(Packet &p, int stream)
{
	QueuePacket(p, stream, true);
}

void EQ::Net::DaybreakConnection::QueuePacket(Packet &p, int stream, bool reliable)
{
	if (*(char*)p.Data() == 0) {
		DynamicPacket packet;
		packet.PutUInt8(0, 0);
		packet.PutPacket(1, p);
		InternalQueuePacket(packet, stream, reliable);
	}

	InternalQueuePacket(p, stream, reliable);
}

void EQ::Net::DaybreakConnection::ResetStats()
{
	m_stats = DaybreakConnectionStats();
}

void EQ::Net::DaybreakConnection::Process()
{
	try {
		m_resend_delay = (uint64_t)(m_stats.last_stat_ping * m_owner->m_options.resend_delay_factor) + m_owner->m_options.resend_delay_ms;
		if (m_resend_delay > 1000) {
			m_resend_delay = 1000;
		}

		auto now = Clock::now();
		auto time_since_hold = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - m_hold_time).count();
		if (time_since_hold >= m_owner->m_options.hold_length_ms) {
			FlushBuffer();
		}

		ProcessQueue();

		auto time_since_stats = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_stats).count();
		if (m_owner->m_options.stats_delay_ms > 0 && time_since_stats >= m_owner->m_options.stats_delay_ms) {
			SendStatSync();
			m_last_stats = now;
		}
	}
	catch (std::exception ex) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Error processing connection: {0}", ex.what());
	}
}

void EQ::Net::DaybreakConnection::ProcessPacket(Packet &p)
{
	m_last_recv = Clock::now();
	m_stats.recv_packets++;
	m_stats.recv_bytes += p.Length();

	if (p.Length() < 1) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Error parsing packet, length was < 1");
		return;
	}

	if (p.GetInt8(0) != 0) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Error parsing packet, did not start with a 0 frame, not a valid protocol packet.");
		return;
	}
	
	auto opcode = p.GetInt8(1);
	if (opcode == OP_KeepAlive || opcode == OP_OutboundPing) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Discard packet of type {0:#x}", opcode);
		return;
	}

	if (PacketCanBeEncoded(p)) {
		if (!ValidateCRC(p)) {
			Log.OutF(Logs::Detail, Logs::Netcode, "Tossed packet that failed CRC of type {0:#x}", p.Length() >= 2 ? p.GetInt8(1) : 0);
			return;
		}

		EQ::Net::DynamicPacket temp;
		temp.PutPacket(0, p);
		temp.Resize(temp.Length() - m_crc_bytes);

		for (int i = 1; i >= 0; --i) {
			switch (m_encode_passes[i]) {
			case EncodeCompression:
				Decompress(temp, DaybreakHeader::size(), temp.Length() - DaybreakHeader::size());
				break;
			case EncodeXOR:
				Decode(temp, DaybreakHeader::size(), temp.Length() - DaybreakHeader::size());
				break;
			default:
				break;
			}
		}

		ProcessDecodedPacket(StaticPacket(temp.Data(), temp.Length()));
	}
	else {
		ProcessDecodedPacket(p);
	}
}

void EQ::Net::DaybreakConnection::ProcessQueue()
{
	for (int i = 0; i < 4; ++i) {
		auto stream = &m_streams[i];
		for (;;) {
			
			auto iter = stream->packet_queue.find(stream->sequence_in);
			if (iter == stream->packet_queue.end()) {
				break;
			}
	
			auto packet = iter->second;
			stream->packet_queue.erase(iter);
			ProcessDecodedPacket(*packet);
			delete packet;
		}
	}
}

void EQ::Net::DaybreakConnection::RemoveFromQueue(int stream, uint16_t seq)
{
	auto s = &m_streams[stream];
	auto iter = s->packet_queue.find(seq);
	if (iter != s->packet_queue.end()) {
		auto packet = iter->second;
		s->packet_queue.erase(iter);
		delete packet;
	}
}

void EQ::Net::DaybreakConnection::AddToQueue(int stream, uint16_t seq, const Packet &p)
{
	auto s = &m_streams[stream];
	auto iter = s->packet_queue.find(seq);
	if (iter == s->packet_queue.end()) {
		DynamicPacket *out = new DynamicPacket();
		out->PutPacket(0, p);

		s->packet_queue.insert(std::make_pair(seq, out));
	}
}

void EQ::Net::DaybreakConnection::ProcessDecodedPacket(const Packet &p)
{
	if (p.GetInt8(0) == 0) {
		if (p.Length() < 2) {
			Log.OutF(Logs::Detail, Logs::Netcode, "Failed to process decoded packet, length < 2\n{0}", p.ToString());
			return;
		}

		Log.OutF(Logs::Detail, Logs::Netcode, "Processing decoded packet of type {0:#x}", p.GetInt8(1));
		switch (p.GetInt8(1)) {
		case OP_Combined: {
			Log.OutF(Logs::Detail, Logs::Netcode, "Processing OP_Combined of length {0}", p.Length());
			char *current = (char*)p.Data() + 2;
			char *end = (char*)p.Data() + p.Length();
			while (current < end) {
				uint8_t subpacket_length = *(uint8_t*)current;
				current += 1;

				Log.OutF(Logs::Detail, Logs::Netcode, "Subpacket length {0}", subpacket_length);

				if (end < current + subpacket_length) {
					Log.OutF(Logs::Detail, Logs::Netcode, "Subpacket length out of range, bailing");
					return;
				}

				ProcessDecodedPacket(StaticPacket(current, subpacket_length));
				current += subpacket_length;
			}
			break;
		}
		
		case OP_AppCombined:
		{
			Log.OutF(Logs::Detail, Logs::Netcode, "Processing OP_AppCombined of length {0}", p.Length());
			uint8_t *current = (uint8_t*)p.Data() + 2;
			uint8_t *end = (uint8_t*)p.Data() + p.Length();

			while (current < end) {
				uint32_t subpacket_length = 0;
				if (*current == 0xFF)
				{
					if (end < current + 3) {
						Log.OutF(Logs::Detail, Logs::Netcode, "Error in OP_AppCombined, end < current + 3");
						throw std::out_of_range("Error in OP_AppCombined, end < current + 3");
					}

					if (*(current + 1) == 0xFF && *(current + 2) == 0xFF) {
						if (end < current + 7) {
							Log.OutF(Logs::Detail, Logs::Netcode, "Error in OP_AppCombined, end < current + 7");
							throw std::out_of_range("Error in OP_AppCombined, end < current + 7");
						}

						subpacket_length = (uint32_t)(
							(*(current + 3) << 24) | 
							(*(current + 4) << 16) | 
							(*(current + 5) << 8) | 
							(*(current + 6))
							);
						current += 7;
					}
					else {
						subpacket_length = (uint32_t)(
							(*(current + 1) << 8) |
							(*(current + 2))
							);
						current += 3;
					}
				}
				else {
					subpacket_length = (uint32_t)((*(current + 0)));
					current += 1;
				}

				Log.OutF(Logs::Detail, Logs::Netcode, "Processing subpacket of length {0}", subpacket_length);
				ProcessDecodedPacket(StaticPacket(current, subpacket_length));
				current += subpacket_length;
			}
		}

		case OP_SessionRequest:
		{
			Log.OutF(Logs::Detail, Logs::Netcode, "OP_SessionRequest recieved current status is {0}", m_status);
			if (m_status == StatusConnected) {
				auto request = p.GetSerialize<DaybreakConnect>(0);

				if (NetworkToHost(request.connect_code) != m_connect_code) {
					return;
				}

				DaybreakConnectReply reply;
				reply.zero = 0;
				reply.opcode = OP_SessionResponse;
				reply.connect_code = HostToNetwork(m_connect_code);
				reply.encode_key = HostToNetwork(m_encode_key);
				reply.crc_bytes = m_crc_bytes;
				reply.max_packet_size = HostToNetwork(m_max_packet_size);
				reply.encode_pass1 = m_encode_passes[0];
				reply.encode_pass2 = m_encode_passes[1];
				DynamicPacket p;
				p.PutSerialize(0, reply);
				InternalSend(p);
			}

			break;
		}

		case OP_SessionResponse:
		{
			Log.OutF(Logs::Detail, Logs::Netcode, "OP_SessionResponse recieved current status is {0}", m_status);
			if (m_status == StatusConnecting) {
				auto reply = p.GetSerialize<DaybreakConnectReply>(0);

				if (m_connect_code == reply.connect_code) {
					m_encode_key = reply.encode_key;
					m_crc_bytes = reply.crc_bytes;
					m_encode_passes[0] = (DaybreakEncodeType)reply.encode_pass1;
					m_encode_passes[1] = (DaybreakEncodeType)reply.encode_pass2;
					m_max_packet_size = reply.max_packet_size;
					ChangeStatus(StatusConnected);
				}
			}
			break;
		}

		case OP_Packet:
		case OP_Packet2:
		case OP_Packet3:
		case OP_Packet4:
		{
			auto header = p.GetSerialize<DaybreakReliableHeader>(0);
			auto sequence = NetworkToHost(header.sequence);
			auto stream_id = header.opcode - OP_Packet;
			auto stream = &m_streams[stream_id];

			Log.OutF(Logs::Detail, Logs::Netcode, "Recv Reliable Packet for stream {0} with sequence {1}", stream_id, sequence);

			auto order = CompareSequence(stream->sequence_in, sequence);
			if (order == SequenceFuture) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Reliable packet resolved to be a future packet, adding to the packet queue");
				SendOutOfOrderAck(stream_id, sequence);
				AddToQueue(stream_id, sequence, p);
			}
			else if (order == SequencePast) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Reliable packet resolved to be a past packet, sending an ack");
				SendAck(stream_id, stream->sequence_in - 1);
			}
			else {
				Log.OutF(Logs::Detail, Logs::Netcode, "Reliable packet is the one we want, removing it from queue and processing it.");
				RemoveFromQueue(stream_id, sequence);
				SendAck(stream_id, stream->sequence_in);
				stream->sequence_in++;
				StaticPacket next((char*)p.Data() + DaybreakReliableHeader::size(), p.Length() - DaybreakReliableHeader::size());
				ProcessDecodedPacket(next);
			}

			break;
		}

		case OP_Fragment:
		case OP_Fragment2:
		case OP_Fragment3:
		case OP_Fragment4:
		{
			auto header = p.GetSerialize<DaybreakReliableHeader>(0);
			auto sequence = NetworkToHost(header.sequence);
			auto stream_id = header.opcode - OP_Fragment;
			auto stream = &m_streams[stream_id];

			Log.OutF(Logs::Detail, Logs::Netcode, "Recv Reliable Fragment Packet for stream {0} with sequence {1}", stream_id, sequence);

			auto order = CompareSequence(stream->sequence_in, sequence);
			
			if (order == SequenceFuture) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Reliable fragment packet resolved to be a future packet, adding to the packet queue");
				SendOutOfOrderAck(stream_id, sequence);
				AddToQueue(stream_id, sequence, p);
			}
			else if (order == SequencePast) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Reliable fragment packet resolved to be a past packet, sending an ack");
				SendAck(stream_id, stream->sequence_in - 1);
			}
			else {
				Log.OutF(Logs::Detail, Logs::Netcode, "Reliable fragment packet is the one we want, removing it from queue and processing it.");
				RemoveFromQueue(stream_id, sequence);
				SendAck(stream_id, stream->sequence_in);
				stream->sequence_in++;
			
				if (stream->fragment_total_bytes == 0) {
					auto fragheader = p.GetSerialize<DaybreakReliableFragmentHeader>(0);
					stream->fragment_total_bytes = NetworkToHost(fragheader.total_size);
					stream->fragment_current_bytes = 0;
					stream->fragment_packet.Reserve(stream->fragment_total_bytes);
					stream->fragment_packet.PutData(
						stream->fragment_current_bytes,
						(char*)p.Data() + DaybreakReliableFragmentHeader::size(), p.Length() - DaybreakReliableFragmentHeader::size());

					stream->fragment_current_bytes += (uint32_t)(p.Length() - DaybreakReliableFragmentHeader::size());

					Log.OutF(Logs::Detail, Logs::Netcode, "Fragment packet created {0}/{1} bytes", stream->fragment_current_bytes, stream->fragment_total_bytes);
				}
				else {
					stream->fragment_packet.PutData(
						stream->fragment_current_bytes,
						(char*)p.Data() + DaybreakReliableHeader::size(), p.Length() - DaybreakReliableHeader::size());

					stream->fragment_current_bytes += (uint32_t)(p.Length() - DaybreakReliableHeader::size());
					
					if (stream->fragment_current_bytes >= stream->fragment_total_bytes) {
						ProcessDecodedPacket(stream->fragment_packet);
						stream->fragment_packet.Clear();
						stream->fragment_total_bytes = 0;
						stream->fragment_current_bytes = 0;
					}

					Log.OutF(Logs::Detail, Logs::Netcode, "Fragment packet accumulated {0}/{1} bytes", stream->fragment_current_bytes, stream->fragment_total_bytes);
				}
			}

			break;
		}

		case OP_Ack:
		case OP_Ack2:
		case OP_Ack3:
		case OP_Ack4:
		{
			auto header = p.GetSerialize<DaybreakReliableHeader>(0);
			auto sequence = NetworkToHost(header.sequence);
			auto stream_id = header.opcode - OP_Ack;

			Log.OutF(Logs::Detail, Logs::Netcode, "Ack recv for stream {0} and sequence {1}", stream_id, sequence);
			Ack(stream_id, sequence);
			break;
		}

		case OP_OutOfOrderAck:
		case OP_OutOfOrderAck2:
		case OP_OutOfOrderAck3:
		case OP_OutOfOrderAck4:
		{
			auto header = p.GetSerialize<DaybreakReliableHeader>(0);
			auto sequence = NetworkToHost(header.sequence);
			auto stream_id = header.opcode - OP_OutOfOrderAck;

			Log.OutF(Logs::Detail, Logs::Netcode, "Out of Order Ack recv for stream {0} and sequence {1}", stream_id, sequence);
			OutOfOrderAck(stream_id, sequence);
			break;
		}

		case OP_SessionDisconnect:
		{
			Log.OutF(Logs::Detail, Logs::Netcode, "OP_SessionDisconnect recieved current status is {0}", m_status);
			if (m_status == StatusConnected || m_status == StatusDisconnecting) {
				DaybreakDisconnect disconnect;
				disconnect.zero = 0;
				disconnect.opcode = OP_SessionDisconnect;
				disconnect.connect_code = HostToNetwork(m_connect_code);
				DynamicPacket out;
				out.PutSerialize(0, disconnect);
				InternalSend(out);
			}

			ChangeStatus(StatusDisconnecting);
			break;
		}

		case OP_Padding:
		{
			Log.OutF(Logs::Detail, Logs::Netcode, "OP_Padding recv, processing packet");
			auto self = m_self.lock();
			if (m_owner->m_on_packet_recv && self) {
				m_owner->m_on_packet_recv(self, StaticPacket((char*)p.Data() + 1, p.Length() - 1));
			}
			break;
		}
		case OP_SessionStatRequest:
		{
			Log.OutF(Logs::Detail, Logs::Netcode, "OP_SessionStatRequest recv");
			auto request = p.GetSerialize<DaybreakSessionStatRequest>(0);

			DaybreakSessionStatResponse response;
			response.zero = 0;
			response.opcode = OP_SessionStatResponse;
			response.timestamp = request.timestamp;
			response.our_timestamp = (uint32_t)EQ::Net::HostToNetwork(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
			response.client_sent = request.packets_sent;
			response.client_recv = request.packets_recv;
			response.server_sent = EQ::Net::HostToNetwork(m_stats.sent_packets);
			response.server_recv = EQ::Net::HostToNetwork(m_stats.recv_packets);
			DynamicPacket out;
			out.PutSerialize(0, response);
			InternalSend(out);
			break;
		}
		case OP_SessionStatResponse:
			Log.OutF(Logs::Detail, Logs::Netcode, "OP_SessionStatResponse recv, not yet implemented.");
			break;
		default:
			Log.OutF(Logs::Detail, Logs::Netcode, "Unhandled opcode {0:#x}", p.GetInt8(1));
			break;
		}
	}
	else {
		Log.OutF(Logs::Detail, Logs::Netcode, "Recv raw app packet, processing");
		auto self = m_self.lock();
		if (m_owner->m_on_packet_recv && self) {
			m_owner->m_on_packet_recv(self, p);
		}
	}
}

bool EQ::Net::DaybreakConnection::ValidateCRC(Packet &p)
{
	if (m_crc_bytes == 0U) {
		return true;
	}

	if (p.Length() < (uint64_t)m_crc_bytes) {
		return false;
	}

	char *data = (char*)p.Data();
	int calculated = 0;
	int actual = 0;
	switch (m_crc_bytes) {
	case 2:
		actual = NetworkToHost(*(int16_t*)&data[p.Length() - (uint64_t)m_crc_bytes]) & 0xffff;
		calculated = Crc32(data, (int)(p.Length() - (uint64_t)m_crc_bytes), m_encode_key) & 0xffff;
		break;
	case 4:
		actual = NetworkToHost(*(int32_t*)&data[p.Length() - (uint64_t)m_crc_bytes]);
		calculated = Crc32(data, (int)(p.Length() - (uint64_t)m_crc_bytes), m_encode_key);
		break;
	default:
		return false;
	}

	if (actual == calculated) {
		return true;
	}

	return false;
}

void EQ::Net::DaybreakConnection::AppendCRC(Packet &p)
{
	if (m_crc_bytes == 0U) {
		return;
	}
	
	int calculated = 0;
	switch (m_crc_bytes) {
	case 2:
		calculated = Crc32(p.Data(), (int)p.Length(), m_encode_key) & 0xffff;
		p.PutInt16(p.Length(), EQ::Net::HostToNetwork((int16_t)calculated));
		break;
	case 4:
		calculated = Crc32(p.Data(), (int)p.Length(), m_encode_key);
		p.PutInt32(p.Length(), EQ::Net::HostToNetwork(calculated));
		break;
	}
}

void EQ::Net::DaybreakConnection::ChangeStatus(DbProtocolStatus new_status)
{
	if (m_owner->m_on_connection_state_change) {
		if (auto self = m_self.lock()) {
			m_owner->m_on_connection_state_change(self, m_status, new_status);
		}
	}

	Log.OutF(Logs::Detail, Logs::Netcode, "Status changed from {0} to {1}", (int)m_status, (int)new_status);
	m_status = new_status;
}

bool EQ::Net::DaybreakConnection::PacketCanBeEncoded(Packet &p) const
{
	if (p.Length() < 2) {
		return false;
	}

	auto opcode = p.GetInt8(1);
	if (opcode == OP_SessionRequest || opcode == OP_SessionResponse || opcode == OP_OutOfSession) {
		return false;
	}

	return true;
}

void EQ::Net::DaybreakConnection::Decode(Packet &p, uint64_t offset, uint64_t length)
{
	int key = m_encode_key;
	char *buffer = (char*)p.Data() + offset;

	uint64_t i = 0;
	for (i = 0; i + 4 <= length; i += 4)
	{
		int pt = (*(int*)&buffer[i]) ^ (key);
		key = (*(int*)&buffer[i]);
		*(int*)&buffer[i] = pt;
	}

	unsigned char KC = key & 0xFF;
	for (; i < length; i++)
	{
		buffer[i] = buffer[i] ^ KC;
	}
}

void EQ::Net::DaybreakConnection::Encode(Packet &p, uint64_t offset, uint64_t length)
{
	int key = m_encode_key;
	char *buffer = (char*)p.Data() + offset;

	uint64_t i = 0;
	for (i = 0; i + 4 <= length; i += 4)
	{
		int pt = (*(int*)&buffer[i]) ^ (key);
		key = pt;
		*(int*)&buffer[i] = pt;
	}

	unsigned char KC = key & 0xFF;
	for (; i < length; i++)
	{
		buffer[i] = buffer[i] ^ KC;
	}
}

uint32_t Inflate(const uint8_t* in, uint32_t in_len, uint8_t* out, uint32_t out_len) {
	if (!in) {
		return 0;
	}

	z_stream zstream;
	memset(&zstream, 0, sizeof(zstream));
	int zerror = 0;
	int i;

	zstream.next_in = const_cast<unsigned char *>(in);
	zstream.avail_in = in_len;
	zstream.next_out = out;
	zstream.avail_out = out_len;
	zstream.opaque = Z_NULL;

	i = inflateInit2(&zstream, 15);

	if (i != Z_OK) {
		return 0;
	}

	zerror = inflate(&zstream, Z_FINISH);

	if (zerror == Z_STREAM_END) {
		inflateEnd(&zstream);
		return zstream.total_out;
	}
	else {
		if (zerror == Z_MEM_ERROR && !zstream.msg)
		{
			return 0;
		}

		zerror = inflateEnd(&zstream);
		return 0;
	}
}

uint32_t Deflate(const uint8_t* in, uint32_t in_len, uint8_t* out, uint32_t out_len) {
	if (!in) {
		return 0;
	}

	z_stream zstream;
	memset(&zstream, 0, sizeof(zstream));
	int zerror;

	zstream.next_in = const_cast<unsigned char *>(in);
	zstream.avail_in = in_len;
	zstream.opaque = Z_NULL;

	deflateInit(&zstream, Z_FINISH);
	zstream.next_out = out;
	zstream.avail_out = out_len;

	zerror = deflate(&zstream, Z_FINISH);

	if (zerror == Z_STREAM_END)
	{
		deflateEnd(&zstream);
		return zstream.total_out;
	}
	else {
		zerror = deflateEnd(&zstream);
		return 0;
	}
}

void EQ::Net::DaybreakConnection::Decompress(Packet &p, uint64_t offset, uint64_t length)
{
	if (length < 2) {
		return;
	}

	static uint8_t new_buffer[4096];
	uint8_t *buffer = (uint8_t*)p.Data() + offset;
	uint32_t new_length = 0;

	if (buffer[0] == 0x5a) {
		new_length = Inflate(buffer + 1, (uint32_t)length - 1, new_buffer, 4096);
	}
	else if (buffer[0] == 0xa5) {
		memcpy(new_buffer, buffer + 1, (size_t)length - 1);
		new_length = (uint32_t)length - 1;
	}
	else {
		return;
	}

	p.Resize(offset);
	p.PutData(offset, new_buffer, new_length);
}

void EQ::Net::DaybreakConnection::Compress(Packet &p, uint64_t offset, uint64_t length)
{
	uint8_t new_buffer[2048] = { 0 };
	uint8_t *buffer = (uint8_t*)p.Data() + offset;
	uint32_t new_length = 0;

	if (length > 30) {
		new_length = Deflate(buffer, (uint32_t)length, new_buffer + 1, 2048) + 1;
		new_buffer[0] = 0x5a;
	}
	else {
		memcpy(new_buffer + 1, buffer, (size_t)length);
		new_buffer[0] = 0xa5;
		new_length = (size_t)length + 1;
	}

	p.Resize(offset);
	p.PutData(offset, new_buffer, new_length);
}

void EQ::Net::DaybreakConnection::ProcessResend()
{
	for (int i = 0; i < 4; ++i) {
		ProcessResend(i);
	}
}

void EQ::Net::DaybreakConnection::ProcessResend(int stream)
{
	auto now = Clock::now();
	auto s = &m_streams[stream];
	for (auto &entry : s->sent_packets) {
		auto time_since_last_send = std::chrono::duration_cast<std::chrono::milliseconds>(now - entry.second.last_sent);
		if(entry.second.times_resent == 0) {
			if ((uint64_t)time_since_last_send.count() > m_resend_delay) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Resending reliable packet\n{0}", entry.second.packet.ToString());
				InternalBufferedSend(entry.second.packet);
				entry.second.last_sent = now;
				entry.second.times_resent++;
			}
		}
		else {
			if ((uint64_t)time_since_last_send.count() > std::min(m_resend_delay / (entry.second.times_resent + 1), (uint64_t)50ULL)) {
				Log.OutF(Logs::Detail, Logs::Netcode, "Resending reliable packet\n{0}", entry.second.packet.ToString());
				InternalBufferedSend(entry.second.packet);
				entry.second.last_sent = now;
				entry.second.times_resent++;
			}
		}
	}
}

void EQ::Net::DaybreakConnection::Ack(int stream, uint16_t seq)
{
	auto now = Clock::now();
	auto s = &m_streams[stream];
	auto iter = s->sent_packets.begin();
	while (iter != s->sent_packets.end()) {
		if (iter->first <= seq) {
			uint64_t round_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - iter->second.last_sent).count();
			m_stats.total_ping += round_time;
			m_stats.total_acks++;
			m_stats.max_ping = std::max(m_stats.max_ping, round_time);
			m_stats.min_ping = std::min(m_stats.min_ping, round_time);
			m_rolling_ping = (m_rolling_ping + round_time) / 2;
			iter = s->sent_packets.erase(iter);
		}
		else {
			++iter;
		}
	}
}

void EQ::Net::DaybreakConnection::OutOfOrderAck(int stream, uint16_t seq)
{
	auto now = Clock::now();
	auto s = &m_streams[stream];
	auto iter = s->sent_packets.find(seq);
	if (iter != s->sent_packets.end()) {
		uint64_t round_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - iter->second.last_sent).count();
		m_stats.total_ping += round_time;
		m_stats.total_acks++;
		m_stats.max_ping = std::max(m_stats.max_ping, round_time);
		m_stats.min_ping = std::min(m_stats.min_ping, round_time);
		m_rolling_ping = (m_rolling_ping + round_time) / 2;
		s->sent_packets.erase(iter);
	}
}

void EQ::Net::DaybreakConnection::SendAck(int stream_id, uint16_t seq)
{
	DaybreakReliableHeader ack;
	ack.zero = 0;
	ack.opcode = OP_Ack + stream_id;
	ack.sequence = HostToNetwork(seq);

	DynamicPacket p;
	p.PutSerialize(0, ack);

	InternalBufferedSend(p);
}

void EQ::Net::DaybreakConnection::SendOutOfOrderAck(int stream_id, uint16_t seq)
{
	DaybreakReliableHeader ack;
	ack.zero = 0;
	ack.opcode = OP_OutOfOrderAck + stream_id;
	ack.sequence = HostToNetwork(seq);

	DynamicPacket p;
	p.PutSerialize(0, ack);

	InternalBufferedSend(p);
}

void EQ::Net::DaybreakConnection::SendStatSync()
{
	DaybreakSessionStatRequest request;
	request.zero = 0;
	request.opcode = OP_SessionStatRequest;
	request.timestamp = (uint32_t)EQ::Net::HostToNetwork(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() & 0xFFFFLL);
	request.stat_ping = (uint32_t)m_stats.last_stat_ping;
	if (m_stats.total_stat_count > 0)
		request.avg_ping = (uint32)(m_stats.total_stat_ping / m_stats.total_stat_count);
	else
		request.avg_ping = 0;
	request.min_ping = (uint32_t)m_stats.min_stat_ping;
	request.max_ping = (uint32_t)m_stats.max_stat_ping;
	request.last_ping = (uint32_t)m_stats.last_stat_ping;
	request.packets_sent = (uint32_t)m_stats.sent_packets + 1;
	request.packets_recv = (uint32_t)m_stats.recv_packets;
	m_last_session_stats = Clock::now();

	EQ::Net::DynamicPacket p;
	p.PutSerialize(0, request);
	InternalSend(p);
}

void EQ::Net::DaybreakConnection::InternalBufferedSend(Packet &p)
{
	if (p.Length() > 0xFFU) {
		FlushBuffer();
		InternalSend(p);
		return;
	}

	//we could add this packet to a combined
	uint64_t raw_size = DaybreakHeader::size() + (uint64_t)m_crc_bytes + m_buffered_packets_length + m_buffered_packets.size() + 1 + p.Length();
	if (raw_size > m_max_packet_size) {
		FlushBuffer();
	}

	DynamicPacket copy;
	copy.PutPacket(0, p);
	m_buffered_packets.push_back(copy);
	m_buffered_packets_length += p.Length();

	if (m_buffered_packets_length + m_buffered_packets.size() > m_owner->m_options.hold_size) {
		FlushBuffer();
	}
}

void EQ::Net::DaybreakConnection::SendConnect()
{
	DaybreakConnect connect;
	connect.zero = 0;
	connect.opcode = OP_SessionRequest;
	connect.protocol_version = HostToNetwork(3U);
	connect.connect_code = (uint32_t)HostToNetwork(m_connect_code);
	connect.max_packet_size = HostToNetwork((uint32_t)m_owner->m_options.max_packet_size);

	DynamicPacket p;
	p.PutSerialize(0, connect);

	InternalSend(p);
}

void EQ::Net::DaybreakConnection::SendKeepAlive()
{
	DaybreakHeader keep_alive;
	keep_alive.zero = 0;
	keep_alive.opcode = OP_KeepAlive;

	DynamicPacket p;
	p.PutSerialize(0, keep_alive);

	InternalSend(p);
}

void EQ::Net::DaybreakConnection::InternalSend(Packet &p)
{
	m_last_send = Clock::now();

	auto send_func = [](uv_udp_send_t* req, int status) {
		delete[](char*)req->data;
		delete req;
	};

	if (PacketCanBeEncoded(p)) {
		DynamicPacket out;
		out.PutPacket(0, p);

		for (int i = 0; i < 2; ++i) {
			switch (m_encode_passes[i]) {
			case EncodeCompression:
				Compress(out, DaybreakHeader::size(), out.Length() - DaybreakHeader::size());
				break;
			case EncodeXOR:
				Encode(out, DaybreakHeader::size(), out.Length() - DaybreakHeader::size());
				break;
			default:
				break;
			}
		}
		
		AppendCRC(out);

		uv_udp_send_t *send_req = new uv_udp_send_t;
		memset(send_req, 0, sizeof(*send_req));
		sockaddr_in send_addr;
		uv_ip4_addr(m_endpoint.c_str(), m_port, &send_addr);
		uv_buf_t send_buffers[1];

		char *data = new char[(size_t)out.Length()];
		memcpy(data, out.Data(), (size_t)out.Length());
		send_buffers[0] = uv_buf_init(data, (size_t)out.Length());
		send_req->data = send_buffers[0].base;

		m_stats.sent_bytes += out.Length();
		m_stats.sent_packets++;
		if (m_owner->m_options.simulated_out_packet_loss && m_owner->m_options.simulated_out_packet_loss >= m_owner->m_rand.Int(0, 100)) {
			delete[] (char*)send_req->data;
			delete send_req;
			return;
		}

		uv_udp_send(send_req, &m_owner->m_socket, send_buffers, 1, (sockaddr*)&send_addr, send_func);
		return;
	}

	uv_udp_send_t *send_req = new uv_udp_send_t;
	sockaddr_in send_addr;
	uv_ip4_addr(m_endpoint.c_str(), m_port, &send_addr);
	uv_buf_t send_buffers[1];
	
	char *data = new char[(size_t)p.Length()];
	memcpy(data, p.Data(), (size_t)p.Length());
	send_buffers[0] = uv_buf_init(data, (size_t)p.Length());
	send_req->data = send_buffers[0].base;
	
	m_stats.sent_bytes += p.Length();
	m_stats.sent_packets++;

	Log.OutF(Logs::Detail, Logs::Netcode, "Sending reliable packet\n{0}", p.ToString());

	if (m_owner->m_options.simulated_out_packet_loss && m_owner->m_options.simulated_out_packet_loss >= m_owner->m_rand.Int(0, 100)) {
		delete[] (char*)send_req->data;
		delete send_req;
		return;
	}

	uv_udp_send(send_req, &m_owner->m_socket, send_buffers, 1, (sockaddr*)&send_addr, send_func);
}

void EQ::Net::DaybreakConnection::InternalQueuePacket(Packet &p, int stream_id, bool reliable)
{
	if (!reliable) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Queue non-reliable packet for stream {0}\n{1}", p.ToString());

		auto max_raw_size = m_max_packet_size - m_crc_bytes;
		if (p.Length() > max_raw_size) {
			InternalQueuePacket(p, stream_id, true);
			return;
		}

		Log.OutF(Logs::Detail, Logs::Netcode, "Non-reliable packet was too large, queuing as a reliable");
		InternalBufferedSend(p);
		return;
	}

	Log.OutF(Logs::Detail, Logs::Netcode, "Queue reliable packet for stream {0}\n{1}", stream_id, p.ToString());

	auto stream = &m_streams[stream_id];
	auto max_raw_size = m_max_packet_size - m_crc_bytes - DaybreakReliableHeader::size();
	uint64_t length = p.Length();
	if (length > max_raw_size) {
		DaybreakReliableFragmentHeader first_header;
		first_header.reliable.zero = 0;
		first_header.reliable.opcode = OP_Fragment + stream_id;
		first_header.reliable.sequence = HostToNetwork(stream->sequence_out);
		first_header.total_size = (uint32_t)HostToNetwork((uint32_t)length);

		Log.OutF(Logs::Detail, Logs::Netcode, "Packet too large, fragmenting {0} total size", first_header.total_size);

		uint64_t used = 0;
		uint64_t sublen = m_max_packet_size - m_crc_bytes - DaybreakReliableFragmentHeader::size();
		DynamicPacket first_packet;
		first_packet.PutSerialize(0, first_header);
		first_packet.PutData(DaybreakReliableFragmentHeader::size(), (char*)p.Data() + used, sublen);
		used += sublen;

		DaybreakSentPacket sent;
		sent.packet.PutPacket(0, first_packet);
		sent.last_sent = Clock::now();
		sent.first_sent = Clock::now();
		sent.times_resent = 0;
		stream->sent_packets.insert(std::make_pair(stream->sequence_out, sent));
		stream->sequence_out++;

		Log.OutF(Logs::Detail, Logs::Netcode, "Fragmented packet {0}/{1}", used, first_header.total_size);

		InternalBufferedSend(first_packet);

		while (used < length) {
			auto left = length - used;
			DynamicPacket packet;
			DaybreakReliableHeader header;
			header.zero = 0;
			header.opcode = OP_Fragment + stream_id;
			header.sequence = HostToNetwork(stream->sequence_out);
			packet.PutSerialize(0, header);

			if (left > max_raw_size) {
				packet.PutData(DaybreakReliableHeader::size(), (char*)p.Data() + used, max_raw_size);
				used += max_raw_size;
			}
			else {
				packet.PutData(DaybreakReliableHeader::size(), (char*)p.Data() + used, left);
				used += left;
			}

			DaybreakSentPacket sent;
			sent.packet.PutPacket(0, packet);
			sent.last_sent = Clock::now();
			sent.first_sent = Clock::now();
			sent.times_resent = 0;
			stream->sent_packets.insert(std::make_pair(stream->sequence_out, sent));
			stream->sequence_out++;

			Log.OutF(Logs::Detail, Logs::Netcode, "Fragmented packet {0}/{1}", used, first_header.total_size);

			InternalBufferedSend(packet);
		}
	}
	else {
		DynamicPacket packet;
		DaybreakReliableHeader header;
		header.zero = 0;
		header.opcode = OP_Packet + stream_id;
		header.sequence = HostToNetwork(stream->sequence_out);
		packet.PutSerialize(0, header);
		packet.PutPacket(DaybreakReliableHeader::size(), p);

		DaybreakSentPacket sent;
		sent.packet.PutPacket(0, packet);
		sent.last_sent = Clock::now();
		sent.first_sent = Clock::now();
		sent.times_resent = 0;
		stream->sent_packets.insert(std::make_pair(stream->sequence_out, sent));
		stream->sequence_out++;

		Log.OutF(Logs::Detail, Logs::Netcode, "Reliable packet was able to be sent without fragmenting");

		InternalBufferedSend(packet);
	}
}

void EQ::Net::DaybreakConnection::FlushBuffer()
{
	if (m_buffered_packets.empty()) {
		return;
	}

	if (m_buffered_packets.size() > 1) {
		Log.OutF(Logs::Detail, Logs::Netcode, "Flushing buffer of size {0} packets and {1} bytes", m_buffered_packets.size(), m_buffered_packets_length);

		StaticPacket out(m_combined.get(), 512);
		uint64_t length = 2;
		for (auto &p : m_buffered_packets) {
			out.PutUInt8(length, (uint8_t)p.Length());
			out.PutPacket(length + 1, p);
			length += (1 + p.Length());
		}

		out.Resize(length);
		InternalSend(out);
	}
	else {
		Log.OutF(Logs::Detail, Logs::Netcode, "Flushing buffer of size 1 packet and {0} bytes", m_buffered_packets_length);

		auto &front = m_buffered_packets.front();
		InternalSend(front);
	}

	m_buffered_packets.clear();
	m_buffered_packets_length = 0;
}

EQ::Net::SequenceOrder EQ::Net::DaybreakConnection::CompareSequence(uint16_t expected, uint16_t actual) const
{
	int diff = (int)actual - (int)expected;
	
	if (diff == 0) {
		return SequenceCurrent;
	}
	
	if (diff > 0) {
		return SequenceFuture;
	}
	
	if (diff < -10000) {
		return SequenceFuture;
	}
	
	return SequencePast;
}
