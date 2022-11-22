#include "../client.h"
#include "../dialogue_window.h"
#include "../../common/serverinfo.h"

void command_serverinfo(Client *c, const Seperator *sep)
{
	auto os = EQ::GetOS();
	auto cpus = EQ::GetCPUs();
	auto process_id = EQ::GetPID();
	auto rss = EQ::GetRSS() / 1048576.0;
	auto uptime = static_cast<uint32>(EQ::GetUptime());

	std::string popup_table;
	auto popup_text = DialogueWindow::CenterMessage(
		DialogueWindow::ColorMessage("green", "Operating System Information")
	);

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("Machine") +
			DialogueWindow::TableCell(os.machine)
		).c_str()
	);

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("System") +
			DialogueWindow::TableCell(os.sysname)
		).c_str()
	);

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("Release") +
			DialogueWindow::TableCell(os.release)
		).c_str()
	);

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("Uptime") +
			DialogueWindow::TableCell(Strings::SecondsToTime(uptime))
		).c_str()
	);

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("Version") +
			DialogueWindow::TableCell(os.version)
		).c_str()
	);

	popup_text.append(DialogueWindow::Table(popup_table));

	popup_table = std::string();

	popup_text.append(DialogueWindow::Break());

	popup_text.append(
		DialogueWindow::CenterMessage(
			DialogueWindow::ColorMessage("green", "CPU Information")
		)
	);

	for (size_t cpu = 0; cpu < cpus.size(); ++cpu) {
		auto &current_cpu = cpus[cpu];
		popup_table.append(
			DialogueWindow::TableRow(
				DialogueWindow::TableCell(
					fmt::format(
						"CPU {}",
						cpu
					)
				) +
				DialogueWindow::TableCell(
					fmt::format(
						"{} ({:.2f}GHz)",
						current_cpu.model,
						current_cpu.speed
					)
				)
			)
		);
	}

	popup_text.append(DialogueWindow::Table(popup_table));

	popup_table = std::string();

	popup_text.append(DialogueWindow::Break());

	popup_text.append(
		DialogueWindow::CenterMessage(
			DialogueWindow::ColorMessage("green", "CPU Information")
		)
	);

	popup_text.append("<table>");

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("Process ID") +
			DialogueWindow::TableCell(std::to_string(process_id))
		)
	);

	popup_table.append(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("RSS") +
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f} MB",
					rss
				)
			)
		)
	);

	popup_text.append(DialogueWindow::Table(popup_table));

	c->SendPopupToClient(
		"Server Information",
		popup_text.c_str()
	);
}

