#include "../common/global_define.h"
#include "../common/types.h"
#include "../common/opcodemgr.h"
#include "../common/event/event_loop.h"
#include "../common/timer.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/eqemu_logsys.h"
#include "../common/http/httplib.h"
#include "login_server.h"
#include "loginserver_webserver.h"
#include "loginserver_command_handler.h"
#include "../common/strings.h"
#include "../common/path_manager.h"
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <thread>

LoginServer server;
EQEmuLogSys LogSys;
bool        run_server = true;
PathManager path;

void ResolveAddresses();
void CatchSignal(int sig_num)
{
}

void LoadDatabaseConnection()
{
	LogInfo("MySQL Database Init");

	server.db = new Database(
		server.config.GetVariableString("database", "user", "root"),
		server.config.GetVariableString("database", "password", ""),
		server.config.GetVariableString("database", "host", "localhost"),
		server.config.GetVariableString("database", "port", "3306"),
		server.config.GetVariableString("database", "db", "peq")
	);

}

void LoadServerConfig()
{
	server.config = EQ::JsonConfigFile::Load(
		fmt::format("{}/login.json", path.GetServerPath())
	);
	LogInfo("Config System Init");

	/**
	 * Worldservers
	 */
	server.options.RejectDuplicateServers(
		server.config.GetVariableBool(
			"worldservers",
			"reject_duplicate_servers",
			false
		)
	);
	server.options.SetShowPlayerCount(server.config.GetVariableBool("worldservers", "show_player_count", false));
	server.options.AllowUnregistered(
		server.config.GetVariableBool(
			"worldservers",
			"unregistered_allowed",
			true
		)
	);
	server.options.SetWorldDevTestServersListBottom(
		server.config.GetVariableBool(
			"worldservers",
			"dev_test_servers_list_bottom",
			false
		)
	);
	server.options.SetWorldSpecialCharacterStartListBottom(
		server.config.GetVariableBool(
			"worldservers",
			"special_character_start_list_bottom",
			false
		)
	);

	/**
	 * Expansion Display Settings
	 */
	server.options.DisplayExpansions(
		server.config.GetVariableBool(
			"client_configuration",
			"display_expansions",
			false
		)); //disable by default
	server.options.MaxExpansions(
		server.config.GetVariableInt(
			"client_configuration",
			"max_expansions_mask",
			67108863
		)); //enable display of all expansions

	/**
	 * Account
	 */
	server.options.AutoCreateAccounts(server.config.GetVariableBool("account", "auto_create_accounts", true));
	server.options.AutoLinkAccounts(server.config.GetVariableBool("account", "auto_link_accounts", false));

#ifdef LSPX
	server.options.EQEmuLoginServerAddress(
		server.config.GetVariableString(
			"general",
			"eqemu_loginserver_address",
			"login.eqemulator.net:5999"
		)
	);
#endif

	/**
	 * Default Loginserver Name (Don't change)
	 */
	server.options.DefaultLoginServerName(
		server.config.GetVariableString(
			"general",
			"default_loginserver_name",
			"local"
		)
	);

	/**
	 * Security
	 */

#ifdef ENABLE_SECURITY
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 13));
#else
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 6));
#endif

	server.options.AllowTokenLogin(server.config.GetVariableBool("security", "allow_token_login", false));
	server.options.AllowPasswordLogin(server.config.GetVariableBool("security", "allow_password_login", true));
	server.options.UpdateInsecurePasswords(
		server.config.GetVariableBool(
			"security",
			"update_insecure_passwords",
			true
		)
	);
}

void start_web_server()
{
	Sleep(1);

	int web_api_port = server.config.GetVariableInt("web_api", "port", 6000);
	LogInfo("Webserver API now listening on port [{0}]", web_api_port);

	httplib::Server api;

	api.set_logger(
		[](const auto &req, const auto &res) {
			if (!req.path.empty()) {
				LogInfo("[API] Request [{}] via [{}:{}]", req.path, req.remote_addr, req.remote_port);
			}
		}
	);

	LoginserverWebserver::RegisterRoutes(api);
	api.listen("0.0.0.0", web_api_port);
}

