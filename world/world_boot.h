#ifndef EQEMU_WORLD_BOOT_H
#define EQEMU_WORLD_BOOT_H

#include <string>
#include "../common/types.h"
#include "../common/discord/discord.h"

extern UCSConnection UCSLink;

class WorldBoot {
public:
	static void GMSayHookCallBackProcessWorld(uint16 log_category, const char *func, std::string message);
	static bool HandleCommandInput(int argc, char **argv);
	static bool LoadServerConfig();
	static bool LoadDatabaseConnections();
	static void CheckForServerScript(bool force_download = false);
	static void CheckForXMLConfigUpgrade();
	static void RegisterLoginservers();
	static bool DatabaseLoadRoutines(int argc, char **argv);
	static void CheckForPossibleConfigurationIssues();
	static void Shutdown();
	static void SendDiscordMessage(int webhook_id, const std::string& message);
	static void DiscordWebhookMessageHandler(uint16 log_category, int webhook_id, const std::string &message)
	{
		std::string message_prefix = fmt::format(
			"[**{}**] **World** ",
			Logs::LogCategoryName[log_category]
		);

		SendDiscordMessage(webhook_id, message_prefix + Discord::FormatDiscordMessage(log_category, message));
	};
};


#endif //EQEMU_WORLD_BOOT_H
