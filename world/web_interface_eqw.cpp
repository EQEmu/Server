#include "web_interface_eqw.h"
#include "web_interface.h"
#include "world_config.h"
#include "login_server_list.h"

extern LoginServerList loginserverlist;

void EQW__IsLocked(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	Json::Value ret = WorldConfig::get()->Locked;
	i->SendResponse(id, ret);
}

void EQW__Lock(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	WorldConfig::LockWorld();
	if (loginserverlist.Connected()) {
		loginserverlist.SendStatus();
	}

	Json::Value ret;
	ret["status"] = "complete";
	i->SendResponse(id, ret);
}

void EQW__Unlock(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	WorldConfig::UnlockWorld();
	if (loginserverlist.Connected()) {
		loginserverlist.SendStatus();
	}

	Json::Value ret;
	ret["status"] = "complete";
	i->SendResponse(id, ret);
}

void RegisterEQW(WebInterface *i)
{
	i->AddCall("EQW::IsLocked", std::bind(EQW__IsLocked, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::Lock", std::bind(EQW__Lock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::Unlock", std::bind(EQW__Unlock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}
