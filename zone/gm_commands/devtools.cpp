#include "../client.h"
#include "../data_bucket.h"

void command_devtools(Client *c, const Seperator *sep)
{
	std::string dev_tools_key = StringFormat("%i-dev-tools-disabled", c->AccountID());

	/**
	 * Handle window toggle
	 */
	if (strcasecmp(sep->arg[1], "disable") == 0) {
		DataBucket::SetData(dev_tools_key, "true");
		c->SetDevToolsEnabled(false);
	}
	if (strcasecmp(sep->arg[1], "enable") == 0) {
		DataBucket::DeleteData(dev_tools_key);
		c->SetDevToolsEnabled(true);
	}

	c->ShowDevToolsMenu();
}

