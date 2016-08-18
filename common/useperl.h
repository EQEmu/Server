#ifndef EMU_PERL_H_
#define EMU_PERL_H_


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
#ifndef snprintf
#define snprintf _snprintf
#endif
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

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
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

#endif /*EMU_PERL_H_*/

