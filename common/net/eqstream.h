#pragma once

#include "../eq_packet.h"
#include "../eq_stream_intf.h"
#include "../opcodemgr.h"
#include "daybreak_connection.h"
#include <vector>
#include <deque>
#include <unordered_map>

namespace EQ
{
	namespace Net
	{
		class EQStream;
		class EQStreamManager : public EQStreamManagerInterface
		{
		public:
			EQStreamManager(const EQStreamManagerInterfaceOptions &options);
			~EQStreamManager();

			virtual void SetOptions(const EQStreamManagerInterfaceOptions& options);
			void OnNewConnection(std::function<void(std::shared_ptr<EQStream>)> func) { m_on_new_connection = func; }
			void OnConnectionStateChange(std::function<void(std::shared_ptr<EQStream>, DbProtocolStatus, DbProtocolStatus)> func) { m_on_connection_state_change = func; }
		private:
			DaybreakConnectionManager m_daybreak;
			std::function<void(std::shared_ptr<EQStream>)> m_on_new_connection;
			std::function<void(std::shared_ptr<EQStream>, DbProtocolStatus, DbProtocolStatus)> m_on_connection_state_change;
			std::map<std::shared_ptr<DaybreakConnection>, std::shared_ptr<EQStream>> m_streams;

			void DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection);
			void DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to);
			void DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p);
			friend class EQStream;
		};

		class EQStream : public EQStreamInterface
		{
		public:
			EQStream(EQStreamManagerInterface *parent, std::shared_ptr<DaybreakConnection> connection);
			~EQStream();

			virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req = true);
			virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req = true);
			virtual EQApplicationPacket *PopPacket();
			virtual void Close();
			virtual void ReleaseFromUse() { };
			virtual void RemoveData() { };
			virtual std::string GetRemoteAddr() const;
			virtual uint32 GetRemoteIP() const;
			virtual uint16 GetRemotePort() const { return m_connection->RemotePort(); }
			virtual bool CheckState(EQStreamState state);
			virtual std::string Describe() const { return "Direct EQStream"; }
			virtual void SetActive(bool val) { }
			virtual MatchState CheckSignature(const Signature *sig);
			virtual EQStreamState GetState();
			virtual void SetOpcodeManager(OpcodeManager **opm) {
				m_opcode_manager = opm;
			}

			virtual Stats GetStats() const;
			virtual void ResetStats();
			virtual EQStreamManagerInterface* GetManager() const;
		private:
			EQStreamManagerInterface *m_owner;
			std::shared_ptr<DaybreakConnection> m_connection;
			OpcodeManager **m_opcode_manager;
			std::deque<std::unique_ptr<EQ::Net::Packet>> m_packet_queue;
			std::unordered_map<int, int> m_packet_recv_count;
			std::unordered_map<int, int> m_packet_sent_count;
			friend class EQStreamManager;
		};
	}
}
