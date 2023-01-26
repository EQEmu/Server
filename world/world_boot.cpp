#include "../common/content/world_content_service.h"
#include "../common/emu_constants.h"
#include "../common/eqemu_logsys.h"
#include "../common/http/httplib.h"
#include "../common/http/uri.h"
#include "../common/net/console_server.h"
#include "../common/net/servertalk_server.h"
#include "../common/repositories/character_task_timers_repository.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "adventure_manager.h"
#include "dynamic_zone_manager.h"
#include "expedition_database.h"
#include "login_server_list.h"
#include "shared_task_manager.h"
#include "ucs.h"
#include "wguild_mgr.h"
#include "world_boot.h"
#include "world_config.h"
#include "world_event_scheduler.h"
#include "world_server_cli.h"
#include "../common/zone_store.h"
#include "worlddb.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/ip_util.h"
#include "../common/zone_store.h"
#include "../common/path_manager.h"

extern ZSList      zoneserver_list;
extern WorldConfig Config;

void WorldBoot::GMSayHookCallBackProcessWorld(uint16 log_category, const char *func, std::string message)
{
	// we don't want to loop up with chat messages
	if (message.find("OP_SpecialMesg") != std::string::npos) {
		return;
	}

	// Cut messages down to 4000 max to prevent client crash
	if (!message.empty()) {
		message = message.substr(0, 4000);
	}

	// Replace Occurrences of % or MessageStatus will crash
	Strings::FindReplace(message, std::string("%"), std::string("."));

	if (message.find('\n') != std::string::npos) {
		auto message_split = Strings::Split(message, '\n');

		for (size_t iter = 0; iter < message_split.size(); ++iter) {
			zoneserver_list.SendEmoteMessage(
				0,
				0,
				AccountStatus::QuestTroupe,
				LogSys.GetGMSayColorFromCategory(log_category),
				fmt::format(
					" {}{}",
					(iter == 0 ? " ---" : ""),
					message_split[iter]
				).c_str()
			);
		}

		return;
	}

	zoneserver_list.SendEmoteMessage(
		0,
		0,
		AccountStatus::QuestTroupe,
		LogSys.GetGMSayColorFromCategory(log_category),
		"%s",
		fmt::format("[{}] [{}] {}", Logs::LogCategoryName[log_category], func, message).c_str()
	);
}

bool WorldBoot::HandleCommandInput(int argc, char **argv)
{
	// command handler
	if (argc > 1) {
		LogSys.SilenceConsoleLogging();
		path.LoadPaths();
		WorldConfig::LoadConfig();
		LoadDatabaseConnections();
		RuleManager::Instance()->LoadRules(&database, "default", false);
		LogSys.EnableConsoleLogging();
		WorldserverCLI::CommandHandler(argc, argv);
	}

	// database version
	uint32 database_version      = CURRENT_BINARY_DATABASE_VERSION;
	uint32 bots_database_version = RuleB(Bots, Enabled) ? CURRENT_BINARY_BOTS_DATABASE_VERSION : 0;
	if (argc >= 2) {
		if (strcasecmp(argv[1], "db_version") == 0) {
			std::cout << "Binary Database Version: " << database_version << " : " << bots_database_version << std::endl;
			return true;
		}
	}

	return false;
}

bool WorldBoot::LoadServerConfig()
{
	LogInfo("Loading server configuration");
	if (!WorldConfig::LoadConfig()) {
		LogError("Loading server configuration failed");
		return false;
	}

	return true;
}


