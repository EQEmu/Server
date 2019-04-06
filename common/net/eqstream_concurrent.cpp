#include "eqstream_concurrent.h"
#include "eqstream_concurrent_message.h"
#include "../event/event_loop.h"
#include "../event/timer.h"
#include "../string_util.h"
#include "../opcodemgr.h"
#include "../eqemu_logsys.h"
#include "../eqemu_logsys_fmt.h"
#include "daybreak_connection.h"
#include <thread>
#include <concurrentqueue.h>
#include <unordered_map>
#include <queue>
#include <deque>
#include <list>

struct EQ::Net::ConcurrentEQStreamManager::Impl
{
	std::thread background;
	bool background_running;
	moodycamel::ConcurrentQueue<ceqs_msg_t> foreground_queue;
	moodycamel::ConcurrentQueue<ceqs_msg_t> background_queue;
	std::unordered_map<uint64_t, std::shared_ptr<DaybreakConnection>> connections;
	std::unique_ptr<EQ::Timer> foreground_loop_timer;
	std::unique_ptr<EQ::Timer> background_loop_timer;
	std::unique_ptr<EQ::Timer> background_update_stats_timer;
	std::unordered_map<uint64_t, std::shared_ptr<ConcurrentEQStream>> streams;
	std::function<void(std::shared_ptr<EQStreamInterface>)> on_new_connection;
	std::function<void(std::shared_ptr<EQStreamInterface>, EQ::Net::DbProtocolStatus, EQ::Net::DbProtocolStatus)> on_connection_state_change;
};

EQ::Net::ConcurrentEQStreamManager::ConcurrentEQStreamManager(const EQStreamManagerInterfaceOptions &options)
	: EQStreamManagerInterface(options)
{
	_impl.reset(new Impl());
	_impl->background = std::thread(std::bind(&ConcurrentEQStreamManager::_BackgroundThread, this));
	_impl->foreground_loop_timer.reset(new EQ::Timer(options.loop, 16, true,
		std::bind(&ConcurrentEQStreamManager::_ForegroundTimer, this, std::placeholders::_1)));
}

EQ::Net::ConcurrentEQStreamManager::~ConcurrentEQStreamManager()
{
	for (auto &s : _impl->streams) {
		s.second->_Invalidate();
	}

	_impl->foreground_loop_timer.release();

	//Tell the background to shutdown and wait for it to actually do so
	ceqs_terminate_msg_t msg;
	msg.type = TerminateBackground;

	_PushToBackgroundQueue((ceqs_msg_t*)&msg);
	_impl->background.join();

	//Go through our incoming messages to make sure we clean up any packets in that need to be freed
	ceqs_msg_t eqs_msg;

	while (_impl->foreground_queue.try_dequeue(eqs_msg)) {
		if (eqs_msg.type == PacketRecv) {
			ceqs_packet_recv_msg_t *eqs_msg_in = (ceqs_packet_recv_msg_t*)&eqs_msg;

			delete eqs_msg_in->packet;
		}
	}
}

