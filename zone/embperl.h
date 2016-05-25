/*
Embperl.h
---------------
eqemu perl wrapper
Eglin
*/

#ifndef EMBPERL_H
#define EMBPERL_H

#ifdef EMBPERL

#include "zone_config.h"

#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>

//headers from the Perl distribution
#include <EXTERN.h>
#define WIN32IO_IS_STDIO

#ifndef WIN32
extern "C" {	//the perl headers dont do this for us...
#endif
#include <perl.h>
#include <XSUB.h>
#ifndef WIN32
};
#endif
#ifdef WIN32
#define snprintf _snprintf
#endif

//perl defines these macros and dosent clean them up, lazy bastards. -- I hate them too!
#ifdef Copy
#undef Copy
#endif

#ifdef list
#undef list
#endif

#ifdef write
#undef write
#endif

#ifdef bool
#undef bool
#endif

#ifdef Zero
#undef Zero
#endif
//These need to be cleaned up on FreeBSD

#ifdef __FreeBSD__
#ifdef do_open
#undef do_open
#endif

#ifdef do_close
#undef do_close
#endif
#endif

//so embedded scripts can use xs extensions (ala 'use socket;')
EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);
EXTERN_C void xs_init(pTHX);

extern const ZoneConfig *Config;
class Embperl
{
private:
	//if we fail inside a script evaluation, this will hold the croak msg (not much help if we die during construction, but that's our own fault)
	mutable std::string errmsg;
	//kludgy workaround for the fact that we can't directly do something like SvIV(get_sv($big[0]{ass}->{struct}))
	SV * my_get_sv(const char * varname) {
		char buffer[256];
		snprintf(buffer, 256, "if(defined(%s)) { $scratch::temp = %s; } else { $scratch::temp = 'UNDEF'; }", varname, varname);
		eval(buffer);
		return get_sv("scratch::temp", false);
	}

	//install a perl func
	void init_eval_file(void);

	bool in_use;	//true if perl is executing
protected:
	//the embedded interpreter
	PerlInterpreter * my_perl;

	void DoInit();

public:
	Embperl(void); //This can throw errors! Buyer beware
	~Embperl(void);

	void Reinit();

	//return the last error msg
	std::string lasterr(void) const { return errmsg;};
	//evaluate an expression. throws string errors on fail
	int eval(const char * code);
	//execute a subroutine. throws lasterr on failure
	int dosub(const char * subname, const std::vector<std::string> * args = nullptr, int mode = G_SCALAR|G_EVAL);

	//Access to perl variables
	//all varnames here should be of the form package::name
	//returns the contents of the perl variable named in varname as a c int
	int geti(const char * varname) { return static_cast<int>(SvIV(my_get_sv(varname))); };
	//returns the contents of the perl variable named in varname as a c float
	float getd(const char * varname) { return static_cast<float>(SvNV(my_get_sv(varname)));};
	//returns the contents of the perl variable named in varname as a string
	std::string getstr(const char * varname) {
		SV * temp = my_get_sv(varname);
		return std::string(SvPV_nolen(temp),SvLEN(temp));
	}

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

	// put key-value pairs in hash
	void sethash(const char *varname, std::map<std::string,std::string> &vals)
	{
		std::map<std::string,std::string>::iterator it;

		// Get hash and clear it.
		HV *hv = get_hv(varname, TRUE);
		hv_clear(hv);

		// Iterate through key-value pairs, storing them in hash
		for (it = vals.begin(); it != vals.end(); ++it)
		{
			int keylen = static_cast<int>(it->first.length());

			SV *val = newSVpv(it->second.c_str(), it->second.length());

			// If val was not added to hash, reset reference count
			if (hv_store(hv, it->first.c_str(), keylen, val, 0) == nullptr)
				val->sv_refcnt = 0;
		}
	}

	//loads a file and compiles it into our interpreter (assuming it hasn't already been read in)
	//idea borrowed from perlembed
	int eval_file(const char * packagename, const char * filename);

	inline bool InUse() const { return(in_use); }

	//check to see if a sub exists in package
	bool SubExists(const char *package, const char *sub);

	//check to see if a variable exists in package
	bool VarExists(const char *package, const char *var);
};
#endif //EMBPERL

#endif //EMBPERL_H
