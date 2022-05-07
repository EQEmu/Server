#include "../client.h"
#include "../../common/serverinfo.h"

void command_serverinfo(Client *c, const Seperator *sep)
{
	auto os = EQ::GetOS();
	auto cpus = EQ::GetCPUs();
	auto process_id = EQ::GetPID();
	auto rss = EQ::GetRSS() / 1048576.0;
	auto uptime = static_cast<uint32>(EQ::GetUptime());

	std::string popup_text;

	popup_text.append("<c \"#00FF00\">Operating System Information</c>");

	popup_text.append("<table>");

	popup_text.append(
		fmt::format(
			"<tr><td>System</td><td>{}</td></tr>",
			os.sysname
		).c_str()
	);

	popup_text.append(
		fmt::format(
			"<tr><td>Release</td><td>{}</td></tr>",
			os.release
		)
	);

	popup_text.append(
		fmt::format(
			"<tr><td>Version</td><td>{}</td></tr>",
			os.version
		)
	);

	popup_text.append(
		fmt::format(
			"<tr><td>Machine</td><td>{}</td></tr>",
			os.machine
		)
	);

	popup_text.append(
		fmt::format(
			"<tr><td>Uptime</td><td>{}</td></tr>",
			ConvertSecondsToTime(uptime)
		)
	);

	popup_text.append("</table>");

	popup_text.append("<c \"#00FF00\">CPU Information</c>");

	popup_text.append("<table>");

	for (size_t cpu = 0; cpu < cpus.size(); ++cpu) {
		auto &current_cpu = cpus[cpu];
		popup_text.append(
			fmt::format(
				"<tr><td>CPU {}</td><td>{} ({:.2f}GHz)</td></tr>",
				cpu,
				current_cpu.model,
				current_cpu.speed
			)
		);
	}

	popup_text.append("</table>");

	popup_text.append("<c \"#00FF00\">Process Information</c>");

	popup_text.append("<table>");

	popup_text.append(
		fmt::format(
			"<tr><td>Process ID</td><td>{}</td></tr>",
			process_id
		)
	);

	popup_text.append(
		fmt::format(
			"<tr><td>RSS</td><td>{:.2f} MB</td></tr>",
			rss
		)
	);

	popup_text.append("</table>");

	c->SendPopupToClient(
		"Server Information",
		popup_text.c_str()
	);
}