void EQ::Net::ConcurrentEQStreamManager::_BackgroundThread() {
	_impl->background_running = true;
	EQ::EventLoop loop;
	auto &eqs_opts = GetOptions();
	auto opts = eqs_opts.daybreak_options;
	opts.loop = &loop;

	std::unique_ptr<DaybreakConnectionManager> dbcm(new DaybreakConnectionManager(opts));
	dbcm->OnNewConnection(std::bind(&ConcurrentEQStreamManager::DaybreakNewConnection, this, std::placeholders::_1));
	dbcm->OnConnectionStateChange(std::bind(&ConcurrentEQStreamManager::DaybreakConnectionStateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	dbcm->OnPacketRecv(std::bind(&ConcurrentEQStreamManager::DaybreakPacketRecv, this, std::placeholders::_1, std::placeholders::_2));

	_impl->background_loop_timer.reset(new EQ::Timer(&loop, 16, true,
		std::bind(&ConcurrentEQStreamManager::_BackgroundTimer, this, std::placeholders::_1)));

	_impl->background_update_stats_timer.reset(new EQ::Timer(&loop, 500, true,
		std::bind(&ConcurrentEQStreamManager::_BackgroundUpdateStatsTimer, this, std::placeholders::_1)));

	while (true == _impl->background_running) {
		loop.Process();
		Sleep(1);
	}

	_impl->background_loop_timer.release();
	_impl->background_update_stats_timer.release();
	dbcm.release();

	ceqs_msg_t eqs_msg;
	while (_impl->background_queue.try_dequeue(eqs_msg)) {
		if (eqs_msg.type == QueuePacket) {
			ceqs_queue_packet_msg_t *eqs_msg_in = (ceqs_queue_packet_msg_t*)&eqs_msg;
			delete eqs_msg_in->packet;
		}
	}
}
 
//Called by background
void EQ::Net::ConcurrentEQStreamManager::_BackgroundTimer(EQ::Timer * t)
{
	ceqs_msg_t msg_queue[16];
	size_t count = 0;
	while ((count = _impl->background_queue.try_dequeue_bulk(msg_queue, 16)) != 0) {
		for (size_t i = 0; i < count; ++i) {
			_ProcessBackgroundMessage(msg_queue[i]);
		}
	}
}

void EQ::Net::ConcurrentEQStreamManager::_BackgroundUpdateStatsTimer(EQ::Timer *t)
{
	ceqs_msg_t msgs[16];
	int i = 0;

	for (auto &c : _impl->connections) {
		auto &connection = c.second;
		auto msg = (ceqs_update_stats_msg_t*)&msgs[i];

		msg->type = ceqs_msg_type::UpdateStats;
		msg->stream_id = connection->GetId();
		msg->stats = connection->GetStats();
		i++;

		if (i >= 16) {
			_impl->foreground_queue.enqueue_bulk(msgs, 16);
			i = 0;
		}
	}

	if (i > 0) {
		_impl->foreground_queue.enqueue_bulk(msgs, i);
	}
}

//Called by background
void EQ::Net::ConcurrentEQStreamManager::_ProcessBackgroundMessage(const ceqs_msg_t &msg)
{
	switch (msg.type) {
	case QueuePacket:
	{
		ceqs_queue_packet_msg_t *msg_in = (ceqs_queue_packet_msg_t*)&msg;

		auto iter = _impl->connections.find(msg_in->stream_id);
		if (iter != _impl->connections.end()) {
			iter->second->QueuePacket(*msg_in->packet, 0, msg_in->ack_req);
		}

		delete msg_in->packet;
		break;
	}
	case TerminateBackground:
	{
		_impl->background_running = false;
		break;
	}
	case CloseConnection:
	{
		ceqs_close_connection_msg_t *msg_in = (ceqs_close_connection_msg_t*)&msg;
		auto iter = _impl->connections.find(msg_in->stream_id);
		if (iter != _impl->connections.end()) {
			iter->second->Close();
		}
		break;
	}
	case ResetStats:
	{
		ceqs_reset_stats_msg_t *msg_in = (ceqs_reset_stats_msg_t*)&msg;
		auto iter = _impl->connections.find(msg_in->stream_id);
		if (iter != _impl->connections.end()) {
			iter->second->ResetStats();
		}
		break;
	}
	default:
		break;
	}
}

//Called by foreground
void EQ::Net::ConcurrentEQStreamManager::_ForegroundTimer(EQ::Timer *t)
{
	ceqs_msg_t msg_queue[16];
	size_t count = 0;
	while ((count = _impl->foreground_queue.try_dequeue_bulk(msg_queue, 16)) != 0) {
		for (size_t i = 0; i < count; ++i) {
			_ProcessForegroundMessage(msg_queue[i]);
		}
	}
}

//Called by foreground
void EQ::Net::ConcurrentEQStreamManager::_ProcessForegroundMessage(const ceqs_msg_t &msg)
{
	switch (msg.type) {
	case NewConnection:
	{
		ceqs_new_connection_msg_t *msg_in = (ceqs_new_connection_msg_t*)&msg;

		std::shared_ptr<ConcurrentEQStream> stream(new ConcurrentEQStream(this, 
			msg_in->stream_id, 
			msg_in->endpoint, 
			msg_in->remote_port,
			(DbProtocolStatus)msg_in->state));

		_impl->streams.insert(std::make_pair(msg_in->stream_id, stream));
		if (_impl->on_new_connection) {
			_impl->on_new_connection(stream);
		}
		break;
	}
	case ConnectionStateChange:
	{
		ceqs_connection_state_change_msg_t *msg_in = (ceqs_connection_state_change_msg_t*)&msg;

		auto iter = _impl->streams.find(msg_in->stream_id);
		if (iter != _impl->streams.end()) {
			iter->second->_SetState((DbProtocolStatus)msg_in->to);

			if ((DbProtocolStatus)msg_in->to == DbProtocolStatus::StatusDisconnected || (DbProtocolStatus)msg_in->to == DbProtocolStatus::StatusDisconnecting) {
				_impl->streams.erase(iter);
			}
		}
		break;
	}
	case PacketRecv:
	{
		ceqs_packet_recv_msg_t *msg_in = (ceqs_packet_recv_msg_t*)&msg;
		std::unique_ptr<EQ::Net::Packet> p(msg_in->packet);

		auto iter = _impl->streams.find(msg_in->stream_id);
		if (iter != _impl->streams.end()) {
			iter->second->_RecvPacket(std::move(p));
		}
		break;
	}
	case UpdateStats:
	{
		ceqs_update_stats_msg_t *msg_in = (ceqs_update_stats_msg_t*)&msg;
		auto iter = _impl->streams.find(msg_in->stream_id);
		if (iter != _impl->streams.end()) {
			iter->second->_UpdateStats(msg_in->stats);
		}
		break;
	}
	default:
		break;
	}
}

void EQ::Net::ConcurrentEQStreamManager::_PushToBackgroundQueue(ceqs_msg_t *msg)
{
	_impl->background_queue.enqueue(*msg);
}

void EQ::Net::ConcurrentEQStreamManager::_PushToForegroundQueue(ceqs_msg_t *msg)
{
	_impl->foreground_queue.enqueue(*msg);
}

//Called by foreground
void EQ::Net::ConcurrentEQStreamManager::OnNewConnection(std::function<void(std::shared_ptr<EQStreamInterface>)> func)
{
	_impl->on_new_connection = func;
}

//Called by foreground
void EQ::Net::ConcurrentEQStreamManager::OnConnectionStateChange(std::function<void(std::shared_ptr<EQStreamInterface>, DbProtocolStatus, DbProtocolStatus)> func)
{
	_impl->on_connection_state_change = func;
}

//Called by background
void EQ::Net::ConcurrentEQStreamManager::DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection)
{
	_impl->connections.insert(std::make_pair(connection->GetId(), connection));
	ceqs_new_connection_msg_t msg;
	msg.type = ceqs_msg_type::NewConnection;
	msg.stream_id = connection->GetId();
	msg.remote_port = connection->RemotePort();
	msg.state = connection->GetStatus();
	strcpy(msg.endpoint, connection->RemoteEndpoint().c_str());
	msg.endpoint[connection->RemoteEndpoint().length()] = 0;

	//Make sure the foreground gets this message
	_PushToForegroundQueue((ceqs_msg_t*)&msg);
}

//Called by background
void EQ::Net::ConcurrentEQStreamManager::DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to)
{
	if (to == DbProtocolStatus::StatusDisconnecting || to == DbProtocolStatus::StatusDisconnected) {
		auto iter = _impl->connections.find(connection->GetId());
		if (iter != _impl->connections.end()) {
			_impl->connections.erase(iter);
		}
	}

	ceqs_connection_state_change_msg_t msg;
	msg.type = ceqs_msg_type::ConnectionStateChange;
	msg.stream_id = connection->GetId();
	msg.from = (int)from;
	msg.to = (int)to;

	//Make sure the foreground gets this message
	_PushToForegroundQueue((ceqs_msg_t*)&msg);
}

