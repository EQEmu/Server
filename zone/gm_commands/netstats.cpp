#include "../client.h"

void command_netstats(Client *c, const Seperator *sep)
{
	bool is_full = !strcasecmp(sep->arg[1], "full");
	bool is_reset = !strcasecmp(sep->arg[1], "reset");

	if (is_reset) {
		auto connection = c->Connection();
		c->Message(Chat::White, "Resetting client stats (packet loss will not read correctly after reset).");
		connection->ResetStats();
		return;
	}

	auto connection = c->Connection();
	auto opts = connection->GetManager()->GetOptions();
	auto eqs_stats = connection->GetStats();
	auto &stats = eqs_stats.DaybreakStats;
	auto now = EQ::Net::Clock::now();
	auto sec_since_stats_reset = std::chrono::duration_cast<std::chrono::duration<double>>(
		now - stats.created
	).count();

	std::string popup_text = "<table>";
	
	popup_text += fmt::format(
		"<tr><td>Sent Bytes</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.sent_bytes,
		stats.sent_bytes / sec_since_stats_reset
	);

	popup_text += fmt::format(
		"<tr><td>Received Bytes</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.recv_bytes,
		stats.recv_bytes / sec_since_stats_reset
	);

	popup_text += "<br><br>";

	popup_text += fmt::format(
		"<tr><td>Bytes Before Encode (Sent)</td><td>{}</td><td>Compression Rate</td><td>{:.2f}%%</td></tr>",
		stats.bytes_before_encode,
		static_cast<double>(stats.bytes_before_encode - stats.sent_bytes) /
		static_cast<double>(stats.bytes_before_encode) * 100.0
	);

	popup_text += fmt::format(
		"<tr><td>Bytes After Decode (Received)</td><td>{}</td><td>Compression Rate</td><td>{:.2f}%%</td></tr>",
		stats.bytes_after_decode,
		static_cast<double>(stats.bytes_after_decode - stats.recv_bytes) /
		static_cast<double>(stats.bytes_after_decode) * 100.0
	);
	
	popup_text += "<br><br>";

	popup_text += fmt::format("<tr><td>Min Ping</td><td>{}</td></tr>", stats.min_ping);
	popup_text += fmt::format("<tr><td>Max Ping</td><td>{}</td></tr>", stats.max_ping);
	popup_text += fmt::format("<tr><td>Last Ping</td><td>{}</td></tr>", stats.last_ping);
	popup_text += fmt::format("<tr><td>Average Ping</td><td>{}</td></tr>", stats.avg_ping);
	
	popup_text += "<br><br>";

	popup_text += fmt::format(
		"<tr><td>(Realtime) Received Packets</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.recv_packets,
		stats.recv_packets / sec_since_stats_reset
	);

	popup_text += fmt::format(
		"<tr><td>(Realtime) Sent Packets</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.sent_packets,
		stats.sent_packets / sec_since_stats_reset
	);

	popup_text += "<br><br>";

	popup_text += fmt::format("<tr><td>(Sync) Received Packets</td><td>{}</td></tr>", stats.sync_recv_packets);
	popup_text += fmt::format("<tr><td>(Sync) Sent Packets</td><td>{}</td></tr>", stats.sync_sent_packets);
	popup_text += fmt::format("<tr><td>(Sync) Remote Received Packets</td><td>{}</td></tr>", stats.sync_remote_recv_packets);
	popup_text += fmt::format("<tr><td>(Sync) Remote Sent Packets</td><td>{}</td></tr>", stats.sync_remote_sent_packets);

	popup_text += "<br><br>";

	popup_text += fmt::format(
		"<tr><td>Packet Loss In</td><td>{:.2f}%%</td></tr>",
		(100.0 * (1.0 - static_cast<double>(stats.sync_recv_packets) / static_cast<double>(stats.sync_remote_sent_packets)))
	);

	popup_text += fmt::format(
		"<tr><td>Packet Loss Out</td><td>{:.2f}%%</td></tr>",
		(100.0 * (1.0 - static_cast<double>(stats.sync_remote_recv_packets) / static_cast<double>(stats.sync_sent_packets)))
	);

	popup_text += "<br><br>";

	popup_text += fmt::format(
		"<tr><td>Resent Packets</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.resent_packets,
		stats.resent_packets / sec_since_stats_reset
	);

	popup_text += fmt::format(
		"<tr><td>Resent Fragments</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.resent_fragments,
		stats.resent_fragments / sec_since_stats_reset
	);

	popup_text += fmt::format(
		"<tr><td>Resent Non-Fragments</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.resent_full,
		stats.resent_full / sec_since_stats_reset
	);

	popup_text += "<br><br>";

	popup_text += fmt::format(
		"<tr><td>Dropped Datarate Packets</td><td>{} ({:.2f} Per Second)</td></tr>",
		stats.dropped_datarate_packets,
		stats.dropped_datarate_packets / sec_since_stats_reset
	);

	if (opts.daybreak_options.outgoing_data_rate > 0.0) {
		popup_text += fmt::format(
			"<tr><td>Outgoing Link Saturation</td><td>{:.2f}%% ({:.2f}kb Per Second)</td></tr>",
			(100.0 * (1.0 - ((opts.daybreak_options.outgoing_data_rate - stats.datarate_remaining) / opts.daybreak_options.outgoing_data_rate))),
			opts.daybreak_options.outgoing_data_rate
		);
	}
	
	if (is_full) {
		popup_text += "<br><br>";

		popup_text += "<tr><td>Sent Packet Types</td></tr>";
		for (auto i = 0; i < _maxEmuOpcode; ++i) {
			auto cnt = eqs_stats.SentCount[i];
			if (cnt > 0) {
				popup_text += fmt::format(
					"<tr><td>{}</td><td>{} ({:.2f} Per Second)</td></tr>",
					OpcodeNames[i],
					cnt,
					cnt / sec_since_stats_reset
				);
			}
		}

		popup_text += "<br><br>";

		popup_text += "<tr><td>Received Packet Types</td></tr>";
		for (auto i = 0; i < _maxEmuOpcode; ++i) {
			auto cnt = eqs_stats.RecvCount[i];
			if (cnt > 0) {
				popup_text += fmt::format(
					"<tr><td>{}</td><td>{} ({:.2f} Per Second)</td></tr>",
					OpcodeNames[i],
					cnt,
					cnt / sec_since_stats_reset
				);
			}
		}
	}

	popup_text += "</table>";

	c->SendPopupToClient(
		"Network Statistics",
		popup_text.c_str()
	);
}

