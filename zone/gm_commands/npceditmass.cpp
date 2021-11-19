#include "../client.h"

void command_npceditmass(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "usage") == 0) {
		c->Message(
			Chat::White,
			"#npceditmass search_column [exact_match: =]search_value change_column change_value (apply)"
		);
		return;
	}

	std::string query = SQL(
		SELECT
			COLUMN_NAME
		FROM
		INFORMATION_SCHEMA.COLUMNS
		WHERE
			table_name = 'npc_types'
		AND
		COLUMN_NAME != 'id'
	);

	std::string search_column, search_value, change_column, change_value;
	if (sep->arg[1]) {
		search_column = sep->arg[1];
	}
	if (sep->arg[2]) {
		search_value = sep->arg[2];
	}
	if (sep->arg[3]) {
		change_column = sep->arg[3];
	}
	if (sep->arg[4]) {
		change_value = sep->arg[4];
	}

	bool valid_change_column = false;
	bool valid_search_column = false;
	auto results             = content_db.QueryDatabase(query);

	std::vector<std::string> possible_column_options;

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (row[0] == change_column) {
			valid_change_column = true;
		}
		if (row[0] == search_column) {
			valid_search_column = true;
		}

		possible_column_options.push_back(row[0]);
	}

	std::string options_glue = ", ";

	if (!valid_search_column) {
		c->Message(Chat::Red, "You must specify a valid search column. [%s] is not valid", search_column.c_str());
		c->Message(Chat::Yellow, "Possible columns [%s]", implode(options_glue, possible_column_options).c_str());
		return;
	}

	if (!valid_change_column) {
		c->Message(Chat::Red, "You must specify a valid change column. [%s] is not valid", change_column.c_str());
		c->Message(Chat::Yellow, "Possible columns [%s]", implode(options_glue, possible_column_options).c_str());
		return;
	}

	if (!valid_search_column || !valid_change_column) {
		c->Message(Chat::Red, "One requested column is invalid");
		return;
	}

	query = fmt::format(
		SQL(
			select
			id,
			name,
			{ 0 },
			{ 1 }
				from
					npc_types
				where
				id IN(
					select
				spawnentry.npcID
				from
					spawnentry
				join spawn2 on spawn2.spawngroupID = spawnentry.spawngroupID
				where
				spawn2.zone = '{2}' and spawn2.version = {3}
			)
		),
		search_column,
		change_column,
		zone->GetShortName(),
		zone->GetInstanceVersion()
	);

	std::string status = "(Searching)";

	if (strcasecmp(sep->arg[5], "apply") == 0) {
		status = "(Applying)";
	}

	std::vector<std::string> npc_ids;

	bool exact_match = false;
	if (search_value[0] == '=') {
		exact_match  = true;
		search_value = search_value.substr(1);
	}

	int found_count = 0;
	results = content_db.QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {

		std::string npc_id                      = row[0];
		std::string npc_name                    = row[1];
		std::string search_column_value         = str_tolower(row[2]);
		std::string change_column_current_value = row[3];

		if (exact_match) {
			if (search_column_value.compare(search_value) != 0) {
				continue;
			}
		}
		else {
			if (search_column_value.find(search_value) == std::string::npos) {
				continue;
			}
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ({0}) [{1}] ({2}) [{3}] Current ({4}) [{5}] New [{6}] {7}",
				npc_id,
				npc_name,
				search_column,
				search_column_value,
				change_column,
				change_column_current_value,
				change_value,
				status
			).c_str()
		);

		npc_ids.push_back(npc_id);

		found_count++;
	}

	std::string saylink = fmt::format(
		"#npceditmass {} {}{} {} {} apply",
		search_column,
		(exact_match ? "=" : ""),
		search_value,
		change_column,
		change_value
	);

	if (strcasecmp(sep->arg[5], "apply") == 0) {
		std::string npc_ids_string = implode(",", npc_ids);
		if (npc_ids_string.empty()) {
			c->Message(Chat::Red, "Error: Ran into an unknown error compiling NPC IDs");
			return;
		}

		content_db.QueryDatabase(
			fmt::format(
				"UPDATE `npc_types` SET {} = '{}' WHERE id IN ({})",
				change_column,
				change_value,
				npc_ids_string
			)
		);

		c->Message(Chat::Yellow, "Changes applied to (%i) NPC's", found_count);
		zone->Repop();
	}
	else {
		c->Message(Chat::Yellow, "Found (%i) NPC's that match this search...", found_count);

		if (found_count > 0) {
			c->Message(
				Chat::Yellow, "To apply these changes, click <%s> or type [%s]",
				EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Apply").c_str(),
				saylink.c_str()
			);
		}
	}
}

