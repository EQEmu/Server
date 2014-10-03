#include "web_interface.h"
#include "method_handler.h"
#include "remote_call.h"

extern WorldServer *worldserver;
extern std::map<std::string, std::pair<int, MethodHandler>> authorized_methods;
extern std::map<std::string, MethodHandler> unauthorized_methods;

/* Web Interface:register_authorized_methods */
void register_authorized_methods()
{
	authorized_methods["WebInterface.Authorize"] = std::make_pair(0, handle_method_token_auth);
	authorized_methods["World.ListZones"] = std::make_pair(10, handle_method_world);
	authorized_methods["World.GetZoneDetails"] = std::make_pair(10, handle_method_world);
	authorized_methods["Zone.Subscribe"] = std::make_pair(10, handle_method_zone);
	authorized_methods["Zone.Unsubscribe"] = std::make_pair(10, handle_method_zone);
	authorized_methods["Zone.GetInitialEntityPositions"] = std::make_pair(10, handle_method_zone);
	authorized_methods["Zone.MoveEntity"] = std::make_pair(10, handle_method_zone); 
	authorized_methods["Zone.Action"] = std::make_pair(10, handle_method_zone);
	authorized_methods["Quest.GetScript"] = std::make_pair(10, handle_method_world);
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
		WriteWebCallResponseBoolean(session, document, false, false);
	} else {
		WriteWebCallResponseBoolean(session, document, true, false);
	}
}

void handle_method_world(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method) {
	CheckParams(0, "[]");
	VerifyID();
	CalculateSize();
	WriteWebProtocolPacket();
}

void handle_method_zone(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method) {
	CheckParams(2, "[zone_id, instance_id]");
	VerifyID();
	CalculateSize();
	WriteWebProtocolPacket();
}
