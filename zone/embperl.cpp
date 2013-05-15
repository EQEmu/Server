/*
embperl.cpp
---------------
wraps a perl interpreter for use in eqemu
Eglin
*/

#ifndef EMBPERL_CPP
#define EMBPERL_CPP

#ifdef EMBPERL

#include "../common/debug.h"
#include <cstdio>
#include <cstdarg>
#include <vector>
#include "embperl.h"
#include "embxs.h"
#include "../common/features.h"
#ifndef GvCV_set
#define GvCV_set(gv,cv) (GvCV(gv) = (cv))
#endif

#ifdef EMBPERL_XS
EXTERN_C XS(boot_quest);
#ifdef EMBPERL_XS_CLASSES
EXTERN_C XS(boot_Mob);
EXTERN_C XS(boot_NPC);
EXTERN_C XS(boot_Client);
EXTERN_C XS(boot_Corpse);
EXTERN_C XS(boot_EntityList);
EXTERN_C XS(boot_Group);
EXTERN_C XS(boot_Raid);
EXTERN_C XS(boot_QuestItem);
EXTERN_C XS(boot_HateEntry);
EXTERN_C XS(boot_Object);
EXTERN_C XS(boot_Doors);
EXTERN_C XS(boot_PerlPacket);
/*XS(XS_Client_new);
//XS(XS_Mob_new);
XS(XS_NPC_new);
//XS(XS_Corpse_new);
XS(XS_EntityList_new);
//XS(XS_Group_new);*/
#endif
#endif
#ifdef EMBPERL_COMMANDS
XS(XS_command_add);
#endif

#ifdef EMBPERL_IO_CAPTURE
XS(XS_EQEmuIO_PRINT);
#endif //EMBPERL_IO_CAPTURE

//so embedded scripts can use xs extensions (ala 'use socket;')
EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);
EXTERN_C void xs_init(pTHX)
{
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = '\0';

	char buf[128];	//shouldent have any function names longer than this.

	//add the strcpy stuff to get rid of const warnings....

	newXS(strcpy(buf, "DynaLoader::boot_DynaLoader"), boot_DynaLoader, file);
	newXS(strcpy(buf, "quest::boot_qc"), boot_qc, file);
#ifdef EMBPERL_XS
	newXS(strcpy(buf, "quest::boot_quest"), boot_quest, file);
#ifdef EMBPERL_XS_CLASSES
	newXS(strcpy(buf, "Mob::boot_Mob"), boot_Mob, file);
	newXS(strcpy(buf, "NPC::boot_Mob"), boot_Mob, file);
	newXS(strcpy(buf, "NPC::boot_NPC"), boot_NPC, file);
///	newXS(strcpy(buf, "NPC::new"), XS_NPC_new, file);
	newXS(strcpy(buf, "Corpse::boot_Mob"), boot_Mob, file);
	newXS(strcpy(buf, "Corpse::boot_Corpse"), boot_Corpse, file);
	newXS(strcpy(buf, "Client::boot_Mob"), boot_Mob, file);
	newXS(strcpy(buf, "Client::boot_Client"), boot_Client, file);
//	newXS(strcpy(buf, "Client::new"), XS_Client_new, file);
	newXS(strcpy(buf, "EntityList::boot_EntityList"), boot_EntityList, file);
//	newXS(strcpy(buf, "EntityList::new"), XS_EntityList_new, file);
	newXS(strcpy(buf, "PerlPacket::boot_PerlPacket"), boot_PerlPacket, file);
	newXS(strcpy(buf, "Group::boot_Group"), boot_Group, file);
	newXS(strcpy(buf, "Raid::boot_Raid"), boot_Raid, file);
	newXS(strcpy(buf, "QuestItem::boot_QuestItem"), boot_QuestItem, file);
	newXS(strcpy(buf, "HateEntry::boot_HateEntry"), boot_HateEntry, file);
	newXS(strcpy(buf, "Object::boot_Object"), boot_Object, file);
	newXS(strcpy(buf, "Doors::boot_Doors"), boot_Doors, file);
;
#endif
#endif
#ifdef EMBPERL_COMMANDS
	newXS(strcpy(buf, "commands::command_add"), XS_command_add, file);
#endif
#ifdef EMBPERL_IO_CAPTURE
	newXS(strcpy(buf, "EQEmuIO::PRINT"), XS_EQEmuIO_PRINT, file);
#endif
}

