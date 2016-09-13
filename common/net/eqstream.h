#pragma once

#include "../patch/patch.h"
#include "../eq_packet.h"
#include "daybreak_connection.h"
#include <vector>

namespace EQ
{
	namespace Net
	{
		struct EQStreamManagerOptions
		{
			EQStreamManagerOptions() {
				
			}

			EQStreamManagerOptions(bool encoded, bool compressed) {
				if (encoded) {
					daybreak_options.encode_passes[0] = EncodeXOR;

					if (compressed) {
						daybreak_options.encode_passes[1] = EncodeCompression;
					}
				}
				else {
					if (compressed) {
						daybreak_options.encode_passes[0] = EncodeCompression;
					}
				}
			}

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
			void OnPacketRecv(std::function<void(std::shared_ptr<EQStream>, EmuOpcode, const Packet &)> func) { m_on_packet_recv = func; }

			void RegisterPotentialPatch(EQ::Patches::BasePatch *patch) { m_possible_patches.push_back(std::unique_ptr<EQ::Patches::BasePatch>(patch)); }
		private:
			EQStreamManagerOptions m_options;
			DaybreakConnectionManager m_daybreak;
			std::function<void(std::shared_ptr<EQStream>)> m_on_new_connection;
			std::function<void(std::shared_ptr<EQStream>, DbProtocolStatus, DbProtocolStatus)> m_on_connection_state_change;
			std::function<void(std::shared_ptr<EQStream>, EmuOpcode, const Packet &)> m_on_packet_recv;
			std::map<std::shared_ptr<DaybreakConnection>, std::shared_ptr<EQStream>> m_streams;
			std::vector<std::unique_ptr<EQ::Patches::BasePatch>> m_possible_patches;

			void DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection);
			void DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to);
			void DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p);
			friend class EQStream;
		};

		class EQStream
		{
		public:
			EQStream(EQStreamManager *parent, std::shared_ptr<DaybreakConnection> connection);
			~EQStream();

			const std::string& RemoteEndpoint() const { return m_connection->RemoteEndpoint(); }
			int RemotePort() const { return m_connection->RemotePort(); }

			void QueuePacket(EmuOpcode type, const Packet &p);
			const DaybreakConnectionStats& GetStats() const { return m_connection->GetStats(); }
			void ResetStats();
			size_t GetRollingPing() const { return m_connection->GetRollingPing(); }
			void Close();
			void QueuePacket(const EQApplicationPacket *p);
			void FastQueuePacket(const EQApplicationPacket **p);

			void RegisterPatch(EQ::Patches::BasePatch *p) { m_patch = p; }
			EQ::Patches::BasePatch *GetRegisteredPatch() { return m_patch; }
		private:
			EQStreamManager *m_owner;
			std::shared_ptr<DaybreakConnection> m_connection;
			EQ::Patches::BasePatch *m_patch;
			friend class EQStreamManager;
		};
	}
}