//Called by background
void EQ::Net::ConcurrentEQStreamManager::DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p)
{
	ceqs_packet_recv_msg_t msg;
	msg.type = ceqs_msg_type::PacketRecv;
	msg.stream_id = connection->GetId();
	msg.packet = new DynamicPacket();
	msg.packet->PutPacket(0, p);

	//Make sure the foreground gets this message
	_PushToForegroundQueue((ceqs_msg_t*)&msg);
}

struct EQ::Net::ConcurrentEQStream::Impl
{
	ConcurrentEQStreamManager *parent;
	uint64_t id;
	std::string remote_endpoint;
	int remote_port;
	uint32_t remote_ip;
	DbProtocolStatus state;
	std::deque<std::unique_ptr<EQ::Net::Packet>> packet_queue;
	OpcodeManager **opcode_manager;
	DaybreakConnectionStats stats;
	std::unordered_map<EmuOpcode, int> packet_recv_count;
	std::unordered_map<EmuOpcode, int> packet_sent_count;
};

//Called by foreground
EQ::Net::ConcurrentEQStream::ConcurrentEQStream(ConcurrentEQStreamManager *parent, uint64_t id, const std::string &remote_endpoint, int remote_port, DbProtocolStatus state)
{
	_impl.reset(new Impl());
	_impl->parent = parent;
	_impl->id = id;
	_impl->remote_endpoint = remote_endpoint;
	_impl->remote_port = remote_port;
	_impl->remote_ip = inet_addr(remote_endpoint.c_str());
	_impl->state = state;
	_impl->opcode_manager = nullptr;
}

