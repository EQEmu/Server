#include "web_interface.h"
#include "method_handler.h"
#include "remote_call.h"

extern WorldServer *worldserver;
extern std::map<std::string, std::pair<int, MethodHandler>> authorized_methods;
extern std::map<std::string, MethodHandler> unauthorized_methods;

void register_authorized_methods()
{
	authorized_methods["token_auth"] = std::make_pair(0, handle_method_token_auth);
	authorized_methods["list_zones"] = std::make_pair(10, handle_method_no_args);
	authorized_methods["get_zone_info"] = std::make_pair(10, handle_method_get_zone_info);
	authorized_methods["subscribe"] = std::make_pair(10, handle_method_subscribe);
}

void register_unauthorized_methods()
{
	unauthorized_methods["token_auth"] = handle_method_token_auth;
}

void register_methods()
{
	register_authorized_methods();
	register_unauthorized_methods();
}

void handle_method_token_auth(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method)
{
	if (!document.HasMember("token")) {
		WriteWebCallResponseString(session, document, "Auth token missing", true);
		return;
	}

	session->auth = document["token"].GetString();
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
	uint32 sz = (uint32)(id.size() + session->uuid.size() + method.size() + 3 + 16);
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
	uint32 sz = (uint32)(id.size() + session->uuid.size() + method.size() + 3 + 16);
	auto &params = document["params"];
	auto &param = params[(rapidjson::SizeType)0];
	if(param.IsNull()) {
		sz += 5;
	}
	else {
		sz += (uint32)strlen(param.GetString());
		sz += 5;
	}

	ServerPacket *pack = new ServerPacket(ServerOP_WIRemoteCall, sz);
	pack->WriteUInt32((uint32)id.size());
	pack->WriteString(id.c_str());
	pack->WriteUInt32((uint32)session->uuid.size());
	pack->WriteString(session->uuid.c_str());
	pack->WriteUInt32((uint32)method.size());
	pack->WriteString(method.c_str());
	pack->WriteUInt32(1);
	pack->WriteUInt32((uint32)strlen(param.GetString()));
	pack->WriteString(param.GetString());
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void handle_method_subscribe(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method) {
	CheckParams(3, "[event, zone_id, instance_id]");
	VerifyID();
	uint32 sz = (uint32)(id.size() + session->uuid.size() + method.size() + 3 + 16);
	auto &params = document["params"];
	for(int i = 0; i < 3; ++i) {
		auto &param = params[i];
		if(param.IsNull()) {
			sz += 5;
		} else {
			sz += (uint32)strlen(param.GetString());
			sz += 5;
		}
	}
}
