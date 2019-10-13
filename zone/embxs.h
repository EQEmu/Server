#ifndef EMBXS_H
#define EMBXS_H

//headers from the Perl distribution
#include <EXTERN.h>
#define WIN32IO_IS_STDIO

#ifndef WIN32
extern "C" {	//the perl headers dont do this for us...
#endif
#if _MSC_VER
#define __inline__ __inline
#define __builtin_expect 
#endif
#include <perl.h>
#include <XSUB.h>
#ifndef WIN32
};
#endif

const char *getItemName(unsigned itemid);
XS(XS_qc_getItemName); /* prototype to pass -Wmissing-prototypes */
EXTERN_C XS(boot_qc); /* prototype to pass -Wmissing-prototypes */
#endif // EMBXS_H