Embperl::Embperl()
{
	in_use = true;	//in case one of these files generates an event

	//setup perl...
	my_perl = perl_alloc();
	if(!my_perl)
		throw "Failed to init Perl (perl_alloc)";
	DoInit();
}

void Embperl::DoInit() {
	const char *argv_eqemu[] = { "",
#ifdef EMBPERL_IO_CAPTURE
		"-w", "-W",
#endif
		"-e", "0;", nullptr };

	int argc = 3;
#ifdef EMBPERL_IO_CAPTURE
	argc = 5;
#endif

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

	//declare our file eval routine.
	try {
		init_eval_file();
	}
	catch(const char *err)
	{
		//remember... lasterr() is no good if we crap out here, in construction
		LogFile->write(EQEMuLog::Quest, "perl error: %s", err);
		throw "failed to install eval_file hook";
	}

#ifdef EMBPERL_IO_CAPTURE
	LogFile->write(EQEMuLog::Quest, "Tying perl output to eqemu logs");
	//make a tieable class to capture IO and pass it into EQEMuLog
	eval_pv(
		"package EQEmuIO; "
//			"&boot_EQEmuIO;"
			"sub TIEHANDLE { my $me = bless {}, $_[0]; $me->PRINT('Creating '.$me); return($me); } "
			"sub WRITE { } "
			//dunno why I need to shift off fmt here, but it dosent like without it
			"sub PRINTF { my $me = shift; my $fmt = shift; $me->PRINT(sprintf($fmt, @_)); } "
			"sub CLOSE { my $me = shift; $me->PRINT('Closing '.$me); } "
			"sub DESTROY { my $me = shift; $me->PRINT('Destroying '.$me); } "
//this ties us for all packages, just do it in quest since thats kinda 'our' package
		"package quest;"
		"	if(tied *STDOUT) { untie(*STDOUT); }"
		"	if(tied *STDERR) { untie(*STDERR); }"
		"	tie *STDOUT, 'EQEmuIO';"
		"	tie *STDERR, 'EQEmuIO';"
		,FALSE);
#endif //EMBPERL_IO_CAPTURE

#ifdef EMBPERL_PLUGIN
	eval_pv(
		"package plugin; "
		,FALSE
	);
#ifdef EMBPERL_EVAL_COMMANDS
	try {
		eval_pv(
			"use IO::Scalar;"
			"$plugin::printbuff='';"
			"tie *PLUGIN,'IO::Scalar',\\$plugin::printbuff;"
		,FALSE);
	}
	catch(const char *err) {
		throw "failed to install plugin printhook, do you lack IO::Scalar?";
	}
#endif

	LogFile->write(EQEMuLog::Quest, "Loading perlemb plugins.");
	try
	{
		eval_pv("main::eval_file('plugin', 'plugin.pl');", FALSE);
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "Warning - plugin.pl: %s", err);
	}
	try
	{
		//should probably read the directory in c, instead, so that
		//I can echo filenames as I do it, but c'mon... I'm lazy and this 1 line reads in all the plugins
		eval_pv(
			"if(opendir(D,'plugins')) { "
			"	my @d = readdir(D);"
			"	closedir(D);"
			"	foreach(@d){ "
			"		main::eval_file('plugin','plugins/'.$_)if/\\.pl$/;"
			"	}"
			"}"
		,FALSE);
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "Perl warning: %s", err);
	}
