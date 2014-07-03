/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
/*#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>*/
#include <websocketpp/common/thread.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "database.h"
#include "socket_server_config.h"
#include "worldserver.h"
#include <list>

#include <signal.h>
volatile bool RunLoops = true;
TimeoutManager timeout_manager;
Database database;
std::string WorldShortName;

const socket_server_config *Config;
WorldServer *worldserver = 0;

void CatchSignal(int sig_num) {
	RunLoops = false;
	if(worldserver)
		worldserver->Disconnect();
}

/* Web Sockets Start Shit */

enum action_type {
	SUBSCRIBE,
	UNSUBSCRIBE,
	MESSAGE
};

struct action {
	action(action_type t, connection_hdl h) : type(t), hdl(h) {}
	action(action_type t, connection_hdl h, server::message_ptr m)
		: type(t), hdl(h), msg(m) {}

	action_type type;
	websocketpp::connection_hdl hdl;
	server::message_ptr msg;
};

class broadcast_server {
public:
	broadcast_server() {
		// Initialize Asio Transport
		m_server.init_asio();

		// Register handler callbacks
		m_server.set_open_handler(bind(&broadcast_server::on_open, this, ::_1));
		m_server.set_close_handler(bind(&broadcast_server::on_close, this, ::_1));
		m_server.set_message_handler(bind(&broadcast_server::on_message, this, ::_1, ::_2));
	}

	void run(uint16_t port) {
		// listen on specified port
		m_server.listen(port);

		// Start the server accept loop
		m_server.start_accept();

		// Start the ASIO io_service run loop
		try {
			m_server.run();
		}
		catch (const std::exception & e) {
			std::cout << e.what() << std::endl;
		}
		catch (websocketpp::lib::error_code e) {
			std::cout << e.message() << std::endl;
		}
		catch (...) {
			std::cout << "other exception" << std::endl;
		}
	}

	void on_open(connection_hdl hdl) {
		unique_lock<mutex> lock(m_action_lock);
		//std::cout << "on_open" << std::endl;
		m_actions.push(action(SUBSCRIBE, hdl));
		lock.unlock();
		m_action_cond.notify_one();
	}

	void on_close(connection_hdl hdl) {
		unique_lock<mutex> lock(m_action_lock);
		//std::cout << "on_close" << std::endl;
		m_actions.push(action(UNSUBSCRIBE, hdl));
		lock.unlock();
		m_action_cond.notify_one();
	}

	void on_message(connection_hdl hdl, server::message_ptr msg) {
		// queue message up for sending by processing thread
		unique_lock<mutex> lock(m_action_lock);
		msg->set_payload("Niggers");
		// std::cout << "on_message" << std::endl;
		m_actions.push(action(MESSAGE, hdl, msg));
		lock.unlock();
		m_action_cond.notify_one();
	}

	void process_messages() {
		while (1) {
			unique_lock<mutex> lock(m_action_lock);

			while (m_actions.empty()) {
				m_action_cond.wait(lock);
			}

			action a = m_actions.front();
			m_actions.pop();

			lock.unlock();

			if (a.type == SUBSCRIBE) {
				unique_lock<mutex> con_lock(m_connection_lock);
				m_connections.insert(a.hdl);
			}
			else if (a.type == UNSUBSCRIBE) {
				unique_lock<mutex> con_lock(m_connection_lock);
				m_connections.erase(a.hdl);
			}
			else if (a.type == MESSAGE) {
				unique_lock<mutex> con_lock(m_connection_lock);

				con_list::iterator it;
				for (it = m_connections.begin(); it != m_connections.end(); ++it) {
					m_server.send(*it, a.msg);
				}
			}
			else {
				// undefined.
			}
		}
	}
private:
	typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

	server m_server;
	con_list m_connections;
	std::queue<action> m_actions;

	mutex m_action_lock;
	mutex m_connection_lock;
	condition_variable m_action_cond;
};

/* Web Sockets Shit End*/

int main() {

	try {
		broadcast_server server_instance;

		// Start a thread to run the processing loop
		thread t(bind(&broadcast_server::process_messages, &server_instance));

		// Run the asio loop with the main thread
		server_instance.run(9002);

		t.join();

	}
	catch (std::exception & e) {
		std::cout << e.what() << std::endl;
	}

	RegisterExecutablePlatform(ExePlatformSocket_Server);
	set_exception_handler();
	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect
	_log(SOCKET_SERVER__INIT, "Starting EQEmu Socket Server.");
	if (!socket_server_config::LoadConfig()) {
		_log(SOCKET_SERVER__INIT, "Loading server configuration failed.");
		return 1;
	}

	Config = socket_server_config::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(SOCKET_SERVER__INIT, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		_log(SOCKET_SERVER__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());

	WorldShortName = Config->ShortName;

	/*
	_log(SOCKET_SERVER__INIT, "Connecting to MySQL...");
	
	if (!database.Connect(
		Config->QSDatabaseHost.c_str(),
		Config->QSDatabaseUsername.c_str(),
		Config->QSDatabasePassword.c_str(),
		Config->QSDatabaseDB.c_str(),
		Config->QSDatabasePort)) {
		_log(WORLD__INIT_ERR, "Cannot continue without a database connection.");
		return 1;
	}
	*/

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(SOCKET_SERVER__ERROR, "Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(SOCKET_SERVER__ERROR, "Could not set signal handler");
		return 1;
	}

	worldserver = new WorldServer; 
	worldserver->Connect();

	while(RunLoops) { 
		Timer::SetCurrentTime(); 
		if (InterserverTimer.Check()) {
			if (worldserver->TryReconnect() && (!worldserver->Connected()))
				worldserver->AsyncConnect();
		}
		worldserver->Process(); 
		timeout_manager.CheckTimeouts(); 
		Sleep(100);
	}



}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef _WINDOWS
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "SOCKET_SERVER: %s", iNewTitle);
	}
	else {
		snprintf(tmp, sizeof(tmp), "SOCKET_SERVER");
	}
	SetConsoleTitle(tmp);
#endif
}
