#include "../../client.h"
#include "../../dialogue_window.h"

void ShowNetworkStats(Client *c, const Seperator *sep)
{
	const bool is_full  = sep->arg[2] ? !strcasecmp(sep->arg[2], "full") : false;
	const bool is_reset = sep->arg[2] ? !strcasecmp(sep->arg[2], "reset") : false;

	if (is_reset) {
		auto connection = c->Connection();
		c->Message(Chat::White, "Resetting client stats (packet loss will not read correctly after reset).");
		connection->ResetStats();
		return;
	}

	const auto connection = c->Connection();
	const auto opts       = connection->GetManager()->GetOptions();
	const auto eqs_stats  = connection->GetStats();

	const auto& stats = eqs_stats.DaybreakStats;

	const auto sec_since_stats_reset = std::chrono::duration_cast<std::chrono::duration<double>>(
		EQ::Net::Clock::now() - stats.created
	).count();

	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Sent Bytes"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.sent_bytes,
					stats.sent_bytes / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Received Bytes"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.recv_bytes,
					stats.recv_bytes / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}{}{}",
			DialogueWindow::TableCell("Bytes Before Encode (Sent)"),
			DialogueWindow::TableCell(std::to_string(stats.bytes_before_encode)),
			DialogueWindow::TableCell("Compression Rate"),
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f}%%",
					static_cast<double>(stats.bytes_before_encode - stats.sent_bytes) /
					static_cast<double>(stats.bytes_before_encode) * 100.0
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}{}{}",
			DialogueWindow::TableCell("Bytes After Encode (Sent)"),
			DialogueWindow::TableCell(std::to_string(stats.bytes_after_decode)),
			DialogueWindow::TableCell("Compression Rate"),
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f}%%",
					static_cast<double>(stats.bytes_after_decode - stats.recv_bytes) /
					static_cast<double>(stats.bytes_after_decode) * 100.0
				)
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Minimum Ping"),
			DialogueWindow::TableCell(std::to_string(stats.min_ping))
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Maximum Ping"),
			DialogueWindow::TableCell(std::to_string(stats.max_ping))
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Last Ping"),
			DialogueWindow::TableCell(std::to_string(stats.last_ping))
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Average Ping"),
			DialogueWindow::TableCell(std::to_string(stats.avg_ping))
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("(Realtime) Received Packets"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.recv_packets,
					stats.recv_packets / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("(Realtime) Sent Packets"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.sent_packets,
					stats.sent_packets / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("(Sync) Received Packets"),
			DialogueWindow::TableCell(std::to_string(stats.sync_recv_packets))
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("(Sync) Sent Packets"),
			DialogueWindow::TableCell(std::to_string(stats.sync_sent_packets))
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("(Sync) Remote Received Packets"),
			DialogueWindow::TableCell(std::to_string(stats.sync_remote_recv_packets))
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("(Sync) Remote Sent Packets"),
			DialogueWindow::TableCell(std::to_string(stats.sync_remote_sent_packets))
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Packet Loss In"),
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f}%%",
					(
						100.0 *
						(
							1.0 -
							static_cast<double>(stats.sync_recv_packets) /
							static_cast<double>(stats.sync_remote_sent_packets)
						)
					)
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Packet Loss Out"),
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f}%%",
					(
						100.0 *
						(
							1.0 -
							static_cast<double>(stats.sync_remote_recv_packets) /
							static_cast<double>(stats.sync_sent_packets)
						)
					)
				)
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Resent Packets"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.resent_packets,
					stats.resent_packets / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Resent Fragments"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.resent_fragments,
					stats.resent_fragments / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Resent Non-Fragments"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.resent_full,
					stats.resent_full / sec_since_stats_reset
				)
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}",
			DialogueWindow::TableCell("Dropped Datarate Packets"),
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f} Per Second)",
					stats.dropped_datarate_packets,
					stats.dropped_datarate_packets / sec_since_stats_reset
				)
			)
		)
	);

	if (opts.daybreak_options.outgoing_data_rate > 0.0) {
		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Outgoing Link Saturation"),
				DialogueWindow::TableCell(
					fmt::format(
						"{:.2f}%% ({:.2f}kb Per Second)",
						(
							100.0 *
							(
								1.0 -
								(
									(
										opts.daybreak_options.outgoing_data_rate -
										stats.datarate_remaining
									) /
									opts.daybreak_options.outgoing_data_rate
								)
							)
						),
						opts.daybreak_options.outgoing_data_rate
					)
				)
			)
		);
	}

	if (is_full) {
		popup_table += DialogueWindow::Break(2);

		std::string sent_rows;

		for (int i = 0; i < _maxEmuOpcode; ++i) {
			const int count = eqs_stats.SentCount[i];
			if (count) {
				sent_rows += DialogueWindow::TableRow(
					fmt::format(
						"{}{}",
						DialogueWindow::TableCell(OpcodeNames[i]),
						DialogueWindow::TableCell(
							fmt::format(
								"{} ({:.2f} Per Second)",
								count,
								count / sec_since_stats_reset
							)
						)
					)
				);
			}
		}

		std::string recv_rows;

		for (int i = 0; i < _maxEmuOpcode; ++i) {
			const int count = eqs_stats.RecvCount[i];
			if (count) {
				recv_rows += DialogueWindow::TableRow(
					fmt::format(
						"{}{}",
						DialogueWindow::TableCell(OpcodeNames[i]),
						DialogueWindow::TableCell(
							fmt::format(
								"{} ({:.2f} Per Second)",
								count,
								count / sec_since_stats_reset
							)
						)
					)
				);
			}
		}

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Sent Packet Types"),
				sent_rows
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Received Packet Types"),
				recv_rows
			)
		);
	}

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		"Network Statistics",
		popup_table.c_str()
	);
}

