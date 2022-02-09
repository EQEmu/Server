#include "../client.h"

void command_netstats(Client *c, const Seperator *sep)
{
	if (c) {
		auto client = c;
		if (c->GetTarget() && c->GetTarget()->IsClient()) {
			client = c->GetTarget()->CastToClient();
		}

		if (strcasecmp(sep->arg[1], "reset") == 0) {
			auto connection = c->Connection();
			c->Message(Chat::White, "Resetting client stats (packet loss will not read correctly after reset).");
			connection->ResetStats();
			return;
		}

		auto connection            = c->Connection();
		auto opts                  = connection->GetManager()->GetOptions();
		auto eqs_stats             = connection->GetStats();
		auto &stats                = eqs_stats.DaybreakStats;
		auto now                   = EQ::Net::Clock::now();
		auto sec_since_stats_reset = std::chrono::duration_cast<std::chrono::duration<double>>(
			now - stats.created
		).count();

		c->Message(Chat::White, "Netstats:");
		c->Message(Chat::White, "--------------------------------------------------------------------");
		c->Message(
			Chat::White,
			"Sent Bytes: %u (%.2f/sec)",
			stats.sent_bytes,
			stats.sent_bytes / sec_since_stats_reset
		);
		c->Message(
			Chat::White,
			"Recv Bytes: %u (%.2f/sec)",
			stats.recv_bytes,
			stats.recv_bytes / sec_since_stats_reset
		);
		c->Message(
			Chat::White, "Bytes Before Encode (Sent): %u, Compression Rate: %.2f%%", stats.bytes_before_encode,
			static_cast<double>(stats.bytes_before_encode - stats.sent_bytes) /
			static_cast<double>(stats.bytes_before_encode) * 100.0
		);
		c->Message(
			Chat::White, "Bytes After Decode (Recv): %u, Compression Rate: %.2f%%", stats.bytes_after_decode,
			static_cast<double>(stats.bytes_after_decode - stats.recv_bytes) /
			static_cast<double>(stats.bytes_after_decode) * 100.0
		);
		c->Message(Chat::White, "Min Ping: %u", stats.min_ping);
		c->Message(Chat::White, "Max Ping: %u", stats.max_ping);
		c->Message(Chat::White, "Last Ping: %u", stats.last_ping);
		c->Message(Chat::White, "Average Ping: %u", stats.avg_ping);
		c->Message(Chat::White, "--------------------------------------------------------------------");
		c->Message(
			Chat::White,
			"(Realtime) Recv Packets: %u (%.2f/sec)",
			stats.recv_packets,
			stats.recv_packets / sec_since_stats_reset
		);
		c->Message(
			Chat::White,
			"(Realtime) Sent Packets: %u (%.2f/sec)",
			stats.sent_packets,
			stats.sent_packets / sec_since_stats_reset
		);
		c->Message(Chat::White, "(Sync) Recv Packets: %u", stats.sync_recv_packets);
		c->Message(Chat::White, "(Sync) Sent Packets: %u", stats.sync_sent_packets);
		c->Message(Chat::White, "(Sync) Remote Recv Packets: %u", stats.sync_remote_recv_packets);
		c->Message(Chat::White, "(Sync) Remote Sent Packets: %u", stats.sync_remote_sent_packets);
		c->Message(
			Chat::White,
			"Packet Loss In: %.2f%%",
			100.0 * (1.0 - static_cast<double>(stats.sync_recv_packets) /
						   static_cast<double>(stats.sync_remote_sent_packets)));
		c->Message(
			Chat::White,
			"Packet Loss Out: %.2f%%",
			100.0 * (1.0 - static_cast<double>(stats.sync_remote_recv_packets) /
						   static_cast<double>(stats.sync_sent_packets)));
		c->Message(Chat::White, "--------------------------------------------------------------------");
		c->Message(
			Chat::White,
			"Resent Packets: %u (%.2f/sec)",
			stats.resent_packets,
			stats.resent_packets / sec_since_stats_reset
		);
		c->Message(
			Chat::White,
			"Resent Fragments: %u (%.2f/sec)",
			stats.resent_fragments,
			stats.resent_fragments / sec_since_stats_reset
		);
		c->Message(
			Chat::White,
			"Resent Non-Fragments: %u (%.2f/sec)",
			stats.resent_full,
			stats.resent_full / sec_since_stats_reset
		);
		c->Message(
			Chat::White,
			"Dropped Datarate Packets: %u (%.2f/sec)",
			stats.dropped_datarate_packets,
			stats.dropped_datarate_packets / sec_since_stats_reset
		);

		if (opts.daybreak_options.outgoing_data_rate > 0.0) {
			c->Message(
				Chat::White,
				"Outgoing Link Saturation %.2f%% (%.2fkb/sec)",
				100.0 * (1.0 - ((opts.daybreak_options.outgoing_data_rate - stats.datarate_remaining) /
								opts.daybreak_options.outgoing_data_rate)),
				opts.daybreak_options.outgoing_data_rate
			);
		}

		if (strcasecmp(sep->arg[1], "full") == 0) {
			c->Message(Chat::White, "--------------------------------------------------------------------");
			c->Message(Chat::White, "Sent Packet Types");
			for (auto i = 0; i < _maxEmuOpcode; ++i) {
				auto cnt = eqs_stats.SentCount[i];
				if (cnt > 0) {
					c->Message(Chat::White, "%s: %u (%.2f / sec)", OpcodeNames[i], cnt, cnt / sec_since_stats_reset);
				}
			}

			c->Message(Chat::White, "--------------------------------------------------------------------");
			c->Message(Chat::White, "Recv Packet Types");
			for (auto i = 0; i < _maxEmuOpcode; ++i) {
				auto cnt = eqs_stats.RecvCount[i];
				if (cnt > 0) {
					c->Message(Chat::White, "%s: %u (%.2f / sec)", OpcodeNames[i], cnt, cnt / sec_since_stats_reset);
				}
			}
		}

		c->Message(Chat::White, "--------------------------------------------------------------------");
	}
}

