#include "client_manager.h"
#include "login_server.h"

extern LoginServer server;
extern bool        run_server;

#include "../common/eqemu_logsys.h"
#include "../common/misc.h"

ClientManager::ClientManager()
{
	int titanium_port = server.config.GetVariableInt("client_configuration", "titanium_port", 5998);

	EQStreamManagerInterfaceOptions titanium_opts(titanium_port, false, false);

	titanium_stream = new EQ::Net::EQStreamManager(titanium_opts);
	titanium_ops    = new RegularOpcodeManager;
	if (!titanium_ops->LoadOpcodes(
		server.config.GetVariableString(
			"client_configuration",
			"titanium_opcodes",
			"login_opcodes.conf"
		).c_str())) {

		LogError(
			"ClientManager fatal error: couldn't load opcodes for Titanium file [{0}]",
			server.config.GetVariableString("client_configuration", "titanium_opcodes", "login_opcodes.conf")
		);

		run_server = false;
	}

	titanium_stream->OnNewConnection(
		[this](std::shared_ptr<EQ::Net::EQStream> stream) {
			LogInfo(
				"New Titanium client connection from [{0}:{1}]",
				long2ip(stream->GetRemoteIP()),
				stream->GetRemotePort()
			);

			stream->SetOpcodeManager(&titanium_ops);
			Client *c = new Client(stream, cv_titanium);
			clients.push_back(c);
		}
	);

	int sod_port = server.config.GetVariableInt("client_configuration", "sod_port", 5999);

	EQStreamManagerInterfaceOptions sod_opts(sod_port, false, false);
	sod_stream = new EQ::Net::EQStreamManager(sod_opts);
	sod_ops    = new RegularOpcodeManager;
	if (
		!sod_ops->LoadOpcodes(
			server.config.GetVariableString(
				"client_configuration",
				"sod_opcodes",
				"login_opcodes.conf"
			).c_str()
		)) {
		LogError(
			"ClientManager fatal error: couldn't load opcodes for SoD file {0}",
			server.config.GetVariableString("client_configuration", "sod_opcodes", "login_opcodes.conf").c_str()
		);

		run_server = false;
	}

	sod_stream->OnNewConnection(
		[this](std::shared_ptr<EQ::Net::EQStream> stream) {
			LogInfo(
				"New SoD+ client connection from [{0}:{1}]",
				long2ip(stream->GetRemoteIP()),
				stream->GetRemotePort()
			);

			stream->SetOpcodeManager(&sod_ops);
			auto *c = new Client(stream, cv_sod);
			clients.push_back(c);
		}
	);
}

ClientManager::~ClientManager()
{
	if (titanium_stream) {
		delete titanium_stream;
	}

	if (titanium_ops) {
		delete titanium_ops;
	}

	if (sod_stream) {
		delete sod_stream;
	}

	if (sod_ops) {
		delete sod_ops;
	}
}

void ClientManager::Process()
{
	ProcessDisconnect();

	auto iter = clients.begin();
	while (iter != clients.end()) {
		if ((*iter)->Process() == false) {
			LogWarning("Client had a fatal error and had to be removed from the login");
			delete (*iter);
			iter = clients.erase(iter);
		}
		else {
			++iter;
		}
	}
}

void ClientManager::ProcessDisconnect()
{
	auto iter = clients.begin();
	while (iter != clients.end()) {
		std::shared_ptr<EQStreamInterface> c = (*iter)->GetConnection();
		if (c->CheckState(CLOSED)) {
			LogInfo("Client disconnected from the server, removing client");
			delete (*iter);
			iter = clients.erase(iter);
		}
		else {
			++iter;
		}
	}
}

/**
 * @param account_id
 * @param loginserver
 */
void ClientManager::RemoveExistingClient(unsigned int account_id, const std::string &loginserver)
{
	auto iter = clients.begin();
	while (iter != clients.end()) {
		if ((*iter)->GetAccountID() == account_id && (*iter)->GetLoginServerName().compare(loginserver) == 0) {
			LogInfo("Client attempting to log in existing client already logged in, removing existing client");
			delete (*iter);
			iter = clients.erase(iter);
		}
		else {
			++iter;
		}
	}
}

/**
 * @param account_id
 * @param loginserver
 * @return
 */
Client *ClientManager::GetClient(unsigned int account_id, const std::string &loginserver)
{
	auto iter = clients.begin();
	while (iter != clients.end()) {
		if ((*iter)->GetAccountID() == account_id && (*iter)->GetLoginServerName().compare(loginserver) == 0) {
			return (*iter);
		}
		++iter;
	}

	return nullptr;
}