#endif //EMBPERL_PLUGIN
#ifdef EMBPERL_COMMANDS
	LogFile->write(EQEMuLog::Quest, "Loading perl commands...");
	try
	{
		eval_pv(
			"package commands;"
			"main::eval_file('commands', 'commands.pl');"
			"&commands::commands_init();"
		, FALSE);
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "Warning - commands.pl: %s", err);
	}
#endif //EMBPERL_COMMANDS
	in_use = false;
}

Embperl::~Embperl()
{
	in_use = true;
#ifdef EMBPERL_IO_CAPTURE
	eval_pv(
		"package quest;"
		"	if(tied *STDOUT) { untie(*STDOUT); }"
		"	if(tied *STDERR) { untie(*STDERR); }"
		,FALSE);
#endif
	perl_free(my_perl);
}

void Embperl::Reinit() {
	in_use = true;
	PL_perl_destruct_level = 1;
	perl_destruct(my_perl);
	DoInit();
	in_use = false;
}

void Embperl::init_eval_file(void)
{
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
/*				"local *FH;open FH, $filename or die \"open '$filename' $!\";"
				"local($/) = undef;my $sub = <FH>;close FH;"
				"my $eval = qq{package $package; sub handler { $sub; }};"
				"{ my($filename,$mtime,$package,$sub); eval $eval; }"
				"die $@ if $@;"
				"$Cache{$package}{mtime} = $mtime; ${$package.'::isloaded'} = 1;}"
*/
			"}"
		"}"
		,FALSE);
 }

void Embperl::eval_file(const char * packagename, const char * filename)
{
	std::vector<std::string> args;
	args.push_back(packagename);
	args.push_back(filename);
	dosub("eval_file", &args);
}

void Embperl::dosub(const char * subname, const std::vector<std::string> * args, int mode)
{//as seen in perlembed docs
#if EQDEBUG >= 5
	if(InUse()) {
		LogFile->write(EQEMuLog::Debug, "Warning: Perl dosub called for %s when perl is allready in use.\n", subname);
	}
#endif
	in_use = true;
	bool err = false;
	dSP;							/* initialize stack pointer */
	ENTER;							/* everything created after here */
	SAVETMPS;						/* ...is a temporary variable. */
	PUSHMARK(SP);					/* remember the stack pointer */
	if(args && args->size())
	{
		for(std::vector<std::string>::const_iterator i = args->begin(); i != args->end(); ++i)
		{/* push the arguments onto the perl stack */
			XPUSHs(sv_2mortal(newSVpv(i->c_str(), i->length())));
		}
	}
	PUTBACK;				/* make local stack pointer global */
	call_pv(subname, mode); /*eval our code*/
	SPAGAIN;				/* refresh stack pointer */
	if(SvTRUE(ERRSV))
	{
		err = true;
	}
	FREETMPS;			/* free temp values */
	LEAVE;				/* ...and the XPUSHed "mortal" args.*/

	in_use = false;
	if(err)
	{
		errmsg = "Perl runtime error: ";
		errmsg += SvPVX(ERRSV);
		throw errmsg.c_str();
	}
}

//evaluate an expression. throw error on fail
void Embperl::eval(const char * code)
{
	std::vector<std::string> arg;
	arg.push_back(code);
// MYRA - added EVAL & KEEPERR to eval per Eglin's recommendation
	dosub("my_eval", &arg, G_SCALAR|G_DISCARD|G_EVAL|G_KEEPERR);
//end Myra
}

bool Embperl::SubExists(const char *package, const char *sub) {
	HV *stash = gv_stashpv(package, false);
	if(!stash)
		return(false);
	int len = strlen(sub);
	return(hv_exists(stash, sub, len));
}

bool Embperl::VarExists(const char *package, const char *var) {
	HV *stash = gv_stashpv(package, false);
	if(!stash)
		return(false);
	int len = strlen(var);
	return(hv_exists(stash, var, len));
}


#endif //EMBPERL

#endif //EMBPERL_CPP
