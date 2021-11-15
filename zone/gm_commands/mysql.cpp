#include "../client.h"

void command_mysql(Client *c, const Seperator *sep)
{
	if (!sep->arg[1][0] || !sep->arg[2][0]) {
		c->Message(Chat::White, "Usage: #mysql query \"Query here\"");
		return;
	}

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "MYSQL In-Game CLI Interface:");
		c->Message(Chat::White, "Example: #mysql query \"Query goes here quoted\" -s -h");
		c->Message(Chat::White, "To use 'like \"%%something%%\" replace the %% with #");
		c->Message(Chat::White, "Example: #mysql query \"select * from table where name like \"#something#\"");
		c->Message(Chat::White, "-s - Spaces select entries apart");
		c->Message(Chat::White, "-h - Colors every other select result");
		return;
	}

	if (strcasecmp(sep->arg[1], "query") == 0) {
		///Parse switches here
		int  argnum  = 3;
		bool optionS = false;
		bool optionH = false;
		while (sep->arg[argnum] && strlen(sep->arg[argnum]) > 1) {
			switch (sep->arg[argnum][1]) {
				case 's':
					optionS = true;
					break;
				case 'h':
					optionH = true;
					break;
				default:
					c->Message(Chat::Yellow, "%s, there is no option '%c'", c->GetName(), sep->arg[argnum][1]);
					return;
			}
			++argnum;
		}

		int         highlightTextIndex = 0;
		std::string query(sep->arg[2]);
		//swap # for % so like queries can work
		std::replace(query.begin(), query.end(), '#', '%');
		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		//Using sep->arg[2] again, replace # with %% so it doesn't screw up when sent through vsnprintf in Message
		query = sep->arg[2];
		int pos = query.find('#');
		while (pos != std::string::npos) {
			query.erase(pos, 1);
			query.insert(pos, "%%");
			pos = query.find('#');
		}
		c->Message(Chat::Yellow, "---Running query: '%s'", query.c_str());

		for (auto row = results.begin(); row != results.end(); ++row) {
			std::stringstream        lineText;
			std::vector<std::string> lineVec;
			for (int                 i = 0; i < results.RowCount(); i++) {
				//split lines that could overflow the buffer in Client::Message and get cut off
				//This will crash MQ2 @ 4000 since their internal buffer is only 2048.
				//Reducing it to 2000 fixes that but splits more results from tables with a lot of columns.
				if (lineText.str().length() > 4000) {
					lineVec.push_back(lineText.str());
					lineText.str("");
				}
				lineText << results.FieldName(i) << ":" << "[" << (row[i] ? row[i] : "nullptr") << "] ";
			}

			lineVec.push_back(lineText.str());

			if (optionS) { //This provides spacing for the space switch
				c->Message(Chat::White, " ");
			}
			if (optionH) { //This option will highlight every other row
				highlightTextIndex = 1 - highlightTextIndex;
			}

			for (int lineNum = 0; lineNum < lineVec.size(); ++lineNum)
				c->Message(highlightTextIndex, lineVec[lineNum].c_str());
		}
	}
}

