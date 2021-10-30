#ifndef EQEMU_LOGINSERVER_H
#define EQEMU_LOGINSERVER_H

#include <utility>
#include "../common/json_config.h"
#include "database.h"
#include "encryption.h"
#include "options.h"
#include "server_manager.h"
#include "client_manager.h"
#include "loginserver_webserver.h"

/**
 * Login server struct, contains every variable for the server that needs to exist outside the scope of main()
 */
struct LoginServer
{
public:

	LoginServer() : db(nullptr), server_manager(nullptr) {

	}

	EQ::JsonConfigFile                 config;
	Database                           *db;
	LoginserverWebserver::TokenManager *token_manager{};
	Options                            options;
	ServerManager                      *server_manager;
	ClientManager                      *client_manager{};
};

#endif

