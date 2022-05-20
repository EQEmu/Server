#include "../client.h"

void command_version(Client *c, const Seperator *sep)
{
	std::string popup_text = "<table>";

	popup_text += fmt::format("<tr><td>Version</td><td>{}</td></tr>", CURRENT_VERSION);
	popup_text += fmt::format("<tr><td>Compiled</td><td>{} {}</td></tr>", COMPILE_DATE, COMPILE_TIME);
	popup_text += fmt::format("<tr><td>Last Modified</td><td>{}</td></tr>", LAST_MODIFIED);

	popup_text += "</table>";

	c->SendPopupToClient(
		"Server Version Information",
		popup_text.c_str()
	);
}

