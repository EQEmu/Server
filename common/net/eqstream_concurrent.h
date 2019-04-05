#pragma once

#include "../eq_stream_intf.h"
#include "eqstream_concurrent_message.h"
#include <memory>

namespace EQ
{
	class Timer;
	namespace Net
	{
		class ConcurrentEQStream;
		class ConcurrentEQStreamManager : public EQStreamManagerInterface
		{
		public:
			ConcurrentEQStreamManager(const EQStreamManagerInterfaceOptions &options);
			~ConcurrentEQStreamManager();

			virtual void OnNewConnection(std::function<void(std::shared_ptr<EQStreamInterface>)> func);
			virtual void OnConnectionStateChange(std::function<void(std::shared_ptr<EQStreamInterface>, EQ::Net::DbProtocolStatus, EQ::Net::DbProtocolStatus)> func);

			void _PushToBackgroundQueue(ceqs_msg_t* msg);
			void _PushToForegroundQueue(ceqs_msg_t* msg);
		private:
			struct Impl;
			std::unique_ptr<Impl> _impl;
			void _BackgroundThread();
			void _BackgroundTimer(EQ::Timer *t);
			void _BackgroundUpdateStatsTimer(EQ::Timer *t);
			void _ProcessBackgroundMessage(const ceqs_msg_t &msg);
			void _ForegroundTimer(EQ::Timer *t);
			void _ProcessForegroundMessage(const ceqs_msg_t &msg);

			void DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection);
			void DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to);
			void DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p);
		};

		class ConcurrentEQStream : public EQStreamInterface
		{
		public:
			ConcurrentEQStream(ConcurrentEQStreamManager *parent, uint64_t id, const std::string &remote_endpoint, int remote_port, DbProtocolStatus state);
			~ConcurrentEQStream();

			virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req = true);
			virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req = true);
			virtual EQApplicationPacket *PopPacket();
			virtual void Close();
			virtual void ReleaseFromUse();
			virtual void RemoveData();
			virtual std::string GetRemoteAddr() const;
			virtual uint32 GetRemoteIP() const;
			virtual uint16 GetRemotePort() const;
			virtual bool CheckState(EQStreamState state);
			virtual std::string Describe() const;
			virtual void SetActive(bool val);
			virtual MatchState CheckSignature(const Signature *sig);
			virtual EQStreamState GetState();
			virtual void SetOpcodeManager(OpcodeManager **opm);
			virtual Stats GetStats() const;
			virtual void ResetStats();
			virtual EQStreamManagerInterface* GetManager() const;

			void _SetState(DbProtocolStatus state);
			void _RecvPacket(std::unique_ptr<EQ::Net::Packet> p);
			void _UpdateStats(const DaybreakConnectionStats &stats);
			void _Invalidate();
		private:
			struct Impl;

			std::unique_ptr<Impl> _impl;
		};
	}
}