bool WorldBoot::LoadDatabaseConnections()
{
	const auto c = EQEmuConfig::get();

	LogInfo(
		"Connecting to MySQL [{}]@[{}]:[{}]",
		c->DatabaseUsername.c_str(),
		c->DatabaseHost.c_str(),
		c->DatabasePort
	);

	if (!database.Connect(
		c->DatabaseHost.c_str(),
		c->DatabaseUsername.c_str(),
		c->DatabasePassword.c_str(),
		c->DatabaseDB.c_str(),
		c->DatabasePort
	)) {
		LogError("Cannot continue without a database connection");

		return false;
	}

	/**
	 * Multi-tenancy: Content database
	 */
	if (!c->ContentDbHost.empty()) {
		if (!content_db.Connect(
			c->ContentDbHost.c_str(),
			c->ContentDbUsername.c_str(),
			c->ContentDbPassword.c_str(),
			c->ContentDbName.c_str(),
			c->ContentDbPort,
			"content"
		)) {
			LogError("Cannot continue without a content database connection");
			return false;
		}
	}
	else {
		content_db.SetMysql(database.getMySQL());
	}

	return true;
}

int get_file_size(const std::string &filename) // path to file
{
	FILE *p_file = NULL;
	p_file = fopen(filename.c_str(), "rb");
	fseek(p_file, 0, SEEK_END);
	int size = ftell(p_file);
	fclose(p_file);
	return size;
}


void WorldBoot::CheckForServerScript(bool force_download)
{
	const std::string file = fmt::format("{}/eqemu_server.pl", path.GetServerPath());
	std::ifstream     f(file);

	/* Fetch EQEmu Server script */
	if (!f || get_file_size(file) < 100 || force_download) {

		if (force_download) {
			std::remove(fmt::format("{}/eqemu_server.pl", path.GetServerPath()).c_str());
		} /* Delete local before fetch */

		std::cout << "Pulling down EQEmu Server Maintenance Script (eqemu_server.pl)..." << std::endl;

		// http get request
		uri u("https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_server.pl");

		httplib::Client r(
			fmt::format(
				"{}://{}",
				u.get_scheme(),
				u.get_host()
			).c_str()
		);

		r.set_connection_timeout(1, 0);
		r.set_read_timeout(1, 0);
		r.set_write_timeout(1, 0);

		if (auto res = r.Get(u.get_path().c_str())) {
			if (res->status == 200) {
				// write file

				std::string script = fmt::format("{}/eqemu_server.pl", path.GetServerPath());
				std::ofstream out(script);
				out << res->body;
				out.close();
#ifdef _WIN32
#else
				system(fmt::format("chmod 755 {}", script).c_str());
				system(fmt::format("chmod +x {}", script).c_str());
#endif
			}
		}
	}
}

void WorldBoot::CheckForXMLConfigUpgrade()
{
	if (!std::ifstream("eqemu_config.json") && std::ifstream("eqemu_config.xml")) {
		CheckForServerScript(true);
		std::string command = fmt::format("perl {}/eqemu_server.pl convert_xml", path.GetServerPath());
		if (system(command.c_str())) {}
	}
	else {
		CheckForServerScript();
	}
}

extern LoginServerList loginserverlist;

void WorldBoot::RegisterLoginservers()
{
	const auto c = EQEmuConfig::get();

	if (c->LoginCount == 0) {
		if (c->LoginHost.length()) {
			loginserverlist.Add(
				c->LoginHost.c_str(),
				c->LoginPort,
				c->LoginAccount.c_str(),
				c->LoginPassword.c_str(),
				c->LoginLegacy
			);
			LogInfo("Added loginserver [{}]:[{}]", c->LoginHost.c_str(), c->LoginPort);
		}
	}
	else {
		LinkedList<LoginConfig *>         loginlist = c->loginlist;
		LinkedListIterator<LoginConfig *> iterator(loginlist);
		iterator.Reset();
		while (iterator.MoreElements()) {
			if (iterator.GetData()->LoginHost.length()) {
				loginserverlist.Add(
					iterator.GetData()->LoginHost.c_str(),
					iterator.GetData()->LoginPort,
					iterator.GetData()->LoginAccount.c_str(),
					iterator.GetData()->LoginPassword.c_str(),
					iterator.GetData()->LoginLegacy
				);

				LogInfo(
					"Added loginserver [{}]:[{}]",
					iterator.GetData()->LoginHost.c_str(),
					iterator.GetData()->LoginPort
				);
			}
			iterator.Advance();
		}
	}
}

