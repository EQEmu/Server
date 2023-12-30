#include "../client.h"
#include "../data_bucket.h"
#include "../dialogue_window.h"
#include "../../common/repositories/data_buckets_repository.h"

void command_databuckets(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #databuckets delete [Key] [Character ID] [NPC ID] [Bot ID]");
		c->Message(Chat::White, "Usage: #databuckets view [Partial Key] [Character ID] [NPC ID] [Bot ID]");
		c->Message(Chat::White, "Note: Character ID, NPC ID, and Bot ID are optional if not needed, if needed they are required for specificity");
		return;
	}

	const bool is_delete = !strcasecmp(sep->arg[1], "delete");
	const bool is_view   = !strcasecmp(sep->arg[1], "view");

	if (
		!is_delete &&
		!is_view
	) {
		c->Message(Chat::White, "Usage: #databuckets delete [Key] [Character ID] [NPC ID] [Bot ID]");
		c->Message(Chat::White, "Usage: #databuckets view [Partial Key] [Character ID] [NPC ID] [Bot ID]");
		c->Message(Chat::White, "Note: Character ID, NPC ID, and Bot ID are optional if not needed, if needed they are required for specificity");
		return;
	}

	if (is_delete) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #databuckets delete [Key] [Character ID] [NPC ID] [Bot ID]");
			c->Message(Chat::White, "Usage: #databuckets view [Partial Key] [Character ID] [NPC ID] [Bot ID]");
			c->Message(Chat::White, "Note: Character ID, NPC ID, and Bot ID are optional if not needed, if needed they are required for specificity");
			return;
		}

		const std::string& key_filter = sep->arg[2];

		uint32 character_id = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;
		uint32 npc_id       = sep->IsNumber(4) ? Strings::ToUnsignedInt(sep->arg[4]) : 0;
		uint32 bot_id       = sep->IsNumber(5) ? Strings::ToUnsignedInt(sep->arg[5]) : 0;

		if (
			!character_id &&
			!npc_id &&
			!bot_id
		) {
			if (!DataBucket::DeleteData(key_filter)) {
				c->Message(
					Chat::White,
					fmt::format(
						"An error occurred deleting data bucket '{}'.",
						key_filter
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Data bucket '{}' deleted.",
					key_filter
				).c_str()
			);
		} else {
			DataBucketKey k = {};

			k.key          = key_filter;
			k.character_id = character_id;
			k.npc_id       = npc_id;
			k.bot_id       = bot_id;

			if (!DataBucket::DeleteData(k)) {
				c->Message(
					Chat::White,
					fmt::format(
						"An error occurred deleting data bucket '{}'.",
						key_filter
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Data bucket '{}' deleted.",
					key_filter
				).c_str()
			);
		}
	} else if (is_view) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #databuckets delete [Key] [Character ID] [NPC ID] [Bot ID]");
			c->Message(Chat::White, "Usage: #databuckets view [Partial Key] [Character ID] [NPC ID] [Bot ID]");
			c->Message(Chat::White, "Note: Character ID, NPC ID, and Bot ID are optional if not needed, if needed they are required for specificity");
			return;
		}

		const std::string& key_filter = sep->arg[2];

		uint32 character_id = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;
		uint32 npc_id       = sep->IsNumber(4) ? Strings::ToUnsignedInt(sep->arg[4]) : 0;
		uint32 bot_id       = sep->IsNumber(5) ? Strings::ToUnsignedInt(sep->arg[5]) : 0;

		std::string where_filter = fmt::format(
			"`key` LIKE '%{}%'",
			Strings::Escape(key_filter)
		);

		if (character_id) {
			where_filter += fmt::format(
				" AND `character_id` = {}",
				character_id
			);
		}

		if (npc_id) {
			where_filter += fmt::format(
				" AND `npc_id` = {}",
				npc_id
			);
		}

		if (bot_id) {
			where_filter += fmt::format(
				" AND `bot_id` = {}",
				bot_id
			);
		}

		where_filter += " LIMIT 50";

		const auto& l = DataBucketsRepository::GetWhere(database, where_filter);

		if (l.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"No data buckets found matching '{}'.",
					key_filter
				).c_str()
			);
			return;
		}

		std::string window_text = DialogueWindow::TableRow(
			DialogueWindow::TableCell("ID") +
			DialogueWindow::TableCell("Key") +
			DialogueWindow::TableCell("Value") +
			DialogueWindow::TableCell("Expires") +
			DialogueWindow::TableCell("Character ID") +
			DialogueWindow::TableCell("NPC ID") +
			DialogueWindow::TableCell("Bot ID")
		);

		uint16 bucket_count  = 0;
		uint16 bucket_number = 1;

		for (const auto& e : l) {
			const std::string& expires_string = e.expires == 0 ? "Never" : std::to_string(e.expires);

			window_text += DialogueWindow::TableRow(
				DialogueWindow::TableCell(std::to_string(e.id)) +
				DialogueWindow::TableCell(e.key_) +
				DialogueWindow::TableCell(e.value) +
				DialogueWindow::TableCell(expires_string) +
				DialogueWindow::TableCell(std::to_string(e.character_id)) +
				DialogueWindow::TableCell(std::to_string(e.npc_id)) +
				DialogueWindow::TableCell(std::to_string(e.bot_id))
			);

			const std::string& delete_link = Saylink::Silent(
				fmt::format(
					"#databuckets delete {} {} {} {}",
					e.key_,
					e.character_id,
					e.npc_id,
					e.bot_id
				),
				"Delete"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Bucket {} | Key: {} | Value: {} | Character ID: {} | NPC ID: {} Bot ID: {} | {}",
					bucket_number,
					e.key_,
					e.value,
					e.character_id,
					e.npc_id,
					e.bot_id,
					delete_link
				).c_str()
			);

			bucket_count++;
			bucket_number++;
		}

		window_text = DialogueWindow::Table(window_text);

		c->SendPopupToClient("Data Buckets", window_text.c_str());

		const std::string& response = fmt::format(
			"Found {} data bucket{} matching '{}'{}.",
			bucket_count,
			bucket_count != 1 ? "s" : "",
			key_filter,
			bucket_count == 50 ? ", max reached" : ""
		);

		c->Message(Chat::White, response.c_str());
	}
}
