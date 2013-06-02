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
#ifndef EQEMU_CONFIG_H
#define EQEMU_CONFIG_H

#include <stdio.h>
#include <list>
#include <map>
#include <string>

/**
 * Keeps track of all the configuration for the application with a small parser.
 * Note: This is not a thread safe class, but only parse writes to variables in the class.
 * Thus making it mostly safe so long as you're careful with where you call Parse()
 */
class Config
{
public:
	Config() { }
	~Config() { }

	/**
	* Parses the selected file for variables, will clear current variables if selected.
	*/
	virtual void Parse(const char *file_name);

	/**
	* Gets a variable if it exists.
	*/
	std::string GetVariable(std::string title, std::string parameter);

protected:
	std::map<std::string, std::map<std::string, std::string> > vars;

private:
	/**
	* Breaks our input up into tokens for Parse().
	* This is private because it's not intended to be overloaded by a derived class which
	* may get it's input from other places than a C file pointer. (a http get request for example).
	* The programmer of a derived class would be expected to make their own Tokenize function for their own Parse().
	*/
	void Tokenize(FILE* input, std::list<std::string> &tokens);
};

#endif

