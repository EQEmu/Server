#include "../client.h"

void command_rules(Client *c, const Seperator *sep)
{
	//super-command for managing rules settings
	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #rules [subcommand].");
		c->Message(Chat::White, "-- Rule Set Manipulation --");
		c->Message(Chat::White, "...listsets - List avaliable rule sets");
		c->Message(Chat::White, "...current - gives the name of the ruleset currently running in this zone");
		c->Message(Chat::White, "...reload - Reload the selected ruleset in this zone");
		c->Message(Chat::White, "...switch (ruleset name) - Change the selected ruleset and load it");
		c->Message(
			Chat::White,
			"...load (ruleset name) - Load a ruleset in just this zone without changing the selected set"
		);
//too lazy to write this right now:
//		c->Message(Chat::White, "...wload (ruleset name) - Load a ruleset in all zones without changing the selected set");
		c->Message(Chat::White, "...store [ruleset name] - Store the running ruleset as the specified name");
		c->Message(Chat::White, "---------------------");
		c->Message(Chat::White, "-- Running Rule Manipulation --");
		c->Message(Chat::White, "...reset - Reset all rules to their default values");
		c->Message(Chat::White, "...get [rule] - Get the specified rule's local value");
		c->Message(Chat::White, "...set (rule) (value) - Set the specified rule to the specified value locally only");
		c->Message(
			Chat::White,
			"...setdb (rule) (value) - Set the specified rule to the specified value locally and in the DB"
		);
		c->Message(
			Chat::White,
			"...list [catname] - List all rules in the specified category (or all categiries if omitted)"
		);
		c->Message(Chat::White, "...values [catname] - List the value of all rules in the specified category");
		return;
	}

	if (!strcasecmp(sep->arg[1], "current")) {
		c->Message(
			Chat::White, "Currently running ruleset '%s' (%d)", RuleManager::Instance()->GetActiveRuleset(),
			RuleManager::Instance()->GetActiveRulesetID());
	}
	else if (!strcasecmp(sep->arg[1], "listsets")) {
		std::map<int, std::string> sets;
		if (!RuleManager::Instance()->ListRulesets(&database, sets)) {
			c->Message(Chat::Red, "Failed to list rule sets!");
			return;
		}

		c->Message(Chat::White, "Avaliable rule sets:");
		std::map<int, std::string>::iterator cur, end;
		cur = sets.begin();
		end = sets.end();
		for (; cur != end; ++cur) {
			c->Message(Chat::White, "(%d) %s", cur->first, cur->second.c_str());
		}
	}
	else if (!strcasecmp(sep->arg[1], "reload")) {
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
		c->Message(
			Chat::White, "The active ruleset (%s (%d)) has been reloaded", RuleManager::Instance()->GetActiveRuleset(),
			RuleManager::Instance()->GetActiveRulesetID());
	}
	else if (!strcasecmp(sep->arg[1], "switch")) {
		//make sure this is a valid rule set..
		int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
		if (rsid < 0) {
			c->Message(Chat::Red, "Unknown rule set '%s'", sep->arg[2]);
			return;
		}
		if (!database.SetVariable("RuleSet", sep->arg[2])) {
			c->Message(Chat::Red, "Failed to update variables table to change selected rule set");
			return;
		}

		//TODO: we likely want to reload this ruleset everywhere...
		RuleManager::Instance()->LoadRules(&database, sep->arg[2], true);

		c->Message(
			Chat::White,
			"The selected ruleset has been changed to (%s (%d)) and reloaded locally",
			sep->arg[2],
			rsid
		);
	}
	else if (!strcasecmp(sep->arg[1], "load")) {
		//make sure this is a valid rule set..
		int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
		if (rsid < 0) {
			c->Message(Chat::Red, "Unknown rule set '%s'", sep->arg[2]);
			return;
		}
		RuleManager::Instance()->LoadRules(&database, sep->arg[2], true);
		c->Message(Chat::White, "Loaded ruleset '%s' (%d) locally", sep->arg[2], rsid);
	}
	else if (!strcasecmp(sep->arg[1], "store")) {
		if (sep->argnum == 1) {
			//store current rule set.
			RuleManager::Instance()->SaveRules(&database);
			c->Message(Chat::White, "Rules saved");
		}
		else if (sep->argnum == 2) {
			RuleManager::Instance()->SaveRules(&database, sep->arg[2]);
			int prersid = RuleManager::Instance()->GetActiveRulesetID();
			int rsid    = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
			if (rsid < 0) {
				c->Message(Chat::Red, "Unable to query ruleset ID after store, it most likely failed.");
			}
			else {
				c->Message(Chat::White, "Stored rules as ruleset '%s' (%d)", sep->arg[2], rsid);
				if (prersid != rsid) {
					c->Message(Chat::White, "Rule set %s (%d) is now active in this zone", sep->arg[2], rsid);
				}
			}
		}
		else {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
	}
	else if (!strcasecmp(sep->arg[1], "reset")) {
		RuleManager::Instance()->ResetRules(true);
		c->Message(Chat::White, "The running ruleset has been set to defaults");

	}
	else if (!strcasecmp(sep->arg[1], "get")) {
		if (sep->argnum != 2) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		std::string value;
		if (!RuleManager::Instance()->GetRule(sep->arg[2], value)) {
			c->Message(Chat::Red, "Unable to find rule %s", sep->arg[2]);
		}
		else {
			c->Message(Chat::White, "%s - %s", sep->arg[2], value.c_str());
		}

	}
	else if (!strcasecmp(sep->arg[1], "set")) {
		if (sep->argnum != 3) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		if (!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], nullptr, false, true)) {
			c->Message(Chat::Red, "Failed to modify rule");
		}
		else {
			c->Message(Chat::White, "Rule modified locally.");
		}
	}
	else if (!strcasecmp(sep->arg[1], "setdb")) {
		if (sep->argnum != 3) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		if (!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], &database, true, true)) {
			c->Message(Chat::Red, "Failed to modify rule");
		}
		else {
			c->Message(Chat::White, "Rule modified locally and in the database.");
		}
	}
	else if (!strcasecmp(sep->arg[1], "list")) {
		if (sep->argnum == 1) {
			std::vector<const char *> rule_list;
			if (!RuleManager::Instance()->ListCategories(rule_list)) {
				c->Message(Chat::Red, "Failed to list categories!");
				return;
			}
			c->Message(Chat::White, "Rule Categories:");
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for (; cur != end; ++cur) {
				c->Message(Chat::White, " %s", *cur);
			}
		}
		else if (sep->argnum == 2) {
			const char *catfilt = nullptr;
			if (std::string("all") != sep->arg[2]) {
				catfilt = sep->arg[2];
			}
			std::vector<const char *> rule_list;
			if (!RuleManager::Instance()->ListRules(catfilt, rule_list)) {
				c->Message(Chat::Red, "Failed to list rules!");
				return;
			}
			c->Message(Chat::White, "Rules in category %s:", sep->arg[2]);
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for (; cur != end; ++cur) {
				c->Message(Chat::White, " %s", *cur);
			}
		}
		else {
			c->Message(Chat::Red, "Invalid argument count, see help.");
		}
	}
	else if (!strcasecmp(sep->arg[1], "values")) {
		if (sep->argnum != 2) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		else {
			const char *catfilt = nullptr;
			if (std::string("all") != sep->arg[2]) {
				catfilt = sep->arg[2];
			}
			std::vector<const char *> rule_list;
			if (!RuleManager::Instance()->ListRules(catfilt, rule_list)) {
				c->Message(Chat::Red, "Failed to list rules!");
				return;
			}
			c->Message(Chat::White, "Rules & values in category %s:", sep->arg[2]);
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for (std::string tmp_value; cur != end; ++cur) {
				if (RuleManager::Instance()->GetRule(*cur, tmp_value)) {
					c->Message(Chat::White, " %s - %s", *cur, tmp_value.c_str());
				}
			}
		}

	}
	else {
		c->Message(Chat::Yellow, "Invalid action specified. use '#rules help' for help");
	}
}


