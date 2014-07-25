#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/EQEmuConfig.h"
#include "../common/web_interface_utils.h"
#include "../common/StringUtil.h"
#include "../common/uuid.h"
#include "worldserver.h"
#include "lib/libwebsockets.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <signal.h>
#include <list>
#include <map>

#define MAX_MESSAGE_LENGTH 2048

struct per_session_data_eqemu {
	bool auth;
	std::string uuid;
	std::list<std::string> *send_queue;
};

volatile bool run = true;
TimeoutManager timeout_manager;
const EQEmuConfig *config = nullptr;
WorldServer *worldserver = nullptr;
libwebsocket_context *context = nullptr;
std::map<std::string, per_session_data_eqemu*> sessions;

void CatchSignal(int sig_num) {
	run = false;
	if(worldserver)
		worldserver->Disconnect();
		
	if(context)
		libwebsocket_cancel_service(context);
}

int callback_http(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {
	switch (reason) {
	case LWS_CALLBACK_HTTP:
		libwebsockets_return_http_status(context, wsi, HTTP_STATUS_FORBIDDEN, NULL);
		break;
	case LWS_CALLBACK_HTTP_BODY_COMPLETION:
		libwebsockets_return_http_status(context, wsi, HTTP_STATUS_OK, NULL);
		return -1;
	default:
		break;
	};
	return 0;
}

int callback_eqemu(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {
	per_session_data_eqemu *session = (per_session_data_eqemu*)user;
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		session->auth = false;
		session->uuid = CreateUUID();
		session->send_queue = new std::list<std::string>();
		sessions[session->uuid] = session;
		printf("Created session %s\n", session->uuid.c_str());
		break;
	case LWS_CALLBACK_RECEIVE: {

		//recv and parse commands here
		if(len < 1)
			break;

		std::string command;
		command.assign((const char*)in, len);

		if(command.compare("get_version") == 0) {
			session->send_queue->push_back("0.8.0");
		}
		if (command.compare("do_pos_update") == 0){
			printf("Sending ServerOP_WIClientRequest with session %s Command Str %s \n", session->uuid.c_str(), command.c_str());
			/* Test Packet */
			ServerPacket* pack = new ServerPacket(ServerOP_WIClientRequest, sizeof(WI_Client_Request_Struct) + command.length() + 1);
			WI_Client_Request_Struct* WICR = (WI_Client_Request_Struct*)pack->pBuffer;
			strn0cpy(WICR->Client_UUID, session->uuid.c_str(), 64);
			strn0cpy(WICR->JSON_Data, command.c_str(), command.length() + 1); 
			worldserver->SendPacket(pack);
			safe_delete(pack);
		}

		break;
	}
	case LWS_CALLBACK_SERVER_WRITEABLE: {
		//send messages here
		char out_message[MAX_MESSAGE_LENGTH + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + 1];
		for (auto iter = session->send_queue->begin(); iter != session->send_queue->end(); ++iter) {

			//out_message
			size_t sz = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + (*iter).size();
			memset(out_message, 0, sz);
			memcpy(&out_message[LWS_SEND_BUFFER_PRE_PADDING], &(*iter)[0], (*iter).size());
			auto n = libwebsocket_write(wsi, (unsigned char*)&out_message[LWS_SEND_BUFFER_PRE_PADDING], (*iter).size(), LWS_WRITE_TEXT);
			if (n < (*iter).size()) {
				//couldn't write the message
				return -1;
			}
		}
		session->send_queue->clear();
		break;
	}
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		//clean up sessions here
		safe_delete(session->send_queue);
		break;

	case LWS_CALLBACK_CLOSED:
		printf("Closed session %s\n", session->uuid.c_str());
		//Session closed but perhaps not yet destroyed, we still don't want to track it though.
		sessions.erase(session->uuid);
		safe_delete(session->send_queue);
		session->uuid.clear();
		session->auth = false;
		break;
	default:
		break;
	};
	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	{ "http-only", callback_http, 0, 0, },
	{ "eqemu", callback_eqemu, sizeof(per_session_data_eqemu), 0, },
	{ nullptr, nullptr, 0, 0 }
};

int main() {
	RegisterExecutablePlatform(ExePlatformWebInterface);
	set_exception_handler();
	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect
	_log(WEB_INTERFACE__INIT, "Starting EQEmu Web Server.");
	
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(WEB_INTERFACE__ERROR, "Could not set signal handler");
		return 1;
	}
	
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(WEB_INTERFACE__ERROR, "Could not set signal handler");
		return 1;
	}

	Timer writable_socket_timer;
	config = EQEmuConfig::get();
	lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = config->WebInterfacePort;
	info.protocols = protocols;
	info.extensions = libwebsocket_get_internal_extensions();
	info.gid = -1;
	info.uid = -1;

	context = libwebsocket_create_context(&info);
	if (context == NULL) {
		_log(WEB_INTERFACE__ERROR, "Could not create websocket handler.");
		return 1;
	}

	worldserver = new WorldServer(config->SharedKey); 
	worldserver->Connect();
	writable_socket_timer.Start(10);

	while(run) { 
		Timer::SetCurrentTime(); 
		
		if (InterserverTimer.Check()) {
			if (worldserver->TryReconnect() && (!worldserver->Connected()))
				worldserver->AsyncConnect();
		}
		worldserver->Process(); 

		timeout_manager.CheckTimeouts();

		if (writable_socket_timer.Check(true)) {
			libwebsocket_callback_on_writable_all_protocol(&protocols[1]);
		}

		libwebsocket_service(context, 5);
		Sleep(1);
	}

	safe_delete(worldserver);
	libwebsocket_context_destroy(context);

	return 0;
}

