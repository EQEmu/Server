#include "../client.h"

void command_network(Client *c, const Seperator *sep)
{
	if (!strcasecmp(sep->arg[1], "getopt")) {
		auto eqsi    = c->Connection();
		auto manager = eqsi->GetManager();
		auto opts    = manager->GetOptions();

		if (!strcasecmp(sep->arg[2], "all")) {
			c->Message(Chat::White, "max_packet_size: %llu", (uint64_t) opts.daybreak_options.max_packet_size);
			c->Message(
				Chat::White,
				"max_connection_count: %llu",
				(uint64_t) opts.daybreak_options.max_connection_count
			);
			c->Message(Chat::White, "keepalive_delay_ms: %llu", (uint64_t) opts.daybreak_options.keepalive_delay_ms);
			c->Message(Chat::White, "resend_delay_factor: %.2f", opts.daybreak_options.resend_delay_factor);
			c->Message(Chat::White, "resend_delay_ms: %llu", (uint64_t) opts.daybreak_options.resend_delay_ms);
			c->Message(Chat::White, "resend_delay_min: %llu", (uint64_t) opts.daybreak_options.resend_delay_min);
			c->Message(Chat::White, "resend_delay_max: %llu", (uint64_t) opts.daybreak_options.resend_delay_max);
			c->Message(Chat::White, "connect_delay_ms: %llu", (uint64_t) opts.daybreak_options.connect_delay_ms);
			c->Message(Chat::White, "connect_stale_ms: %llu", (uint64_t) opts.daybreak_options.connect_stale_ms);
			c->Message(Chat::White, "stale_connection_ms: %llu", (uint64_t) opts.daybreak_options.stale_connection_ms);
			c->Message(Chat::White, "crc_length: %llu", (uint64_t) opts.daybreak_options.crc_length);
			c->Message(Chat::White, "hold_size: %llu", (uint64_t) opts.daybreak_options.hold_size);
			c->Message(Chat::White, "hold_length_ms: %llu", (uint64_t) opts.daybreak_options.hold_length_ms);
			c->Message(
				Chat::White,
				"simulated_in_packet_loss: %llu",
				(uint64_t) opts.daybreak_options.simulated_in_packet_loss
			);
			c->Message(
				Chat::White,
				"simulated_out_packet_loss: %llu",
				(uint64_t) opts.daybreak_options.simulated_out_packet_loss
			);
			c->Message(Chat::White, "tic_rate_hertz: %.2f", opts.daybreak_options.tic_rate_hertz);
			c->Message(Chat::White, "resend_timeout: %llu", (uint64_t) opts.daybreak_options.resend_timeout);
			c->Message(
				Chat::White,
				"connection_close_time: %llu",
				(uint64_t) opts.daybreak_options.connection_close_time
			);
			c->Message(Chat::White, "encode_passes[0]: %llu", (uint64_t) opts.daybreak_options.encode_passes[0]);
			c->Message(Chat::White, "encode_passes[1]: %llu", (uint64_t) opts.daybreak_options.encode_passes[1]);
			c->Message(Chat::White, "port: %llu", (uint64_t) opts.daybreak_options.port);
		}
		else {
			c->Message(Chat::White, "Unknown get option: %s", sep->arg[2]);
			c->Message(Chat::White, "Available options:");
			//Todo the rest of these when im less lazy.
			//c->Message(Chat::White, "max_packet_size");
			//c->Message(Chat::White, "max_connection_count");
			//c->Message(Chat::White, "keepalive_delay_ms");
			//c->Message(Chat::White, "resend_delay_factor");
			//c->Message(Chat::White, "resend_delay_ms");
			//c->Message(Chat::White, "resend_delay_min");
			//c->Message(Chat::White, "resend_delay_max");
			//c->Message(Chat::White, "connect_delay_ms");
			//c->Message(Chat::White, "connect_stale_ms");
			//c->Message(Chat::White, "stale_connection_ms");
			//c->Message(Chat::White, "crc_length");
			//c->Message(Chat::White, "hold_size");
			//c->Message(Chat::White, "hold_length_ms");
			//c->Message(Chat::White, "simulated_in_packet_loss");
			//c->Message(Chat::White, "simulated_out_packet_loss");
			//c->Message(Chat::White, "tic_rate_hertz");
			//c->Message(Chat::White, "resend_timeout");
			//c->Message(Chat::White, "connection_close_time");
			//c->Message(Chat::White, "encode_passes[0]");
			//c->Message(Chat::White, "encode_passes[1]");
			//c->Message(Chat::White, "port");
			c->Message(Chat::White, "all");
		}
	}
	else if (!strcasecmp(sep->arg[1], "setopt")) {
		auto eqsi    = c->Connection();
		auto manager = eqsi->GetManager();
		auto opts    = manager->GetOptions();

		if (!strcasecmp(sep->arg[3], "")) {
			c->Message(Chat::White, "Missing value for set");
			return;
		}

		std::string value = sep->arg[3];
		if (!strcasecmp(sep->arg[2], "max_connection_count")) {
			opts.daybreak_options.max_connection_count = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "keepalive_delay_ms")) {
			opts.daybreak_options.keepalive_delay_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_factor")) {
			opts.daybreak_options.resend_delay_factor = std::stod(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_ms")) {
			opts.daybreak_options.resend_delay_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_min")) {
			opts.daybreak_options.resend_delay_min = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_max")) {
			opts.daybreak_options.resend_delay_max = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "connect_delay_ms")) {
			opts.daybreak_options.connect_delay_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "connect_stale_ms")) {
			opts.daybreak_options.connect_stale_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "stale_connection_ms")) {
			opts.daybreak_options.stale_connection_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "hold_size")) {
			opts.daybreak_options.hold_size = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "hold_length_ms")) {
			opts.daybreak_options.hold_length_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "simulated_in_packet_loss")) {
			opts.daybreak_options.simulated_in_packet_loss = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "simulated_out_packet_loss")) {
			opts.daybreak_options.simulated_out_packet_loss = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_timeout")) {
			opts.daybreak_options.resend_timeout = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "connection_close_time")) {
			opts.daybreak_options.connection_close_time = std::stoull(value);
			manager->SetOptions(opts);
		}
		else {
			c->Message(Chat::White, "Unknown set option: %s", sep->arg[2]);
			c->Message(Chat::White, "Available options:");
			c->Message(Chat::White, "max_connection_count");
			c->Message(Chat::White, "keepalive_delay_ms");
			c->Message(Chat::White, "resend_delay_factor");
			c->Message(Chat::White, "resend_delay_ms");
			c->Message(Chat::White, "resend_delay_min");
			c->Message(Chat::White, "resend_delay_max");
			c->Message(Chat::White, "connect_delay_ms");
			c->Message(Chat::White, "connect_stale_ms");
			c->Message(Chat::White, "stale_connection_ms");
			c->Message(Chat::White, "hold_size");
			c->Message(Chat::White, "hold_length_ms");
			c->Message(Chat::White, "simulated_in_packet_loss");
			c->Message(Chat::White, "simulated_out_packet_loss");
			c->Message(Chat::White, "resend_timeout");
			c->Message(Chat::White, "connection_close_time");
		}
	}
	else {
		c->Message(Chat::White, "Unknown command: %s", sep->arg[1]);
		c->Message(Chat::White, "Network commands avail:");
		c->Message(Chat::White, "getopt optname - Retrieve the current option value set.");
		c->Message(Chat::White, "setopt optname - Set the current option allowed.");
	}
}

