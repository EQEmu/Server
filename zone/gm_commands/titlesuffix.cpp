#include "../client.h"
#include "../titles.h"

void command_titlesuffix(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #titlesuffix [Remove|Title] [Save (0 = False, 1 = True)]"
		);
		return;
	}
	
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	std::string suffix = is_remove ? "" : sep->arg[1];
	bool save_suffix = sep->IsNumber(2) ? atobool(sep->arg[2]) : false;
		
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	if (suffix.size() > 31) {
		c->Message(Chat::White, "Title suffix must be 31 characters or less.");
		return;
	}

	if (!suffix.empty()) {
		find_replace(suffix, "_", " ");
	}

	if (!save_suffix || is_remove) {
		target->SetTitleSuffix(suffix.c_str());
	} else if (save_suffix) {
		title_manager.CreateNewPlayerSuffix(target, suffix.c_str());
	}

	target->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title suffix has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove && save_suffix ? " and saved" : "",
			c->GetTargetDescription(target),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					suffix
				)
			)
		).c_str()
	);
}
