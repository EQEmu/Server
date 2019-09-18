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

//a lot of this is copied from embperl.cpp, but I didnt feel like factoring the common stuff out

#ifdef EMBPERL

#include "../common/global_define.h"
#include "eqw_parser.h"
#include "eqw.h"
#include "../common/eqdb.h"

#include "worlddb.h"

#ifndef GvCV_set
#define GvCV_set(gv,cv) (GvCV(gv) = (cv))
#endif

XS(XS_EQWIO_PRINT);

//so embedded scripts can use xs extensions (ala 'use socket;')
EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);
EXTERN_C XS(boot_EQW);
EXTERN_C XS(boot_EQDB);
EXTERN_C XS(boot_EQDBRes);
EXTERN_C XS(boot_HTTPRequest);
EXTERN_C XS(boot_EQLConfig);

EXTERN_C void xs_init(pTHX)
{
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = '\0';

	char buf[128];	//shouldent have any function names longer than this.

	//add the strcpy stuff to get rid of const warnings....

	newXS(strcpy(buf, "DynaLoader::boot_DynaLoader"), boot_DynaLoader, file);
	newXS(strcpy(buf, "EQW::boot_EQW"), boot_EQW, file);
	newXS(strcpy(buf, "EQDB::boot_EQDB"), boot_EQDB, file);
	newXS(strcpy(buf, "EQDBRes::boot_EQDBRes"), boot_EQDBRes, file);
	newXS(strcpy(buf, "HTTPRequest::boot_HTTPRequest"), boot_HTTPRequest, file);
	newXS(strcpy(buf, "EQLConfig::boot_EQLConfig"), boot_EQLConfig, file);
	newXS(strcpy(buf, "EQWIO::PRINT"), XS_EQWIO_PRINT, file);
}

EQWParser::EQWParser() {
	//setup perl...
	my_perl = perl_alloc();
	_empty_sv = newSV(0);
	if (!my_perl) {
		LogInfo("Error: perl_alloc failed!");
	}
	else {
		DoInit();
	}
}

