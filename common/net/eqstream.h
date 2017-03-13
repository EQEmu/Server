#pragma once

#include "../eq_packet.h"
#include "../eq_stream_intf.h"
#include "../opcodemgr.h"
#include "daybreak_connection.h"
#include <vector>
#include <deque>

namespace EQ
{
	namespace Net
	{
		struct EQStreamManagerOptions
		{
			EQStreamManagerOptions() {
				opcode_size = 2;
			}

			EQStreamManagerOptions(int port, bool encoded, bool compressed) {
				opcode_size = 2;

				//World seems to support both compression and xor zone supports one or the others.
				//Enforce one or the other in the convienence construct
				//Login I had trouble getting to recognize compression at all 
				//but that might be because it was still a bit buggy when i was testing that.
				if (compressed) {
					daybreak_options.encode_passes[0] = EncodeCompression;
				}
				else if (encoded) {
					daybreak_options.encode_passes[0] = EncodeXOR;
				}

				daybreak_options.port = port;
			}

			int opcode_size;
			DaybreakConnectionManagerOptions daybreak_options;
		};

		class EQStream;
		class EQStreamManager
		{
		public:
			EQStreamManager(EQStreamManagerOptions &options);
			~EQStreamManager();

			void OnNewConnection(std::function<void(std::shared_ptr<EQStream>)> func) { m_on_new_connection = func; }
			void OnConnectionStateChange(std::function<void(std::shared_ptr<EQStream>, DbProtocolStatus, DbProtocolStatus)> func) { m_on_connection_state_change = func; }
		private:
			EQStreamManagerOptions m_options;
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
			EQStream(EQStreamManager *parent, std::shared_ptr<DaybreakConnection> connection);
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

			const std::string& RemoteEndpoint() const { return m_connection->RemoteEndpoint(); }
			const DaybreakConnectionStats& GetStats() const { return m_connection->GetStats(); }
			void ResetStats() { m_connection->ResetStats(); }
			size_t GetRollingPing() const { return m_connection->GetRollingPing(); }
		private:
			EQStreamManager *m_owner;
			std::shared_ptr<DaybreakConnection> m_connection;
			OpcodeManager **m_opcode_manager;
			std::deque<std::unique_ptr<EQ::Net::Packet>> m_packet_queue;
			friend class EQStreamManager;
		};
	}
}