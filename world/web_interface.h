#pragma once

#include "../common/net/servertalk_server_connection.h"
#include "../common/json/json.h"
#include <map>
#include <string>
#include <functional>



class WebInterface
{
public:
	typedef std::function<void(WebInterface *, const std::string&, const std::string&, const Json::Value&)> WebInterfaceCall;
	WebInterface(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	~WebInterface();

	std::string GetUUID() const { return m_connection->GetUUID(); }
	void SendResponse(const std::string &id, const Json::Value &response);
	void SendError(const std::string &message);
	void SendError(const std::string &message, const std::string &id);
	void SendEvent(const Json::Value &value);
	void AddCall(const std::string &method, WebInterfaceCall call);
private:
	void OnCall(uint16 opcode, EQ::Net::Packet &p);
	void Send(const Json::Value &value);

	std::shared_ptr<EQ::Net::ServertalkServerConnection> m_connection;
	std::map<std::string, WebInterfaceCall> m_calls;
};

class WebInterfaceList
{
public:
	WebInterfaceList();
	~WebInterfaceList();

	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void SendResponse(const std::string &uuid, std::string &id, const Json::Value &response);
	void SendEvent(const Json::Value &value);
	void SendError(const std::string &uuid, const std::string &message);
	void SendError(const std::string &uuid, const std::string &message, const std::string &id);

private:
	std::map<std::string, std::unique_ptr<WebInterface>> m_interfaces;
};
