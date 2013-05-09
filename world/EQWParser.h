/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#ifndef EQWPARSER_H_
#define EQWPARSER_H_



#ifdef EMBPERL

#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <string>

#include "../common/useperl.h"

class HTTPRequest;

class EQWParser {
public:
	EQWParser();
	~EQWParser();

	void EQW_eval(const char *pkg, const char *code);
	void SetHTTPRequest(const char *pkg, HTTPRequest *it);

	//put an integer into a perl varable
	void seti(const char *varname, int val) const {
		SV *t = get_sv(varname, true);
		sv_setiv(t, val);
	}
	//put a real into a perl varable
	void setd(const char *varname, float val) const {
		SV *t = get_sv(varname, true);
		sv_setnv(t, val);
	}
	//put a string into a perl varable
	void setstr(const char *varname, const char *val) const {
		SV *t = get_sv(varname, true);
		sv_setpv(t, val);
	}

protected:
	void DoInit();
	bool eval(const char * code, std::string &error);
	bool dosub(const char * subname, const std::vector<std::string> &args, std::string &error, int mode = G_SCALAR|G_DISCARD|G_EVAL);
	bool eval_file(const char * packagename, const char * filename, std::string &error);

	//the embedded interpreter
	PerlInterpreter * my_perl;
	SV *_empty_sv;
};
#endif //EMBPERL

#endif /*EQWPARSER_H_*/

