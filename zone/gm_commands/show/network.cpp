#include "../../client.h"
#include "../../dialogue_window.h"

void ShowNetwork(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "#show network get - Retrieve the current option values");
		c->Message(Chat::White, "#show network set [Option] - Set the current option allowed");
		return;
	}

	const bool is_get = !strcasecmp(sep->arg[2], "get");
	const bool is_set = !strcasecmp(sep->arg[2], "set");
	if (!is_get && !is_set) {
		c->Message(Chat::White, "#show network get - Retrieve the current option values");
		c->Message(Chat::White, "#show network set [Option] - Set the current option allowed");
		return;
	}

	auto eqsi    = c->Connection();
	auto manager = eqsi->GetManager();
	auto opts    = manager->GetOptions();

	if (is_get) {
		std::string popup_table;

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Option"),
				DialogueWindow::TableCell("Value")
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Max Packet Size"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.max_packet_size))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Max Connection Count"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.max_connection_count))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Keep Alive Delay"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.keepalive_delay_ms))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Resend Delay Factor"),
				DialogueWindow::TableCell(
					fmt::format(
						"{:.2f}",
						opts.daybreak_options.resend_delay_factor
					)
				)
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Resend Delay"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.resend_delay_ms))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Resend Delay Minimum"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.resend_delay_min))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Resend Delay Maximum"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.resend_delay_max))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Connect Delay"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.connect_delay_ms))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Connect Stale"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.connect_stale_ms))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Stale Connection"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.stale_connection_ms))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("CRC Length"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.crc_length))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Hold Size"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.hold_size))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Hold Length"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.hold_length_ms))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Simulated In Packet Loss"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.simulated_in_packet_loss))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Simulated Out Packet Loss"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.simulated_out_packet_loss))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Tic Rate (Hz)"),
				DialogueWindow::TableCell(
					fmt::format(
						"{:.2f}",
						opts.daybreak_options.tic_rate_hertz
					)
				)
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Resend Timeout"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.resend_timeout))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Connection Close Time"),
				DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.daybreak_options.connection_close_time))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Encode Passes (1)"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.encode_passes[0]))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Encode Passes (2)"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.encode_passes[1]))
			)
		);

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell("Port"),
				DialogueWindow::TableCell(std::to_string(opts.daybreak_options.port))
			)
		);

		popup_table = DialogueWindow::Table(popup_table);

		c->SendPopupToClient(
			"Network Information",
			popup_table.c_str()
		);
	} else if (is_set) {
		const std::string& value = arguments == 3 ? sep->arg[3] : std::string();
		if (value.empty()) {
			c->Message(Chat::White, "Usage: #show network set connect_delay_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set connect_stale_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set connection_close_time [Value]");
			c->Message(Chat::White, "Usage: #show network set hold_length_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set hold_size [Value]");
			c->Message(Chat::White, "Usage: #show network set keepalive_delay_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set max_connection_count [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_factor [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_min [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_max [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_timeout [Value]");;
			c->Message(Chat::White, "Usage: #show network set simulated_in_packet_loss [Value]");
			c->Message(Chat::White, "Usage: #show network set simulated_out_packet_loss [Value]");
			c->Message(Chat::White, "Usage: #show network set stale_connection_ms [Value]");
			return;
		}

		if (!strcasecmp(sep->arg[2], "max_connection_count")) {
			opts.daybreak_options.max_connection_count = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "keepalive_delay_ms")) {
			opts.daybreak_options.keepalive_delay_ms = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "resend_delay_factor")) {
			opts.daybreak_options.resend_delay_factor = std::stod(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "resend_delay_ms")) {
			opts.daybreak_options.resend_delay_ms = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "resend_delay_min")) {
			opts.daybreak_options.resend_delay_min = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "resend_delay_max")) {
			opts.daybreak_options.resend_delay_max = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "connect_delay_ms")) {
			opts.daybreak_options.connect_delay_ms = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "connect_stale_ms")) {
			opts.daybreak_options.connect_stale_ms = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "stale_connection_ms")) {
			opts.daybreak_options.stale_connection_ms = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "hold_size")) {
			opts.daybreak_options.hold_size = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "hold_length_ms")) {
			opts.daybreak_options.hold_length_ms = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "simulated_in_packet_loss")) {
			opts.daybreak_options.simulated_in_packet_loss = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "simulated_out_packet_loss")) {
			opts.daybreak_options.simulated_out_packet_loss = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "resend_timeout")) {
			opts.daybreak_options.resend_timeout = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else if (!strcasecmp(sep->arg[2], "connection_close_time")) {
			opts.daybreak_options.connection_close_time = Strings::ToUnsignedBigInt(value);
			manager->SetOptions(opts);
		} else {
			c->Message(Chat::White, "Usage: #show network set connect_delay_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set connect_stale_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set connection_close_time [Value]");
			c->Message(Chat::White, "Usage: #show network set hold_length_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set hold_size [Value]");
			c->Message(Chat::White, "Usage: #show network set keepalive_delay_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set max_connection_count [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_factor [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_ms [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_min [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_delay_max [Value]");
			c->Message(Chat::White, "Usage: #show network set resend_timeout [Value]");;
			c->Message(Chat::White, "Usage: #show network set simulated_in_packet_loss [Value]");
			c->Message(Chat::White, "Usage: #show network set simulated_out_packet_loss [Value]");
			c->Message(Chat::White, "Usage: #show network set stale_connection_ms [Value]");
		}
	}
}

