#include "web_interface.h"
#include "method_handler.h"
#include "remote_call.h"

extern WorldServer *worldserver;
extern std::map<std::string, std::pair<int, MethodHandler>> authorized_methods;
extern std::map<std::string, MethodHandler> unauthorized_methods;

/* Web Interface */
void register_authorized_methods()
{
	authorized_methods["WebInterface.Authorize"] = std::make_pair(0, handle_method_token_auth);
	authorized_methods["World.ListZones"] = std::make_pair(10, handle_method_no_args);
	authorized_methods["World.GetZoneDetails"] = std::make_pair(10, handle_method_get_zone_info);
	authorized_methods["Zone.Subscribe"] = std::make_pair(10, handle_method_subscribe);
	authorized_methods["Zone.Unsubscribe"] = std::make_pair(10, handle_method_subscribe);
	authorized_methods["Zone.GetInitialEntityPositions"] = std::make_pair(10, handle_method_void_event);
}

void register_unauthorized_methods()
{
	unauthorized_methods["WebInterface.Authorize"] = handle_method_token_auth;
}

void register_methods()
{
	register_authorized_methods();
	register_unauthorized_methods();
}

void handle_method_token_auth(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method)
{
	CheckParams(1, "[token]");
	
	session->auth = document["params"][(rapidjson::SizeType)0].GetString();
	if (!CheckTokenAuthorization(session)) {
		WriteWebCallResponseBoolean(session, document, "false", false);
	} else {
		WriteWebCallResponseBoolean(session, document, "true", false);
	}
}

void handle_method_no_args(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method)
{
	CheckParams(0, "[]");
	VerifyID();
	CalculateSize();

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCall, sz);
	pack->WriteUInt32((uint32)id.size());
	pack->WriteString(id.c_str());
	pack->WriteUInt32((uint32)session->uuid.size());
	pack->WriteString(session->uuid.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32(0);
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void handle_method_get_zone_info(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method)
{
	CheckParams(1, "[zoneserver_id]");
	VerifyID();
	CalculateSize();

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCall, sz);
	pack->WriteUInt32((uint32)id.size());
	pack->WriteString(id.c_str());
	pack->WriteUInt32((uint32)session->uuid.size());
	pack->WriteString(session->uuid.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32(1);

	auto &params = document["params"];
	auto &param = params[(rapidjson::SizeType)0];
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void handle_method_subscribe(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method) {
	CheckParams(3, "[zone_id, instance_id, event_name]");
	VerifyID();
	CalculateSize();

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCall, sz);
	pack->WriteUInt32((uint32)id.size());
	pack->WriteString(id.c_str());
	pack->WriteUInt32((uint32)session->uuid.size());
	pack->WriteString(session->uuid.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32(3);

	auto &params = document["params"];
	auto &param = params[(rapidjson::SizeType)0];
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());

	param = params[1];
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());

	param = params[2];
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void handle_method_void_event(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method) {
	CheckParams(2, "[zone_id, instance_id]");
	VerifyID();
	CalculateSize();

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCall, sz);
	pack->WriteUInt32((uint32)id.size());
	pack->WriteString(id.c_str());
	pack->WriteUInt32((uint32)session->uuid.size());
	pack->WriteString(session->uuid.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32(2);

	auto &params = document["params"];
	auto &param = params[(rapidjson::SizeType)0];
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());

	param = params[1];
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());

	worldserver->SendPacket(pack);
	safe_delete(pack);
}