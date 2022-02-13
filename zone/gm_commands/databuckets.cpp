#include "../client.h"
#include "../data_bucket.h"

void command_databuckets(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::Yellow, "Usage: #databuckets view (partial key)|(limit) OR #databuckets delete (key)");
		return;
	}
	if (strcasecmp(sep->arg[1], "view") == 0) {

		std::string key_filter;
		uint8       limit = 50;
		for (int    i     = 2; i < 4; i++) {
			if (sep->arg[i][0] == '\0') {
				break;
			}
			if (strcasecmp(sep->arg[i], "limit") == 0) {
				limit = (uint8) atoi(sep->arg[i + 1]);
				continue;
			}
		}
		if (sep->arg[2]) {
			key_filter = str_tolower(sep->arg[2]);
		}
		std::string query = "SELECT `id`, `key`, `value`, `expires` FROM data_buckets";
		if (!key_filter.empty()) { query += StringFormat(" WHERE `key` LIKE '%%%s%%'", key_filter.c_str()); }
		query += StringFormat(" LIMIT %u", limit);
		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}
		if (results.RowCount() == 0) {
			c->Message(Chat::Yellow, "No data_buckets found");
			return;
		}
		int         _ctr         = 0;
		// put in window for easier readability in case want command line for something else
		std::string window_title = "Data Buckets";
		std::string window_text  =
						"<table>"
						"<tr>"
						"<td>ID</td>"
						"<td>Expires</td>"
						"<td>Key</td>"
						"<td>Value</td>"
						"</tr>";
		for (auto   row          = results.begin(); row != results.end(); ++row) {
			auto        id      = static_cast<uint32>(atoi(row[0]));
			std::string key     = row[1];
			std::string value   = row[2];
			std::string expires = row[3];
			window_text.append(
				StringFormat(
					"<tr>"
					"<td>%u</td>"
					"<td>%s</td>"
					"<td>%s</td>"
					"<td>%s</td>"
					"</tr>",
					id,
					expires.c_str(),
					key.c_str(),
					value.c_str()
				));
			_ctr++;
			std::string del_saylink = StringFormat("#databuckets delete %s", key.c_str());
			c->Message(
				Chat::White,
				"%s : %s",
				EQ::SayLinkEngine::GenerateQuestSaylink(del_saylink, false, "Delete").c_str(),
				key.c_str(),
				"  Value:  ",
				value.c_str());
		}
		window_text.append("</table>");
		c->SendPopupToClient(window_title.c_str(), window_text.c_str());
		std::string response = _ctr > 0 ? StringFormat("Found %i matching data buckets", _ctr).c_str()
			: "No Databuckets found.";
		c->Message(Chat::Yellow, response.c_str());
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (DataBucket::DeleteData(sep->argplus[2])) {
			c->Message(Chat::Yellow, "data bucket %s deleted.", sep->argplus[2]);
		}
		else {
			c->Message(Chat::Red, "An error occurred deleting data bucket %s", sep->argplus[2]);
		}
		return;
	}
}

