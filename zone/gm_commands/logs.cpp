#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_logs(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"#logs list - Shows current log settings and categories loaded into the current process' memory for the first 50 log categories"
		);
		c->Message(
			Chat::White,
			"#logs list [Start Category ID] - Shows current log settings and categories loaded into the current process' memory, only shows 50 at a time starting at specified Category ID"
		);
		c->Message(
			Chat::White,
			"#logs reload - Reload all settings in world and all zone processes with what is defined in the database"
		);
		c->Message(
			Chat::White,
			"#logs set [console|file|gmsay] [Category ID] [Debug Level (1-3)] - Sets log settings during the lifetime of the zone"
		);
		return;
	}
	
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_reload = !strcasecmp(sep->arg[1], "reload");
	bool is_set = !strcasecmp(sep->arg[1], "set");

	if (!is_list && !is_reload && !is_set) {
		c->Message(
			Chat::White,
			"#logs list - Shows current log settings and categories loaded into the current process' memory for the first 50 log categories"
		);
		c->Message(
			Chat::White,
			"#logs list [Start Category ID] - Shows current log settings and categories loaded into the current process' memory, only shows 50 at a time starting at specified Category ID"
		);
		c->Message(
			Chat::White,
			"#logs reload - Reload all settings in world and all zone processes with what is defined in the database"
		);
		c->Message(
			Chat::White,
			"#logs set [console|file|gmsay] [Category ID] [Debug Level (1-3)] - Sets log settings during the lifetime of the zone"
		);
		return;
	}

	if (is_list || (is_set && !sep->IsNumber(3))) {
		uint32 start_category_id = 1;
		if (sep->IsNumber(2)) {
			start_category_id = std::stoul(sep->arg[2]);
		}

		uint32 max_category_id = (start_category_id + 49);

		std::string popup_text = "<table>";

		popup_text += "<tr>";
		popup_text += "<td>ID</td>";
		popup_text += "<td>Name</td>";
		popup_text += "<td>Console</td>";
		popup_text += "<td>File</td>";
		popup_text += "<td>GM Say</td>";
		popup_text += "</tr>";

		for (int index = start_category_id; index <= max_category_id; index++) {
			if (index >= Logs::LogCategory::MaxCategoryID) {
				max_category_id = (Logs::LogCategory::MaxCategoryID - 1);
				break;
			}
			
			popup_text += fmt::format(
				"<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>",
				index,
				Logs::LogCategoryName[index],
				LogSys.log_settings[index].log_to_console,
				LogSys.log_settings[index].log_to_file,
				LogSys.log_settings[index].log_to_gmsay
			);
		}

		popup_text += "</table>";

		std::string popup_title = fmt::format(
			"Log Settings [{} to {}]",
			start_category_id,
			max_category_id
		);

		c->SendPopupToClient(
			popup_title.c_str(),
			popup_text.c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Viewing log category settings from {} ({}) to {} ({}).",
				Logs::LogCategoryName[start_category_id],
				start_category_id,
				Logs::LogCategoryName[max_category_id],
				max_category_id
			).c_str()
		);

		int next_category_id = (max_category_id + 1);
		if (next_category_id < Logs::LogCategory::MaxCategoryID) {
			auto next_list_string = fmt::format(
				"#logs list {}",
				next_category_id
			);

			auto next_list_link = EQ::SayLinkEngine::GenerateQuestSaylink(
				next_list_string,
				false,
				next_list_string
			);

			c->Message(
				Chat::White,
				fmt::format(
					"To view the next 50 log settings, you can use {}.",
					next_list_link
				).c_str()
			);
		}
	} else if (is_reload) {
		c->Message(Chat::White, "Attempting to reload Log Settings globally.");
		auto pack = new ServerPacket(ServerOP_ReloadLogs, 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	} else if (is_set && sep->IsNumber(3)) {
		auto logs_set = false;
		bool is_console = !strcasecmp(sep->arg[2], "console");
		bool is_file = !strcasecmp(sep->arg[2], "file");
		bool is_gmsay = !strcasecmp(sep->arg[2], "gmsay");

		if (!sep->IsNumber(4) || (!is_console && !is_file && !is_gmsay)) {
			c->Message(
				Chat::White,
				"#logs set [console|file|gmsay] [Category ID] [Debug Level (1-3)] - Sets log settings during the lifetime of the zone"
			);
			c->Message(Chat::White, "Example: #logs set gmsay 20 1 - Would output Quest errors to gmsay");
			return;
		}

		logs_set = true;

		auto category_id = std::stoul(sep->arg[3]);
		auto setting = std::stoul(sep->arg[4]);

		if (is_console) {
			LogSys.log_settings[category_id].log_to_console = setting;
		} else if (is_file) {
			LogSys.log_settings[category_id].log_to_file = setting;
		} else if (is_gmsay) {
			LogSys.log_settings[category_id].log_to_gmsay = setting;
		}

		if (logs_set) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) is now set to Debug Level {} for {}.",
					Logs::LogCategoryName[category_id],
					category_id,
					setting,
					sep->arg[2]
				).c_str()
			);
		}

		LogSys.log_settings[category_id].is_category_enabled = setting ? 1 : 0;
	}
}

