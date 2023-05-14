#include "../client.h"

void command_viewentityvariables(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	Mob *t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const auto &l = t->GetEntityVariables();

	uint32 variable_count  = 0;
	uint32 variable_number = 1;
	const std::string search_criteria = arguments ? sep->argplus[1] : "";

	for (const auto &e: l) {
		if (
			search_criteria.empty() ||
			Strings::Contains(Strings::ToLower(e), Strings::ToLower(search_criteria))
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Variable {} | Name: {} Value: {} | {}",
					variable_number,
					e,
					t->GetEntityVariable(e),
					Saylink::Silent(
						fmt::format(
							"#deleteentityvariable {}",
							e
						),
						"Delete"
					)
				).c_str()
			);

			variable_count++;
			variable_number++;
		}
	}

	if (!variable_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no entity variables{}.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has",
				(
					!search_criteria.empty() ?
					fmt::format(
						" matching '{}'",
						search_criteria
					) :
					""
				)
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} {} entity variable{}{}, would you like to {} them?",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			variable_count,
			variable_count != 1 ? "s" : "",
			(
				!search_criteria.empty() ?
				fmt::format(
					" matching '{}'",
					search_criteria
				) :
				""
			),
			Saylink::Silent(
				"#clearentityvariables",
				"clear"
			)
		).c_str()
	);
}
