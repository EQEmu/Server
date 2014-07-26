#include "web_interface.h"
#include "call_handler.h"

extern std::map<std::string, CallHandler> authorized_calls;
extern std::map<std::string, CallHandler> unauthorized_calls;

void register_authorized_calls()
{
}

void register_unauthorized_calls()
{
	unauthorized_calls["token_auth"] = handle_call_token_auth;
}

void register_calls()
{
	register_authorized_calls();
	register_unauthorized_calls();
}

void handle_call_token_auth(per_session_data_eqemu *session, rapidjson::Document &document)
{
	if (!document.HasMember("token")) {
		WriteWebCallResponse(session, document, "token_missing");
		return;
	}

	session->auth = document["token"].GetString();
	if (!CheckTokenAuthorization(session)) {
		WriteWebCallResponse(session, document, "unauthorized");
	} else {
		WriteWebCallResponse(session, document, "authorized");
	}
}
