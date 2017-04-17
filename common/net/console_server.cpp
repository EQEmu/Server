#include "console_server.h"
#include "../string_util.h"
#include <fmt/format.h>

EQ::Net::ConsoleServer::ConsoleServer(const std::string &addr, int port)
{
	m_server.reset(new EQ::Net::TCPServer());
	m_server->Listen(addr, port, false, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		ConsoleServerConnection *c = new ConsoleServerConnection(this, connection);
		m_connections.insert(std::make_pair(c->GetUUID(), std::unique_ptr<ConsoleServerConnection>(c)));
	});
}

EQ::Net::ConsoleServer::~ConsoleServer()
{
}

void EQ::Net::ConsoleServer::RegisterCall(const std::string &command, int status_required, const std::string& help_definition, ConsoleServerCallback fn)
{
	m_commands[command] = { fn, status_required, help_definition };
}

void EQ::Net::ConsoleServer::RegisterLogin(ConsoleServerLoginCallback fn)
{
	m_login = fn;
}

void EQ::Net::ConsoleServer::ConnectionDisconnected(ConsoleServerConnection *c)
{
	auto iter = m_connections.find(c->GetUUID());
	if (iter != m_connections.end()) {
		m_connections.erase(iter);
	}
}

void EQ::Net::ConsoleServer::ProcessCommand(ConsoleServerConnection *c, const std::string &cmd)
{
	auto split = SplitString(cmd, ' ');
	
	if (split.size() > 0) {
		auto command = split[0];
		ToLowerString(command);

		if (command == "help" || command == "?") {
			c->SendLine("Commands:");
			for (auto &cmd_def : m_commands) {
				if (cmd_def.second.status_required <= c->Admin()) {
					auto display = fmt::format("  {0}", cmd_def.second.help_definition);
					c->SendLine(display);
				}
			}

			c->SendPrompt();
			return;
		}
	
		split.erase(split.begin(), split.begin() + 1);
		
		auto cmd_def = m_commands.find(command);
		if (cmd_def != m_commands.end()) {
			if (c->Admin() >= cmd_def->second.status_required) {
				cmd_def->second.fn(c, command, split);
				c->SendPrompt();
			}
			else {
				c->SendLine(fmt::format("Access denied for command: {0}", command));
				c->SendPrompt();
			}
		}
		else {
			c->SendLine(fmt::format("Command not found: {0}", command));
			c->SendPrompt();
		}
	}
	else {
		c->SendPrompt();
	}
}