//Called by foreground
EQ::Net::ConcurrentEQStream::~ConcurrentEQStream()
{
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::QueuePacket(const EQApplicationPacket *p, bool ack_req)
{
	if (!_impl->parent) {
		return;
	}

	if (_impl->opcode_manager && *_impl->opcode_manager) {
		auto &options = _impl->parent->GetOptions();
		uint16 opcode = 0;
		if (p->GetOpcodeBypass() != 0) {
			opcode = p->GetOpcodeBypass();
		}
		else {
			_impl->packet_sent_count[p->GetOpcode()]++;
			opcode = (*_impl->opcode_manager)->EmuToEQ(p->GetOpcode());
		}

		EQ::Net::DynamicPacket *out = new EQ::Net::DynamicPacket();
		switch (options.opcode_size) {
		case 1:
			out->PutUInt8(0, opcode);
			out->PutData(1, p->pBuffer, p->size);
			break;
		case 2:
			out->PutUInt16(0, opcode);
			out->PutData(2, p->pBuffer, p->size);
			break;
		}

		ceqs_queue_packet_msg_t msg;
		msg.type = ceqs_msg_type::QueuePacket;
		msg.stream_id = _impl->id;
		msg.packet = out;
		msg.ack_req = ack_req;

		//Make sure the background gets this message
		_impl->parent->_PushToBackgroundQueue((ceqs_msg_t*)&msg);
	}
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::FastQueuePacket(EQApplicationPacket **p, bool ack_req)
{
	QueuePacket(*p, ack_req);
	delete *p;
	*p = nullptr;
}

//Called by foreground
EQApplicationPacket *EQ::Net::ConcurrentEQStream::PopPacket()
{
	if (!_impl->parent) {
		return nullptr;
	}

	if (_impl->packet_queue.empty()) {
		return nullptr;
	}

	if (_impl->opcode_manager != nullptr && *_impl->opcode_manager != nullptr) {
		auto &options = _impl->parent->GetOptions();
		auto &p = _impl->packet_queue.front();

		uint16 opcode = 0;
		switch (options.opcode_size) {
		case 1:
			opcode = p->GetUInt8(0);
			break;
		case 2:
			opcode = p->GetUInt16(0);
			break;
		}

		EmuOpcode emu_op = (*_impl->opcode_manager)->EQToEmu(opcode);
		_impl->packet_recv_count[emu_op]++;

		EQApplicationPacket *ret = new EQApplicationPacket(emu_op, (unsigned char*)p->Data() + options.opcode_size, p->Length() - options.opcode_size);
		ret->SetProtocolOpcode(opcode);
		_impl->packet_queue.pop_front();
		return ret;
	}

	return nullptr;
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::Close()
{
	if (!_impl->parent) {
		return;
	}

	ceqs_close_connection_msg_t msg;
	msg.type = CloseConnection;
	msg.stream_id = _impl->id;

	_impl->parent->_PushToBackgroundQueue((ceqs_msg_t*)&msg);
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::ReleaseFromUse()
{
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::RemoveData()
{
}

//Called by foreground
std::string EQ::Net::ConcurrentEQStream::GetRemoteAddr() const
{
	return _impl->remote_endpoint;
}

//Called by foreground
uint32 EQ::Net::ConcurrentEQStream::GetRemoteIP() const
{
	return _impl->remote_ip;
}

//Called by foreground
uint16 EQ::Net::ConcurrentEQStream::GetRemotePort() const
{
	return _impl->remote_port;
}

//Called by foreground
bool EQ::Net::ConcurrentEQStream::CheckState(EQStreamState state)
{
	return GetState() == state;
}

//Called by foreground
std::string EQ::Net::ConcurrentEQStream::Describe() const
{
	return "Concurrent EQStream";
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::SetActive(bool val)
{
}

//Called by foreground
EQStreamInterface::MatchState EQ::Net::ConcurrentEQStream::CheckSignature(const Signature *sig)
{
	if (!_impl->parent) {
		return MatchFailed;
	}

	if (!_impl->packet_queue.empty()) {
		auto& options = _impl->parent->GetOptions();
		auto p = _impl->packet_queue.front().get();
		uint16 opcode = 0;
		size_t length = p->Length() - options.opcode_size;
		switch (options.opcode_size) {
		case 1:
			opcode = p->GetUInt8(0);
			break;
		case 2:
			opcode = p->GetUInt16(0);
			break;
		}

		if (sig->ignore_eq_opcode != 0 && opcode == sig->ignore_eq_opcode) {
			if (_impl->packet_queue.size() > 1) {
				p = _impl->packet_queue[1].get();
				opcode = 0;
				length = p->Length() - options.opcode_size;
				switch (options.opcode_size) {
				case 1:
					opcode = p->GetUInt8(0);
					break;
				case 2:
					opcode = p->GetUInt16(0);
					break;
				}
			}
			else {
				return MatchNotReady;
			}
		}

		if (opcode == sig->first_eq_opcode) {
			if (length == sig->first_length) {
				LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode matched {2:#x} and length matched {3}",
					GetRemoteAddr(), GetRemotePort(), sig->first_eq_opcode, length);
				return MatchSuccessful;
			}
			else if (length == 0) {
				LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode matched {2:#x} and length is ignored.",
					GetRemoteAddr(), GetRemotePort(), sig->first_eq_opcode);
				return MatchSuccessful;
			}
			else {
				LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode matched {2:#x} but length {3} did not match expected {4}",
					GetRemoteAddr(), GetRemotePort(), sig->first_eq_opcode, length, sig->first_length);
				return MatchFailed;
			}
		}
		else {
			LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode {1:#x} did not match expected {2:#x}",
				GetRemoteAddr(), GetRemotePort(), opcode, sig->first_eq_opcode);
			return MatchFailed;
		}
	}

	return MatchNotReady;
}

//Called by foreground
EQStreamState EQ::Net::ConcurrentEQStream::GetState()
{
	switch (_impl->state) {
	case StatusConnecting:
		return UNESTABLISHED;
	case StatusConnected:
		return ESTABLISHED;
	case StatusDisconnecting:
		return DISCONNECTING;
	default:
		return CLOSED;
	}
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::SetOpcodeManager(OpcodeManager **opm)
{
	_impl->opcode_manager = opm;
}

//Called by foreground
EQStreamInterface::Stats EQ::Net::ConcurrentEQStream::GetStats() const
{
	EQStreamInterface::Stats ret;
	ret.DaybreakStats = _impl->stats;

	for (int i = 0; i < _maxEmuOpcode; ++i) {
		ret.RecvCount[i] = 0;
		ret.SentCount[i] = 0;
	}

	for (auto &s : _impl->packet_sent_count) {
		ret.SentCount[s.first] = s.second;
	}

	for (auto &r : _impl->packet_recv_count) {
		ret.RecvCount[r.first] = r.second;
	}

	return ret;
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::ResetStats()
{
	if (!_impl->parent) {
		return;
	}

	ceqs_reset_stats_msg_t msg;
	msg.type = ceqs_msg_type::ResetStats;
	msg.stream_id = _impl->id;

	_impl->parent->_PushToBackgroundQueue((ceqs_msg_t*)&msg);
}

//Called by foreground
EQStreamManagerInterface *EQ::Net::ConcurrentEQStream::GetManager() const
{
	return _impl->parent;
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::_SetState(DbProtocolStatus state)
{
	_impl->state = state;
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::_RecvPacket(std::unique_ptr<EQ::Net::Packet> p)
{
	_impl->packet_queue.push_back(std::move(p));
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::_UpdateStats(const DaybreakConnectionStats &stats)
{
	_impl->stats = stats;
}

//Called by foreground
void EQ::Net::ConcurrentEQStream::_Invalidate()
{
	_impl->parent = nullptr;
}