void EQWParser::DoInit() {
	const char *argv_eqemu[] = { "",
		"-w", "-W",
		"-e", "0;", nullptr };

	int argc = 5;

	char **argv = (char **)argv_eqemu;
	char **env = { nullptr };

	PL_perl_destruct_level = 1;

	perl_construct(my_perl);

	PERL_SYS_INIT3(&argc, &argv, &env);

	perl_parse(my_perl, xs_init, argc, argv, env);

	perl_run(my_perl);

	//a little routine we use a lot.
	eval_pv("sub my_eval {eval $_[0];}", TRUE);	//dies on error

	//ruin the perl exit and command:
	eval_pv("sub my_exit {}",TRUE);
	eval_pv("sub my_sleep {}",TRUE);
	if(gv_stashpv("CORE::GLOBAL", FALSE)) {
		GV *exitgp = gv_fetchpv("CORE::GLOBAL::exit", TRUE, SVt_PVCV);
		GvCV_set(exitgp, perl_get_cv("my_exit", TRUE));	//dies on error
		GvIMPORTED_CV_on(exitgp);
		GV *sleepgp = gv_fetchpv("CORE::GLOBAL::sleep", TRUE, SVt_PVCV);
		GvCV_set(sleepgp, perl_get_cv("my_sleep", TRUE));	//dies on error
		GvIMPORTED_CV_on(sleepgp);
	}

	//setup eval_file
	eval_pv(
	"our %Cache;"
	"use Symbol qw(delete_package);"
	"sub eval_file {"
		"my($package, $filename) = @_;"
		"$filename=~s/\'//g;"
		"if(! -r $filename) { print \"Unable to read perl file '$filename'\\n\"; return; }"
		"my $mtime = -M $filename;"
		"if(defined $Cache{$package}{mtime}&&$Cache{$package}{mtime} <= $mtime && !($package eq 'plugin')){"
		"	return;"
		"} else {"
		//we 'my' $filename,$mtime,$package,$sub to prevent them from changing our state up here.
		"	eval(\"package $package; my(\\$filename,\\$mtime,\\$package,\\$sub); \\$isloaded = 1; require '$filename'; \");"
		"}"
	"}"
	,FALSE);

	//make a tie-able class to capture IO and get it where it needs to go
	eval_pv(
		"package EQWIO; "
//			"&boot_EQEmuIO;"
			"sub TIEHANDLE { my $me = bless {}, $_[0]; $me->PRINT('Creating '.$me); return($me); } "
			"sub WRITE { } "
			"sub PRINTF { my $me = shift; my $fmt = shift; $me->PRINT(sprintf($fmt, @_)); } "
			"sub CLOSE { my $me = shift; $me->PRINT('Closing '.$me); } "
			"sub DESTROY { my $me = shift; $me->PRINT('Destroying '.$me); } "
//this ties us for all packages
		"package MAIN;"
		"	if(tied *STDOUT) { untie(*STDOUT); }"
		"	if(tied *STDERR) { untie(*STDERR); }"
		"	tie *STDOUT, 'EQWIO';"
		"	tie *STDERR, 'EQWIO';"
		,FALSE);

	eval_pv(
		"package world; "
		,FALSE
	);

	//make sure the EQW pointer is set up in this package
	EQW *curc = EQW::Singleton();
	SV *l = get_sv("world::EQW", true);
	if(curc != nullptr) {
		sv_setref_pv(l, "EQW", curc);
	} else {
		//clear out the value, mainly to get rid of blessedness
		sv_setsv(l, _empty_sv);
	}

	//make sure the EQDB pointer is set up in this package
	EQDB::SetMySQL(database.getMySQL());
	EQDB *curc_db = EQDB::Singleton();
	SV *l_db = get_sv("world::EQDB", true);
	if(curc_db != nullptr) {
		sv_setref_pv(l_db, "EQDB", curc_db);
	} else {
		//clear out the value, mainly to get rid of blessedness
		sv_setsv(l_db, _empty_sv);
	}

	//load up EQW
	eval_pv(
		"package EQW;"
		"&boot_EQW;"			//load our EQW XS
		"package EQDB;"
		"&boot_EQDB;"			//load our EQW XS
		"package EQDBRes;"
		"&boot_EQDBRes;"			//load our EQW XS
		"package HTTPRequest;"
		"&boot_HTTPRequest;"			//load our HTTPRequest XS
		"package EQLConfig;"
		"&boot_EQLConfig;"			//load our EQLConfig XS
	, FALSE );


#ifdef EMBPERL_PLUGIN
	LogInfo("Loading worldui perl plugins");
	std::string err;
	if(!eval_file("world", "worldui.pl", err)) {
		LogInfo("Warning - world.pl: [{}]", err.c_str());
	}

	eval_pv(
		"package world; "
		"if(opendir(D,'worldui')) { "
		"	my @d = readdir(D);"
		"	closedir(D);"
		"	foreach(@d){ "
		"		next unless(/\\.pl$); "
		"		require 'templates/'.$_;"
		"	}"
		"}"
	,FALSE);
#endif //EMBPERL_PLUGIN
}

EQWParser::~EQWParser() {
	//removed to try to stop perl from exploding on reload, we'll see
/*	eval_pv(
		"package quest;"
		"	untie *STDOUT;"
		"	untie *STDERR;"
	,FALSE);
*/
	perl_free(my_perl);
}

bool EQWParser::eval_file(const char * packagename, const char * filename, std::string &error)
{
	std::vector<std::string> args;
	args.push_back(packagename);
	args.push_back(filename);
	return(dosub("eval_file", args, error));
}

