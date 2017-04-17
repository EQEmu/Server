/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "config.h"

/**
* Retrieves the variable we want from our title or theme
* First gets the map from the title
* Then gets the argument from the map we got from title
*/
std::string Config::GetVariable(std::string title, std::string parameter)
{
	std::map<std::string, std::map<std::string, std::string> >::iterator iter = vars.find(title);
	if(iter != vars.end())
	{
		std::map<std::string, std::string>::iterator arg_iter = iter->second.find(parameter);
		if(arg_iter != iter->second.end())
		{
			return arg_iter->second;
		}
	}

	return std::string("");
}

/**
* Opens a file and passes it to the tokenizer
* Then it parses the tokens returned and puts them into titles and variables.
*/
void Config::Parse(const char *file_name)
{
	if(file_name == nullptr)
	{
		Log(Logs::General, Logs::Error, "Config::Parse(), file_name passed was null.");
		return;
	}

	vars.clear();
	FILE *input = fopen(file_name, "r");
	if(input)
	{
		std::list<std::string> tokens;
		Tokenize(input, tokens);

		char mode = 0;
		std::string title, param, arg;
		std::list<std::string>::iterator iter = tokens.begin();
		while(iter != tokens.end())
		{
			if((*iter).compare("[") == 0)
			{
				title.clear();
				bool first = true;
				++iter;
				if(iter == tokens.end())
				{
					Log(Logs::General, Logs::Error, "Config::Parse(), EOF before title done parsing.");
					fclose(input);
					vars.clear();
					return;
				}

				while((*iter).compare("]") != 0 && iter != tokens.end())
				{
					if(!first)
					{
						title += " ";
					}
					else
					{
						first = false;
					}

					title += (*iter);
					++iter;
				}
				++iter;
			}

			if(mode == 0)
			{
				param = (*iter);
				mode++;
			}
			else if(mode == 1)
			{
				mode++;
				if((*iter).compare("=") != 0)
				{
					Log(Logs::General, Logs::Error, "Config::Parse(), invalid parse token where = should be.");
					fclose(input);
					vars.clear();
					return;
				}
			}
			else
			{
				arg = (*iter);
				mode = 0;
				std::map<std::string, std::map<std::string, std::string> >::iterator map_iter = vars.find(title);
				if(map_iter != vars.end())
				{
					map_iter->second[param] = arg;
					vars[title] = map_iter->second;
				}
				else
				{
					std::map<std::string, std::string> var_map;
					var_map[param] = arg;
					vars[title] = var_map;
				}
			}
			++iter;
		}
		fclose(input);
	}
	else
	{
		Log(Logs::General, Logs::Error, "Config::Parse(), file was unable to be opened for parsing.");
	}
}

/**
* Pretty basic lexical analyzer
* Breaks up the input character stream into tokens and puts them into the list provided.
* Ignores # as a line comment
*/
void Config::Tokenize(FILE *input, std::list<std::string> &tokens)
{
	auto c = fgetc(input);
	std::string lexeme;

	while(c != EOF)
	{
		if(isspace(c))
		{
			if(lexeme.size() > 0)
			{
				tokens.push_back(lexeme);
				lexeme.clear();
			}
			c = fgetc(input);
			continue;
		}

		if(isalnum(c))
		{
			lexeme += c;
			c = fgetc(input);
			continue;
		}

		switch(c)
		{
		case '#':
			{
				if(lexeme.size() > 0)
				{
					tokens.push_back(lexeme);
					lexeme.clear();
				}

				while(c != '\n' && c != EOF)
				{
					c = fgetc(input);
				}
				break;
			}
		case '[':
		case ']':
		case '=':
			{
				if(lexeme.size() > 0)
				{
					tokens.push_back(lexeme);
					lexeme.clear();
				}

				lexeme += c;
				tokens.push_back(lexeme);
				lexeme.clear();
				break;
			}
		default:
			{
				lexeme += c;
			}
		}

		c = fgetc(input);
	}

	if(lexeme.size() > 0)
	{
		tokens.push_back(lexeme);
	}
}