extern SharedTaskManager   shared_task_manager;
extern AdventureManager    adventure_manager;
extern WorldEventScheduler event_scheduler;

bool WorldBoot::DatabaseLoadRoutines(int argc, char **argv)
{
	// ignore
	bool ignore_db = false;
	if (argc >= 2) {
		if (strcasecmp(argv[1], "ignore_db") == 0) {
			ignore_db = true;
		}
		else {
			std::cerr << "Error, unknown command line option" << std::endl;
			return false;
		}
	}

	if (!ignore_db) {
		LogInfo("Checking Database Conversions");
		database.CheckDatabaseConversions();
	}

	// logging system init
	auto logging = LogSys.SetDatabase(&database)
		->SetLogPath(path.GetLogPath())
		->LoadLogDatabaseSettings();

	if (RuleB(Logging, WorldGMSayLogging)) {
		logging->SetGMSayHandler(&WorldBoot::GMSayHookCallBackProcessWorld);
	}

	logging->StartFileLogs();

	LogInfo("Loading variables");
	database.LoadVariables();

	std::string hotfix_name;
	if (database.GetVariable("hotfix_name", hotfix_name)) {
		if (!hotfix_name.empty()) {
			LogInfo("Current hotfix in use: [{}]", hotfix_name.c_str());
		}
	}

	guild_mgr.SetDatabase(&database);

	LogInfo("Purging expired data buckets");
	database.PurgeAllDeletedDataBuckets();

	LogInfo("Loading zones");

	zone_store.LoadZones(content_db);

	if (zone_store.GetZones().empty()) {
		LogError("Failed to load zones data, check your schema for possible errors");
		return 1;
	}

	LogInfo("Clearing groups");
	database.ClearGroup();
	LogInfo("Clearing raids");
	database.ClearRaid();
	database.ClearRaidDetails();
	database.ClearRaidLeader();
	LogInfo("Clearing inventory snapshots");
	database.ClearInvSnapshots();
	LogInfo("Loading items");

	if (!content_db.LoadItems(hotfix_name)) {
		LogError("Error: Could not load item data. But ignoring");
	}

	if (!content_db.LoadSkillCaps(std::string(hotfix_name))) {
		LogError("Error: Could not load skill cap data. But ignoring");
	}

	guild_mgr.LoadGuilds();

	//rules:
	{
		if (!RuleManager::Instance()->UpdateOrphanedRules(&database)) {
			LogInfo("Failed to process 'Orphaned Rules' update operation.");
		}

		if (!RuleManager::Instance()->UpdateInjectedRules(&database, "default")) {
			LogInfo("Failed to process 'Injected Rules' for ruleset 'default' update operation.");
		}

		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			LogInfo("Loading rule set [{}]", tmp.c_str());

			if (!RuleManager::Instance()->LoadRules(&database, tmp.c_str(), false)) {
				LogInfo("Failed to load ruleset [{}], falling back to defaults", tmp.c_str());
			}
		}
		else {

			if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
				LogInfo("No rule set configured, using default rules");
			}
		}

		if (!RuleManager::Instance()->RestoreRuleNotes(&database)) {
			LogInfo("Failed to process 'Restore Rule Notes' update operation.");
		}
	}

	EQ::InitializeDynamicLookups();

	if (RuleB(World, ClearTempMerchantlist)) {
		LogInfo("Clearing temporary merchant lists");
		database.ClearMerchantTemp();
	}

	LogInfo("Loading EQ time of day");
	TimeOfDay_Struct eqTime{};
	time_t           realtime;
	eqTime = database.LoadTime(realtime);
	zoneserver_list.worldclock.SetCurrentEQTimeOfDay(eqTime, realtime);

	LogInfo("Deleted [{}] stale player corpses from database", database.DeleteStalePlayerCorpses());

	LogInfo("Loading adventures");
	if (!adventure_manager.LoadAdventureTemplates()) {
		LogInfo("Unable to load adventure templates");
	}

	if (!adventure_manager.LoadAdventureEntries()) {
		LogInfo("Unable to load adventure templates");
	}

	adventure_manager.LoadLeaderboardInfo();

	LogInfo("Purging expired dynamic zones and members");
	dynamic_zone_manager.PurgeExpiredDynamicZones();

	LogInfo("Purging expired expeditions");
	ExpeditionDatabase::PurgeExpiredExpeditions();
	ExpeditionDatabase::PurgeExpiredCharacterLockouts();

	LogInfo("Purging expired character task timers");
	CharacterTaskTimersRepository::DeleteWhere(database, "expire_time <= NOW()");

	LogInfo("Purging expired instances");
	database.PurgeExpiredInstances();

	LogInfo("Loading dynamic zones");
	dynamic_zone_manager.LoadTemplates();
	dynamic_zone_manager.CacheAllFromDatabase();

	LogInfo("Loading char create info");
	content_db.LoadCharacterCreateAllocations();
	content_db.LoadCharacterCreateCombos();

	LogInfo("Initializing [EventScheduler]");
	event_scheduler.SetDatabase(&database)->LoadScheduledEvents();

	LogInfo("Initializing [WorldContentService]");
	content_service.SetDatabase(&database)
		->SetExpansionContext()
		->ReloadContentFlags();

	LogInfo("Initializing [SharedTaskManager]");
	shared_task_manager.SetDatabase(&database)
		->SetContentDatabase(&content_db)
		->LoadTaskData()
		->LoadSharedTaskState();

	shared_task_manager.PurgeExpiredSharedTasks();

	return true;
}

