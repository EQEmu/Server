#include "../common/eqemu_logsys.h"
#include "web_interface.h"
#include "method_handler.h"
#include "remote_call.h"

EQEmuLogSys Log;
volatile bool run = true;
TimeoutManager timeout_manager;
const EQEmuConfig *config = nullptr;
WorldServer *worldserver = nullptr;
libwebsocket_context *context = nullptr;
SharedDatabase *db = nullptr;

std::map<std::string, per_session_data_eqemu*> sessions;
std::map<std::string, std::pair<int, MethodHandler>> authorized_methods;
std::map<std::string, MethodHandler> unauthorized_methods;

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
		session->uuid = CreateUUID();
		session->send_queue = new std::list<std::string>();
		sessions[session->uuid] = session;
		break;
	case LWS_CALLBACK_RECEIVE: {

		//recv and parse commands here
		if(len < 1)
			break;
			
		rapidjson::Document document;
		if(document.Parse((const char*)in).HasParseError()) {
			WriteWebCallResponseString(session, document, "Malformed JSON data", true, true);
			break;
		}

		std::string method;
		if(document.HasMember("method")) {
			method = document["method"].GetString();
		}

		if(method.length() == 0) {
			//No function called, toss this message
			WriteWebCallResponseString(session, document, "No method specified", true);
			break;
		}
		
		int status = CheckTokenAuthorization(session);
		if (status == 0) {
			//check func call against functions that dont req auth
			if (unauthorized_methods.count(method) == 0) {
				WriteWebCallResponseString(session, document, "No suitable method found: " + method, true);
				break;
			}
			auto call_func = unauthorized_methods[method];
			call_func(session, document, method);
		}
		else if(status > 0) {
			//check func call against functions that req auth
			if (authorized_methods.count(method) == 0) {
				WriteWebCallResponseString(session, document, "No suitable method found: " + method, true);
				break;
			}
		
			//check status level
			auto iter = authorized_methods.find(method);
			if(iter->second.first > status) {
				WriteWebCallResponseString(session, document, "Method " + method + " requires status " + std::to_string((long)iter->second.first), true);
				break;
			}

			auto call_func = iter->second.second;
			call_func(session, document, method);
		}

		break;
	}
	case LWS_CALLBACK_SERVER_WRITEABLE: {
		std::vector<char> out_message;
		for (auto iter = session->send_queue->begin(); iter != session->send_queue->end(); ++iter) {
			out_message.resize((*iter).size() + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + 1);
			memset(&out_message[0], 0, (*iter).size() + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING + 1);
			memcpy(&out_message[LWS_SEND_BUFFER_PRE_PADDING], &(*iter)[0], (*iter).size());
			int n = libwebsocket_write(wsi, (unsigned char*)&out_message[LWS_SEND_BUFFER_PRE_PADDING], (*iter).size(), LWS_WRITE_TEXT);
			if(n < (*iter).size()) {
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
		//Session closed but perhaps not yet destroyed, we still don't want to track it though.
		sessions.erase(session->uuid);
		safe_delete(session->send_queue);
		session->uuid.clear();
		break;
	default:
		break;
	};
	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	{ "http-only", callback_http, 0, 0, },
	{ "eqemu", callback_eqemu, sizeof(per_session_data_eqemu), 1048576, },
	{ nullptr, nullptr, 0, 0 }
};

int main() {
	RegisterExecutablePlatform(ExePlatformWebInterface);
	Log.LoadLogSettingsDefaults();
	
	set_exception_handler();
	register_methods();
	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect
	Log.Out(Logs::General, Logs::WebInterface_Server, "Starting EQEmu Web Server.");
	
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::General, Logs::Error, "Could not set signal handler");
		return 1;
	}
	
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::General, Logs::Error, "Could not set signal handler");
		return 1;
	}

	Timer writable_socket_timer;
	config = EQEmuConfig::get();
	lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = config->WebInterfacePort;
	info.protocols = protocols;
	info.extensions = nullptr;
	info.gid = -1;
	info.uid = -1;

	context = libwebsocket_create_context(&info);
	if (context == NULL) {
		Log.Out(Logs::General, Logs::Error, "Could not create websocket handler.");
		return 1;
	}

	db = new SharedDatabase();
	Log.Out(Logs::General, Logs::WebInterface_Server, "Connecting to database...");
	if(!db->Connect(config->DatabaseHost.c_str(), config->DatabaseUsername.c_str(),
		config->DatabasePassword.c_str(), config->DatabaseDB.c_str(), config->DatabasePort)) {
		Log.Out(Logs::General, Logs::WebInterface_Server, "Unable to connect to the database, cannot continue without a database connection");
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
	safe_delete(db);
	libwebsocket_context_destroy(context);

	return 0;
}