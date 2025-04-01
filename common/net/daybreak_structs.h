#pragma once

#include <cereal/cereal.hpp>
#include <cstdint>
#include "endian.h"

namespace EQ
{
	namespace Net
	{
		struct DaybreakHeader
		{
			static size_t size() { return 2; }
			uint8_t zero;
			uint8_t opcode;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode);
			}
		};

		struct DaybreakConnect
		{
			static size_t size() { return 14; }
			uint8_t zero;
			uint8_t opcode;
			uint32_t protocol_version;
			uint32_t connect_code;
			uint32_t max_packet_size;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					protocol_version,
					connect_code,
					max_packet_size);
			}
		};

		struct DaybreakConnectReply
		{
			static size_t size() { return 17; }
			uint8_t zero;
			uint8_t opcode;
			uint32_t connect_code;
			uint32_t encode_key;
			uint8_t crc_bytes;
			uint8_t encode_pass1;
			uint8_t encode_pass2;
			uint32_t max_packet_size;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					connect_code,
					encode_key,
					crc_bytes,
					encode_pass1,
					encode_pass2,
					max_packet_size);
			}
		};

		struct DaybreakDisconnect
		{
			static size_t size() { return 8; }
			uint8_t zero;
			uint8_t opcode;
			uint32_t connect_code;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					connect_code);
			}
		};

		struct DaybreakReliableHeader
		{
			static size_t size() { return 4; }
			uint8_t zero;
			uint8_t opcode;
			uint16_t sequence;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					sequence);
			}
		};

		struct DaybreakReliableFragmentHeader
		{
			static size_t size() { return 4 + DaybreakReliableHeader::size(); }
			DaybreakReliableHeader reliable;
			uint32_t total_size;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(reliable,
					total_size);
			}
		};

		struct DaybreakSessionStatRequest
		{
			static size_t size() { return 40; }
			uint8_t zero;
			uint8_t opcode;
			uint16_t timestamp;
			uint32_t stat_ping;
			uint32_t avg_ping;
			uint32_t min_ping;
			uint32_t max_ping;
			uint32_t last_ping;
			uint64_t packets_sent;
			uint64_t packets_recv;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					timestamp,
					stat_ping,
					avg_ping,
					min_ping,
					max_ping,
					last_ping,
					packets_sent,
					packets_recv);
			}
		};

		struct DaybreakSessionStatResponse
		{
			static size_t size() { return 40; }
			uint8_t zero;
			uint8_t opcode;
			uint16_t timestamp;
			uint32_t our_timestamp;
			uint64_t client_sent;
			uint64_t client_recv;
			uint64_t server_sent;
			uint64_t server_recv;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					timestamp,
					our_timestamp,
					client_sent,
					client_recv,
					server_sent,
					server_recv);
			}
		};
	}
}