bool EQWParser::dosub(const char * subname, const std::vector<std::string> &args, std::string &error, int mode) {
	bool err = false;
	dSP;				// initialize stack pointer
	ENTER;				// everything created after here
	SAVETMPS;			// ...is a temporary variable
	PUSHMARK(SP);		// remember the stack pointer
	if(!args.empty())
	{
		for (auto i = args.begin(); i != args.end(); ++i) { /* push the arguments onto the perl stack */
			XPUSHs(sv_2mortal(newSVpv(i->c_str(), i->length())));
		}
	}
	PUTBACK;					// make local stack pointer global
	call_pv(subname, mode);		/*eval our code*/
	SPAGAIN;					// refresh stack pointer
	if(SvTRUE(ERRSV)) {
		err = true;
	}
	FREETMPS;		// free temp values
	LEAVE;			// ...and the XPUSHed "mortal" args.

	if(err) {
		error = "Perl runtime error: ";
		error += SvPVX(ERRSV);
		return(false);
	}
	return(true);
}

bool EQWParser::eval(const char * code, std::string &error) {
	std::vector<std::string> arg;
	arg.push_back(code);
	return(dosub("my_eval", arg, error, G_SCALAR|G_DISCARD|G_EVAL|G_KEEPERR));
}

void EQWParser::EQW_eval(const char *pkg, const char *code) {
	char namebuf[64];

	snprintf(namebuf, 64, "package %s;", pkg);
	eval_pv(namebuf, FALSE);

	//make sure the EQW pointer is set up
	EQW *curc = EQW::Singleton();
	snprintf(namebuf, 64, "EQW");
//	snprintf(namebuf, 64, "%s::EQW", pkg);
	SV *l = get_sv(namebuf, true);
	if(curc != nullptr) {
		sv_setref_pv(l, "EQW", curc);
	} else {
		//clear out the value, mainly to get rid of blessedness
		sv_setsv(l, _empty_sv);
	}
	//make sure the EQDB pointer is set up
	EQDB *curc_db = EQDB::Singleton();
	snprintf(namebuf, 64, "EQDB");
//	snprintf(namebuf, 64, "%s::EQW", pkg);
	SV *l_db = get_sv(namebuf, true);
	if(curc_db != nullptr) {
		sv_setref_pv(l_db, "EQDB", curc_db);
	} else {
		//clear out the value, mainly to get rid of blessedness
		sv_setsv(l_db, _empty_sv);
	}

	std::string err;
	if(!eval(code, err)) {
		EQW::Singleton()->AppendOutput(err.c_str());
	}
}

void EQWParser::SetHTTPRequest(const char *pkg, HTTPRequest *it) {
	char namebuf[64];

	snprintf(namebuf, 64, "package %s;", pkg);
	eval_pv(namebuf, FALSE);

	snprintf(namebuf, 64, "request");
//	snprintf(namebuf, 64, "%s::EQW", pkg);
	SV *l = get_sv(namebuf, true);
	if(it != nullptr) {
		sv_setref_pv(l, "HTTPRequest", it);
	} else {
		//clear out the value, mainly to get rid of blessedness
		sv_setsv(l, _empty_sv);
	}

}
/*
$editors = array();
$editors["merchant"] = new MerchantEditor();
#... for other editors

if(defined($editors[$editor])) {
	$edit = $editors[$editor];
	$edit->dispatch($action);
}

class MerchantEditor extends BaseEditor {
	MerchantEditor() {
		$this->RegisterAction(0, "get_merchantlist", "merchant/merchant.tmpl.php", "no");
		$this->RegisterAction(1, "get_merchantlist", "merchant/merchant.edit.tmpl.php", "no");
	}
}

function dispatch() {
	my $dispatcher = $this->_dispatchers[$action];
	$body = new Template($dispatcher["template"]);
	my $proc = $dispatcher["proc"];
	$vars = $this->$proc();
	if($dispatcher["guestmode"] == "no") {
		check_authorization();
	}
	if ($vars) {
		foreach ($vars as $key=>$value) {
			$body->set($key, $value);
		}
	}
}

*/

#endif //EMBPERL

