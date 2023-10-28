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

#include <perlbind/perlbind.h>
namespace perl = perlbind;

#undef Null

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

	//evaluate an expression. throws string errors on fail
	int eval(const char * code);
	//execute a subroutine. throws lasterr on failure
	int dosub(const char * subname, const std::vector<std::string> * args = nullptr, int mode = G_SCALAR|G_EVAL);

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
	// put a pointer into a blessed perl variable
	void setptr(const char* varname, const char* classname, void* val) const {
		SV* t = get_sv(varname, GV_ADD);
		sv_setref_pv(t, classname, val);
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

	//check to see if a sub exists in package
	bool SubExists(const char *package, const char *sub);
};
#endif //EMBPERL

#endif //EMBPERL_H
