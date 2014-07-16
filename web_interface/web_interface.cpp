#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/EQEmuConfig.h"
#include "worldserver.h"
#include "lib/libwebsockets.h"
#include <signal.h>
#include <list>

volatile bool run = true;
TimeoutManager timeout_manager;
const EQEmuConfig *config = nullptr;
WorldServer *worldserver = nullptr;
libwebsocket_context *context = nullptr;

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

struct per_session_data_eqemu {
	bool auth;
	std::list<std::string> *send_queue;
};

int callback_eqemu(libwebsocket_context *context, libwebsocket *wsi, libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {
	per_session_data_eqemu *session = (per_session_data_eqemu*)user;
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		session->auth = false;
		session->send_queue = new std::list<std::string>();
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
	}
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		//send stuff here
		for (auto iter = session->send_queue->begin(); iter != session->send_queue->end(); ++iter) {
			auto n = libwebsocket_write(wsi, (unsigned char*)&(*iter)[0], (*iter).size(), LWS_WRITE_TEXT);
			if (n < (*iter).size()) {
				//couldn't write the message
				return -1;
			}
		}
		session->send_queue->clear();
		break;
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		//clean stuff up here
		delete session->send_queue;
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
	writable_socket_timer.Start(50);

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

