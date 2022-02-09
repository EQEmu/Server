#include "../client.h"
#include "../../common/serverinfo.h"

void command_serverinfo(Client *c, const Seperator *sep)
{
	auto os     = EQ::GetOS();
	auto cpus   = EQ::GetCPUs();
	auto pid    = EQ::GetPID();
	auto rss    = EQ::GetRSS();
	auto uptime = EQ::GetUptime();

	c->Message(Chat::White, "Operating System Information");
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "System: %s", os.sysname.c_str());
	c->Message(Chat::White, "Release: %s", os.release.c_str());
	c->Message(Chat::White, "Version: %s", os.version.c_str());
	c->Message(Chat::White, "Machine: %s", os.machine.c_str());
	c->Message(Chat::White, "Uptime: %.2f seconds", uptime);
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "CPU Information");
	c->Message(Chat::White, "==================================================");
	for (size_t i = 0; i < cpus.size(); ++i) {
		auto &cp = cpus[i];
		c->Message(Chat::White, "CPU #%i: %s, Speed: %.2fGhz", i, cp.model.c_str(), cp.speed);
	}
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "Process Information");
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "PID: %u", pid);
	c->Message(Chat::White, "RSS: %.2f MB", rss / 1048576.0);
	c->Message(Chat::White, "==================================================");
}

