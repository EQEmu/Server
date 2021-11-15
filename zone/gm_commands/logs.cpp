#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_logs(Client *c, const Seperator *sep)
{
	int logs_set = 0;
	if (sep->argnum > 0) {
		/* #logs reload_all */
		if (strcasecmp(sep->arg[1], "reload_all") == 0) {
			auto pack = new ServerPacket(ServerOP_ReloadLogs, 0);
			worldserver.SendPacket(pack);
			c->Message(
				Chat::Red,
				"Successfully sent the packet to world to reload log settings from the database for all zones"
			);
			safe_delete(pack);
		}
		/* #logs list_settings */
		if (strcasecmp(sep->arg[1], "list_settings") == 0 ||
			(strcasecmp(sep->arg[1], "set") == 0 && strcasecmp(sep->arg[3], "") == 0)) {
			c->Message(Chat::White, "[Category ID | console | file | gmsay | Category Description]");
			int      redisplay_columns = 0;
			for (int i                 = 0; i < Logs::LogCategory::MaxCategoryID; i++) {
				if (redisplay_columns == 10) {
					c->Message(Chat::White, "[Category ID | console | file | gmsay | Category Description]");
					redisplay_columns = 0;
				}
				c->Message(
					0,
					StringFormat(
						"--- %i | %u | %u | %u | %s",
						i,
						LogSys.log_settings[i].log_to_console,
						LogSys.log_settings[i].log_to_file,
						LogSys.log_settings[i].log_to_gmsay,
						Logs::LogCategoryName[i]
					).c_str());
				redisplay_columns++;
			}
		}
		/* #logs set */
		if (strcasecmp(sep->arg[1], "set") == 0) {
			if (strcasecmp(sep->arg[2], "console") == 0) {
				LogSys.log_settings[atoi(sep->arg[3])].log_to_console = atoi(sep->arg[4]);
				logs_set = 1;
			}
			else if (strcasecmp(sep->arg[2], "file") == 0) {
				LogSys.log_settings[atoi(sep->arg[3])].log_to_file = atoi(sep->arg[4]);
				logs_set = 1;
			}
			else if (strcasecmp(sep->arg[2], "gmsay") == 0) {
				LogSys.log_settings[atoi(sep->arg[3])].log_to_gmsay = atoi(sep->arg[4]);
				logs_set = 1;
			}
			else {
				c->Message(
					Chat::White,
					"--- #logs set [console|file|gmsay] <category_id> <debug_level (1-3)> - Sets log settings during the lifetime of the zone"
				);
				c->Message(Chat::White, "--- #logs set gmsay 20 1 - Would output Quest errors to gmsay");
			}
			if (logs_set == 1) {
				c->Message(Chat::Yellow, "Your Log Settings have been applied");
				c->Message(
					Chat::Yellow,
					"Output Method: %s :: Debug Level: %i - Category: %s",
					sep->arg[2],
					atoi(sep->arg[4]),
					Logs::LogCategoryName[atoi(sep->arg[3])]
				);
			}
			/* We use a general 'is_category_enabled' now, let's update when we update any output settings
				This is used in hot places of code to check if its enabled in any way before triggering logs
			*/
			if (atoi(sep->arg[4]) > 0) {
				LogSys.log_settings[atoi(sep->arg[3])].is_category_enabled = 1;
			}
			else {
				LogSys.log_settings[atoi(sep->arg[3])].is_category_enabled = 0;
			}
		}
	}
	else {
		c->Message(Chat::White, "#logs usage:");
		c->Message(
			Chat::White,
			"--- #logs reload_all - Reload all settings in world and all zone processes with what is defined in the database"
		);
		c->Message(
			Chat::White,
			"--- #logs list_settings - Shows current log settings and categories loaded into the current process' memory"
		);
		c->Message(
			Chat::White,
			"--- #logs set [console|file|gmsay] <category_id> <debug_level (1-3)> - Sets log settings during the lifetime of the zone"
		);
	}
}

