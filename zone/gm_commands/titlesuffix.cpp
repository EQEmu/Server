#include "../client.h"
#include "../titles.h"

void command_titlesuffix(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #titlesuffix [Title Suffix] (use \"-1\" to remove title suffix)"
		);
		return;
	}
	
	bool is_remove = !strcasecmp(sep->argplus[1], "-1");
	std::string suffix = is_remove ? "" : sep->argplus[1];
		
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

	if (is_remove) {
		target->SetTitleSuffix(suffix);
	} else {
		title_manager.CreateNewPlayerSuffix(target, suffix);
	}

	target->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title suffix has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
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
