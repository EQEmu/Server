#pragma once

#include "../eq_stream_intf.h"
#include <memory>

namespace EQ
{
	namespace Net
	{
		class ConcurrentEQStream;
		class ConcurrentEQStreamManager : public EQStreamManagerInterface
		{
		public:
			ConcurrentEQStreamManager(const EQStreamManagerInterfaceOptions &options);
			~ConcurrentEQStreamManager();

			void OnNewConnection(std::function<void(std::shared_ptr<ConcurrentEQStream>)> func);
			void OnConnectionStateChange(std::function<void(std::shared_ptr<ConcurrentEQStream>, DbProtocolStatus, DbProtocolStatus)> func);
		private:
			struct Impl;
			std::unique_ptr<Impl> _impl;

			void DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection);
			void DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to);
			void DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p);
			friend class EQStream;
		};

		class ConcurrentEQStream : public EQStreamInterface
		{
		public:
			ConcurrentEQStream(EQStreamManagerInterface *parent, uint64_t id);
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
		private:
			struct Impl;

			std::unique_ptr<Impl> _impl;
			friend class ConcurrentEQStreamManager;
		};
	}
}
