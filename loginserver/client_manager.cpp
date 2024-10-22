#include "client_manager.h"
#include "login_server.h"

extern LoginServer server;
extern bool        run_server;

#include "../common/eqemu_logsys.h"
#include "../common/misc.h"
#include "../common/path_manager.h"
#include "../common/file.h"

void CheckTitaniumOpcodeFile(const std::string &path) {
	if (File::Exists(path)) {
		return;
	}

	auto f = fopen(path.c_str(), "w");
	if (f) {
		fprintf(f, "#EQEmu Public Login Server OPCodes\n");
		fprintf(f, "OP_SessionReady=0x0001\n");
		fprintf(f, "OP_Login=0x0002\n");
		fprintf(f, "OP_ServerListRequest=0x0004\n");
		fprintf(f, "OP_PlayEverquestRequest=0x000d\n");
		fprintf(f, "OP_PlayEverquestResponse=0x0021\n");
		fprintf(f, "OP_ChatMessage=0x0016\n");
		fprintf(f, "OP_LoginAccepted=0x0017\n");
		fprintf(f, "OP_ServerListResponse=0x0018\n");
		fprintf(f, "OP_Poll=0x0029\n");
		fprintf(f, "OP_EnterChat=0x000f\n");
		fprintf(f, "OP_PollResponse=0x0011\n");
		fclose(f);
	}
}

void CheckSoDOpcodeFile(const std::string& path) {
	if (File::Exists(path)) {
		return;
	}

	auto f = fopen(path.c_str(), "w");
	if (f) {
		fprintf(f, "#EQEmu Public Login Server OPCodes\n");
		fprintf(f, "OP_SessionReady=0x0001\n");
		fprintf(f, "OP_Login=0x0002\n");
		fprintf(f, "OP_ServerListRequest=0x0004\n");
		fprintf(f, "OP_PlayEverquestRequest=0x000d\n");
		fprintf(f, "OP_PlayEverquestResponse=0x0022\n");
		fprintf(f, "OP_ChatMessage=0x0017\n");
		fprintf(f, "OP_LoginAccepted=0x0018\n");
		fprintf(f, "OP_ServerListResponse=0x0019\n");
		fprintf(f, "OP_Poll=0x0029\n");
		fprintf(f, "OP_LoginExpansionPacketData=0x0031\n");
		fprintf(f, "OP_EnterChat=0x000f\n");
		fprintf(f, "OP_PollResponse=0x0011\n");
		fclose(f);
	}
}

void CheckLarionOpcodeFile(const std::string& path) {
	if (File::Exists(path)) {
		return;
	}

	auto f = fopen(path.c_str(), "w");
	if (f) {
		fprintf(f, "#EQEmu Public Login Server OPCodes\n");
		fprintf(f, "OP_SessionReady=0x0001\n");
		fprintf(f, "OP_Login=0x0002\n");
		fprintf(f, "OP_ServerListRequest=0x0004\n");
		fprintf(f, "OP_PlayEverquestRequest=0x000d\n");
		fprintf(f, "OP_PlayEverquestResponse=0x0022\n");
		fprintf(f, "OP_ChatMessage=0x0017\n");
		fprintf(f, "OP_LoginAccepted=0x0018\n");
		fprintf(f, "OP_ServerListResponse=0x0019\n");
		fprintf(f, "OP_Poll=0x0029\n");
		fprintf(f, "OP_EnterChat=0x000f\n");
		fprintf(f, "OP_PollResponse=0x0011\n");
		fprintf(f, "OP_SystemFingerprint=0x0016\n");
		fprintf(f, "OP_ExpansionList=0x0030\n");
		fclose(f);
	}
}

ClientManager::ClientManager()
{
	int titanium_port = server.config.GetVariableInt("client_configuration", "titanium_port", 5998);

	EQStreamManagerInterfaceOptions titanium_opts(titanium_port, false, false);

	titanium_stream = new EQ::Net::EQStreamManager(titanium_opts);
	titanium_ops    = new RegularOpcodeManager;

	std::string opcodes_path = fmt::format(
		"{}/{}",
		path.GetOpcodePath(),
		"login_opcodes.conf"
	);

	CheckTitaniumOpcodeFile(opcodes_path);

	if (!titanium_ops->LoadOpcodes(opcodes_path.c_str())) {
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

	opcodes_path = fmt::format(
		"{}/{}",
		path.GetOpcodePath(),
		"login_opcodes_sod.conf"
	);

	CheckSoDOpcodeFile(opcodes_path);

	if (!sod_ops->LoadOpcodes(opcodes_path.c_str())) {
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

	int larion_port = server.config.GetVariableInt("client_configuration", "larion_port", 15900);

	EQStreamManagerInterfaceOptions larion_opts(larion_port, false, false);

	larion_stream = new EQ::Net::EQStreamManager(larion_opts);
	larion_ops = new RegularOpcodeManager;

	opcodes_path = fmt::format(
		"{}/{}",
		path.GetOpcodePath(),
		"login_opcodes_larion.conf"
	);

	CheckLarionOpcodeFile(opcodes_path);

	if (!larion_ops->LoadOpcodes(opcodes_path.c_str())) {
		LogError(
			"ClientManager fatal error: couldn't load opcodes for Larion file [{0}]",
			server.config.GetVariableString("client_configuration", "larion_opcodes", "login_opcodes.conf")
		);

		run_server = false;
	}

	larion_stream->OnNewConnection(
		[this](std::shared_ptr<EQ::Net::EQStream> stream) {
			LogInfo(
				"New Larion client connection from [{0}:{1}]",
				long2ip(stream->GetRemoteIP()),
				stream->GetRemotePort()
			);

			stream->SetOpcodeManager(&larion_ops);
			Client* c = new Client(stream, cv_larion);
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