int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();

	LogInfo("Logging System Init");

	if (argc == 1) {
		LogSys.LoadLogSettingsDefaults();
	}

	path.LoadPaths();

	/**
	 * Command handler
	 */
	if (argc > 1) {
		LogSys.SilenceConsoleLogging();

		LoadServerConfig();
		LoadDatabaseConnection();

		LogSys.LoadLogSettingsDefaults();
		LogSys.log_settings[Logs::Debug].log_to_console = static_cast<uint8>(Logs::General);
		LogSys.log_settings[Logs::Debug].is_category_enabled = 1;

		LoginserverCommandHandler::CommandHandler(argc, argv);
	}

	LoadServerConfig();

	/**
	 * mysql connect
	 */
	LoadDatabaseConnection();

	if (argc == 1) {
		LogSys.SetDatabase(server.db)
			->SetLogPath("logs")
			->LoadLogDatabaseSettings()
			->StartFileLogs();
	}

	/**
	 * make sure our database got created okay, otherwise cleanup and exit
	 */
	if (!server.db) {
		LogError("Database Initialization Failure");
		LogInfo("Log System Shutdown");
		return 1;
	}

	/**
	 * create server manager
	 */
	LogInfo("Server Manager Init");
	server.server_manager = new ServerManager();
	if (!server.server_manager) {
		LogError("Server Manager Failed to Start");
		LogInfo("Database System Shutdown");
		delete server.db;
		return 1;
	}

	/**
	 * create client manager
	 */
	LogInfo("Client Manager Init");
	server.client_manager = new ClientManager();
	if (!server.client_manager) {
		LogError("Client Manager Failed to Start");
		LogInfo("Server Manager Shutdown");
		delete server.server_manager;

		LogInfo("Database System Shutdown");
		delete server.db;
		return 1;
	}

#ifdef WIN32
#ifdef UNICODE
		SetConsoleTitle(L"EQEmu Login Server");
#else
		SetConsoleTitle("EQEmu Login Server");
#endif
#endif

	LogInfo("Server Started");

	/**
	 * Web API
	 */
	bool web_api_enabled = server.config.GetVariableBool("web_api", "enabled", true);
	if (web_api_enabled) {
		std::thread web_api_thread(start_web_server);
		web_api_thread.detach();
	}

	LogInfo("[Config] [Account] CanAutoCreateAccounts [{0}]", server.options.CanAutoCreateAccounts());
	LogInfo("[Config] [ClientConfiguration] DisplayExpansions [{0}]", server.options.IsDisplayExpansions());
	LogInfo("[Config] [ClientConfiguration] MaxExpansions [{0}]", server.options.GetMaxExpansions());

#ifdef LSPX
	LogInfo("[Config] [Account] CanAutoLinkAccounts [{0}]", server.options.CanAutoLinkAccounts());
#endif
	LogInfo("[Config] [WorldServer] IsRejectingDuplicateServers [{0}]", server.options.IsRejectingDuplicateServers());
	LogInfo("[Config] [WorldServer] IsUnregisteredAllowed [{0}]", server.options.IsUnregisteredAllowed());
	LogInfo("[Config] [WorldServer] ShowPlayerCount [{0}]", server.options.IsShowPlayerCountEnabled());
	LogInfo(
		"[Config] [WorldServer] DevAndTestServersListBottom [{0}]",
		server.options.IsWorldDevTestServersListBottom()
	);
	LogInfo(
		"[Config] [WorldServer] SpecialCharactersStartListBottom [{0}]",
		server.options.IsWorldSpecialCharacterStartListBottom()
	);
	LogInfo("[Config] [Security] GetEncryptionMode [{0}]", server.options.GetEncryptionMode());
	LogInfo("[Config] [Security] IsTokenLoginAllowed [{0}]", server.options.IsTokenLoginAllowed());
	LogInfo("[Config] [Security] IsPasswordLoginAllowed [{0}]", server.options.IsPasswordLoginAllowed());
	LogInfo("[Config] [Security] IsUpdatingInsecurePasswords [{0}]", server.options.IsUpdatingInsecurePasswords());

	auto loop_fn = [&](EQ::Timer* t) {
		Timer::SetCurrentTime();

		if (!run_server) {
			EQ::EventLoop::Get().Shutdown();
			return;
		}

		server.client_manager->Process();
	};

	EQ::Timer process_timer(loop_fn);
	process_timer.Start(32, true);

	EQ::EventLoop::Get().Run();

	LogInfo("Server Shutdown");

	LogInfo("Client Manager Shutdown");
	delete server.client_manager;

	LogInfo("Server Manager Shutdown");
	delete server.server_manager;

	LogInfo("Database System Shutdown");
	delete server.db;

	return 0;
}