void WorldBoot::CheckForPossibleConfigurationIssues()
{
	const auto c = EQEmuConfig::get();

	if (c->DisableConfigChecks) {
		LogInfo("Configuration checking [disabled]");
		return;
	}

	const std::string   local_address  = IpUtil::GetLocalIPAddress();
	const std::string   public_address = IpUtil::GetPublicIPAddress();
	const std::string   config_file    = "eqemu_config.json";
	const std::ifstream is_in_docker("/.dockerenv");

	if (local_address.empty() && public_address.empty()) {
		LogInfo("Configuration check, probes failed for local and public address, returning");
		return;
	}

	LogInfo("Checking for possible configuration issues");
	LogInfo("To disable configuration checks, set [server.disable_config_checks] to [true] in [{}]", config_file);

	std::string config_address = c->WorldAddress;
	if (!IpUtil::IsIPAddress(config_address)) {
		config_address = IpUtil::DNSLookupSync(c->WorldAddress, 9000);
		LogInfo(
			"World config address using DNS [{}] resolves to [{}]",
			c->WorldAddress,
			config_address
		);
	}

	std::cout << std::endl;

	// lan detection
	if (local_address != public_address
		&& IpUtil::IsIpInPrivateRfc1918(local_address)
		&& local_address != c->LocalAddress
		&& !is_in_docker
		) {
		LogWarning("# LAN detection (Configuration)");
		LogWarning("");
		LogWarning("You appear to be on a LAN and your localaddress may not be properly set!");
		LogWarning("This can prevent local clients from properly connecting to your server");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server.world] variable [localaddress]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", c->LocalAddress, local_address);
		std::cout << std::endl;
	}

	// docker configuration
	if (
		(
			(c->LocalAddress.empty() && config_address.empty()) ||
			(config_address != public_address)
		)
		&& is_in_docker
		) {
		LogWarning("# Docker Configuration (Configuration)");
		LogWarning("");
		LogWarning("You appear to running EQEmu in a docker container");
		LogWarning("In order for networking to work properly you will need to properly configure your server");
		LogWarning("");
		LogWarning(
			"If your Docker host is on a [LAN] or behind a NAT / Firewall, your [localaddress] variable under [server.world] will need to");
		LogWarning(
			"be set to your LAN address on the host, not the container address. [address] will need to be your public address");
		LogWarning("");
		LogWarning(
			"If your Docker host is directly on the [public internet], your [localaddress] variable under [server.world] can be set to [127.0.0.1]."
		);
		LogWarning("");
		LogWarning("[address] will need to be your public address");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server.world] variable(s) [localaddress] [address]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", c->LocalAddress, local_address);
		LogWarning(
			"Public address (eqemu_config) value [{}] detected value [{}]",
			config_address,
			public_address
		);
		std::cout << std::endl;
	}

	// docker LAN not set
	if (c->LocalAddress.empty() && is_in_docker) {
		LogWarning("# Docker LAN (Configuration)");
		LogWarning("");
		LogWarning("You appear to running EQEmu in a docker container");
		LogWarning(
			"Your local address does not appear to be set, this may not be an issue if your deployment is not on a LAN"
		);
		LogWarning("");
		LogWarning(
			"If your Docker host is on a [LAN] or behind a NAT / Firewall, your [localaddress] variable under [server.world] will need to");
		LogWarning(
			"be set to your LAN address on the host, not the container address. [address] will need to be your public address");
		LogWarning("");
		LogWarning(
			"If your Docker host is directly on the [public internet], your [localaddress] variable under [server.world] can be set to [127.0.0.1]."
		);
		LogWarning("");
		LogWarning("[address] will need to be your public address");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server.world] variable(s) [localaddress] [address]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", c->LocalAddress, local_address);
		LogWarning(
			"Public address (eqemu_config) value [{}] detected value [{}]",
			config_address,
			public_address
		);
		std::cout << std::endl;
	}

	// public address different from configuration
	if (!config_address.empty() && public_address != config_address) {
		LogWarning("# Public address (Configuration)");
		LogWarning("");
		LogWarning("Your configured public address appears to be different from what's detected!");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server.world] variable [address]", config_file);
		LogWarning("");
		LogWarning(
			"Public address (eqemu_config) value [{}] detected value [{}]",
			config_address,
			public_address
		);
		std::cout << std::endl;
	}

	// public address set to meta-address
	if (config_address == "0.0.0.0") {
		LogWarning("# Public meta-address (Configuration)");
		LogWarning("");
		LogWarning("Your configured public address is set to a meta-address (0.0.0.0) (all-interfaces)");
		LogWarning(
			"The meta-address may not work properly and it is recommended you configure your public address explicitly");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server.world] variable [address]", config_file);
		LogWarning("");
		LogWarning(
			"Public address (eqemu_config) value [{}] detected value [{}]",
			config_address,
			public_address
		);
		std::cout << std::endl;
	}

	// local address set to meta-address
	if (c->LocalAddress == "0.0.0.0") {
		LogWarning("# Local meta-address (Configuration)");
		LogWarning("");
		LogWarning("Your configured local address is set to a meta-address (0.0.0.0) (all-interfaces)");
		LogWarning(
			"The meta-address may not work properly and it is recommended you configure your local address explicitly"
		);
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server.world] variable [localaddress]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", c->LocalAddress, local_address);
		std::cout << std::endl;
	}

	// ucs (public)
	if (
		(!config_address.empty() && c->MailHost != config_address) ||
		(!config_address.empty() && c->ChatHost != config_address)
		) {
		LogWarning("# UCS Address Mailhost (Configuration)");
		LogWarning("");
		LogWarning(
			"UCS (Universal Chat Service) mail or chat appears to use a different address from your main world address"
		);
		LogWarning("This can result in a chat service that doesn't network properly");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#mailserver]");
		LogWarning("");
		LogWarning(
			"[server.world.address] value [{}] [server.chatserver.host] [{}]",
			config_address,
			c->ChatHost
		);
		LogWarning(
			"[server.world.address] value [{}] [server.mailserver.host] [{}]",
			config_address,
			c->MailHost
		);
		std::cout << std::endl;
	}
}

