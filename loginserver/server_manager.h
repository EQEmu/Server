#ifndef EQEMU_SERVERMANAGER_H
#define EQEMU_SERVERMANAGER_H

#include "../common/global_define.h"
#include "../common/servertalk.h"
#include "../common/packet_dump.h"
#include "../common/net/servertalk_server.h"
#include "world_server.h"
#include "client.h"
#include <list>

/**
 * Server manager class, deals with management of the world servers
 */
class ServerManager {
public:

	/**
	 * Constructor, sets up the TCP server and starts listening
	 */
	ServerManager();

	/**
	* Destructor, shuts down the TCP server.
	*/
	~ServerManager();

	/**
	 * Sends a request to world to see if the client is banned or suspended
	 *
	 * @param server_id
	 * @param client_account_id
	 * @param client_loginserver
	 */
	void SendUserToWorldRequest(
		unsigned int server_id,
		unsigned int client_account_id,
		const std::string &client_loginserver
	);

	/**
	 * Creates a server list packet for the client
	 *
	 * @param client
	 * @param sequence
	 * @return
	 */
	EQApplicationPacket *CreateServerListPacket(Client *client, uint32 sequence);

	/**
	 * Checks to see if there is a server exists with this name, ignoring option
	 *
	 * @param server_long_name
	 * @param server_short_name
	 * @param ignore
	 * @return
	 */
	bool ServerExists(std::string server_long_name, std::string server_short_name, WorldServer *ignore = nullptr);

	/**
	 * Destroys a server with this name, ignoring option
	 *
	 * @param server_long_name
	 * @param server_short_name
	 * @param ignore
	 */
	void DestroyServerByName(std::string server_long_name, std::string server_short_name, WorldServer *ignore = nullptr);

	/**
	 * @return
	 */
	const std::list<std::unique_ptr<WorldServer>> &getWorldServers() const;

private:

	/**
	 * Retrieves a server(if exists) by ip address
	 * Useful utility for the reconnect process
	 *
	 * @param ip_address
	 * @param port
	 * @return
	 */
	WorldServer *GetServerByAddress(const std::string &ip_address, int port);

	std::unique_ptr<EQ::Net::ServertalkServer> m_server_connection;
	std::list<std::unique_ptr<WorldServer>>    m_world_servers;

};

#endif

