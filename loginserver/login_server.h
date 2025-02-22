#ifndef EQEMU_LOGINSERVER_H
#define EQEMU_LOGINSERVER_H

#include <utility>
#include "../common/json_config.h"
#include "encryption.h"
#include "options.h"
#include "world_server_manager.h"
#include "client_manager.h"
#include "loginserver_webserver.h"

struct LoginServer {
public:

	LoginServer() : server_manager(nullptr)
	{

	}

	EQ::JsonConfigFile                 config;
	LoginserverWebserver::TokenManager *token_manager{};
	Options                            options;
	WorldServerManager                 *server_manager;
	ClientManager                      *client_manager{};
};

#endif

