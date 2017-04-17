/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef EQEMU_LOGINSERVER_H
#define EQEMU_LOGINSERVER_H

#include "config.h"
#include "database.h"
#include "database_mysql.h"
#include "database_postgresql.h"
#include "encryption.h"
#include "options.h"
#include "server_manager.h"
#include "client_manager.h"

/**
* Login server struct, contains every variable for the server that needs to exist
* outside the scope of main().
*/
struct LoginServer
{
public:
	LoginServer() : config(nullptr), db(nullptr), server_manager(nullptr) { }

	Config *config;
	Database *db;
	Options options;
	ServerManager *server_manager;
	ClientManager *client_manager;
};

#endif

