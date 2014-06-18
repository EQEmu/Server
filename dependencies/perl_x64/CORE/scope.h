/*    scope.h
 *
 *    Copyright (C) 1993, 1994, 1996, 1997, 1998, 1999, 2000, 2001,
 *    2002, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#define SAVEt_ITEM		0
#define SAVEt_SV		1
#define SAVEt_AV		2
#define SAVEt_HV		3
#define SAVEt_INT		4
#define SAVEt_LONG		5
#define SAVEt_I32		6
#define SAVEt_IV		7
#define SAVEt_SPTR		8
#define SAVEt_APTR		9
#define SAVEt_HPTR		10
#define SAVEt_PPTR		11
#define SAVEt_NSTAB		12
#define SAVEt_SVREF		13
#define SAVEt_GP		14
#define SAVEt_FREESV		15
#define SAVEt_FREEOP		16
#define SAVEt_FREEPV		17
#define SAVEt_CLEARSV		18
#define SAVEt_DELETE		19
#define SAVEt_DESTRUCTOR	20
#define SAVEt_REGCONTEXT	21
#define SAVEt_STACK_POS		22
#define SAVEt_I16		23
#define SAVEt_AELEM		24
#define SAVEt_HELEM		25
#define SAVEt_OP		26
#define SAVEt_HINTS		27
#define SAVEt_ALLOC		28
#define SAVEt_GENERIC_SVREF	29
#define SAVEt_DESTRUCTOR_X	30
#define SAVEt_VPTR		31
#define SAVEt_I8		32
#define SAVEt_COMPPAD		33
#define SAVEt_GENERIC_PVREF	34
#define SAVEt_PADSV_AND_MORTALIZE	35
#define SAVEt_MORTALIZESV	36
#define SAVEt_SHARED_PVREF	37
#define SAVEt_BOOL		38
#define SAVEt_SET_SVFLAGS	39
#define SAVEt_SAVESWITCHSTACK	40
#define SAVEt_COP_ARYBASE	41
#define SAVEt_RE_STATE		42
#define SAVEt_COMPILE_WARNINGS	43
#define SAVEt_STACK_CXPOS	44
#define SAVEt_PARSER		45
#define SAVEt_ADELETE		46
#define SAVEt_I32_SMALL		47
#define SAVEt_INT_SMALL		48
#define SAVEt_GVSV		49
#define SAVEt_FREECOPHH		50

#define SAVEf_SETMAGIC		1
#define SAVEf_KEEPOLDELEM	2

#define SAVE_TIGHT_SHIFT	6
#define SAVE_MASK		0x3F

#define save_aelem(av,idx,sptr)	save_aelem_flags(av,idx,sptr,SAVEf_SETMAGIC)
#define save_helem(hv,key,sptr)	save_helem_flags(hv,key,sptr,SAVEf_SETMAGIC)

#ifndef SCOPE_SAVES_SIGNAL_MASK
#define SCOPE_SAVES_SIGNAL_MASK 0
#endif

#define SSCHECK(need) if (PL_savestack_ix + (I32)(need) > PL_savestack_max) savestack_grow()
#define SSGROW(need) if (PL_savestack_ix + (I32)(need) > PL_savestack_max) savestack_grow_cnt(need)
#define SSPUSHINT(i) (PL_savestack[PL_savestack_ix++].any_i32 = (I32)(i))
#define SSPUSHLONG(i) (PL_savestack[PL_savestack_ix++].any_long = (long)(i))
#define SSPUSHBOOL(p) (PL_savestack[PL_savestack_ix++].any_bool = (p))
#define SSPUSHIV(i) (PL_savestack[PL_savestack_ix++].any_iv = (IV)(i))
#define SSPUSHUV(u) (PL_savestack[PL_savestack_ix++].any_uv = (UV)(u))
#define SSPUSHPTR(p) (PL_savestack[PL_savestack_ix++].any_ptr = (void*)(p))
#define SSPUSHDPTR(p) (PL_savestack[PL_savestack_ix++].any_dptr = (p))
#define SSPUSHDXPTR(p) (PL_savestack[PL_savestack_ix++].any_dxptr = (p))
#define SSPOPINT (PL_savestack[--PL_savestack_ix].any_i32)
#define SSPOPLONG (PL_savestack[--PL_savestack_ix].any_long)
#define SSPOPBOOL (PL_savestack[--PL_savestack_ix].any_bool)
#define SSPOPIV (PL_savestack[--PL_savestack_ix].any_iv)
#define SSPOPUV (PL_savestack[--PL_savestack_ix].any_uv)
#define SSPOPPTR (PL_savestack[--PL_savestack_ix].any_ptr)
#define SSPOPDPTR (PL_savestack[--PL_savestack_ix].any_dptr)
#define SSPOPDXPTR (PL_savestack[--PL_savestack_ix].any_dxptr)

/*
=head1 Callback Functions

=for apidoc Ams||SAVETMPS
Opening bracket for temporaries on a callback.  See C<FREETMPS> and
L<perlcall>.

=for apidoc Ams||FREETMPS
Closing bracket for temporaries on a callback.  See C<SAVETMPS> and
L<perlcall>.

=for apidoc Ams||ENTER
Opening bracket on a callback.  See C<LEAVE> and L<perlcall>.

=for apidoc Ams||LEAVE
Closing bracket on a callback.  See C<ENTER> and L<perlcall>.

=over

=item ENTER_with_name(name)

Same as C<ENTER>, but when debugging is enabled it also associates the
given literal string with the new scope.

=item LEAVE_with_name(name)

Same as C<LEAVE>, but when debugging is enabled it first checks that the
scope has the given name. Name must be a literal string.

=back

=cut
*/

#define SAVETMPS save_int((int*)&PL_tmps_floor), PL_tmps_floor = PL_tmps_ix
#define FREETMPS if (PL_tmps_ix > PL_tmps_floor) free_tmps()

#ifdef DEBUGGING
#define ENTER							\
    STMT_START {						\
	push_scope();						\
	DEBUG_SCOPE("ENTER")					\
    } STMT_END
#define LEAVE							\
    STMT_START {						\
	DEBUG_SCOPE("LEAVE")					\
	pop_scope();						\
    } STMT_END
#define ENTER_with_name(name)						\
    STMT_START {							\
	push_scope();							\
	if (PL_scopestack_name)						\
	    PL_scopestack_name[PL_scopestack_ix-1] = name;		\
	DEBUG_SCOPE("ENTER \"" name "\"")				\
    } STMT_END
#define LEAVE_with_name(name)						\
    STMT_START {							\
	DEBUG_SCOPE("LEAVE \"" name "\"")				\
	if (PL_scopestack_name)	{					\
	    assert(((char*)PL_scopestack_name[PL_scopestack_ix-1]	\
			== (char*)name)					\
		    || strEQ(PL_scopestack_name[PL_scopestack_ix-1], name));        \
	}								\
	pop_scope();							\
    } STMT_END
#else
#define ENTER push_scope()
#define LEAVE pop_scope()
#define ENTER_with_name(name) ENTER
#define LEAVE_with_name(name) LEAVE
#endif
#define LEAVE_SCOPE(old) if (PL_savestack_ix > old) leave_scope(old)

#define SAVEI8(i)	save_I8((I8*)&(i))
#define SAVEI16(i)	save_I16((I16*)&(i))
#define SAVEI32(i)	save_I32((I32*)&(i))
#define SAVEINT(i)	save_int((int*)&(i))
#define SAVEIV(i)	save_iv((IV*)&(i))
#define SAVELONG(l)	save_long((long*)&(l))
#define SAVEBOOL(b)	save_bool(&(b))
#define SAVESPTR(s)	save_sptr((SV**)&(s))
#define SAVEPPTR(s)	save_pptr((char**)&(s))
#define SAVEVPTR(s)	save_vptr((void*)&(s))
#define SAVEPADSVANDMORTALIZE(s)	save_padsv_and_mortalize(s)
#define SAVEFREESV(s)	save_freesv(MUTABLE_SV(s))
#define SAVEMORTALIZESV(s)	save_mortalizesv(MUTABLE_SV(s))
#define SAVEFREEOP(o)	save_freeop((OP*)(o))
#define SAVEFREEPV(p)	save_freepv((char*)(p))
#define SAVECLEARSV(sv)	save_clearsv((SV**)&(sv))
#define SAVEGENERICSV(s)	save_generic_svref((SV**)&(s))
#define SAVEGENERICPV(s)	save_generic_pvref((char**)&(s))
#define SAVESHAREDPV(s)		save_shared_pvref((char**)&(s))
#define SAVESETSVFLAGS(sv,mask,val)	save_set_svflags(sv,mask,val)
#define SAVEFREECOPHH(h)	save_pushptr((void *)(h), SAVEt_FREECOPHH)
#define SAVEDELETE(h,k,l) \
	  save_delete(MUTABLE_HV(h), (char*)(k), (I32)(l))
#define SAVEHDELETE(h,s) \
	  save_hdelete(MUTABLE_HV(h), (s))
#define SAVEADELETE(a,k) \
	  save_adelete(MUTABLE_AV(a), (I32)(k))
#define SAVEDESTRUCTOR(f,p) \
	  save_destructor((DESTRUCTORFUNC_NOCONTEXT_t)(f), (void*)(p))

#define SAVEDESTRUCTOR_X(f,p) \
	  save_destructor_x((DESTRUCTORFUNC_t)(f), (void*)(p))

#define SAVESTACK_POS() \
    STMT_START {				\
	SSCHECK(2);				\
	SSPUSHINT(PL_stack_sp - PL_stack_base);	\
	SSPUSHUV(SAVEt_STACK_POS);		\
    } STMT_END

#define SAVEOP()	save_op()

#define SAVEHINTS()	save_hints()

#define SAVECOMPPAD() save_pushptr(MUTABLE_SV(PL_comppad), SAVEt_COMPPAD)

#define SAVESWITCHSTACK(f,t) \
    STMT_START {					\
	save_pushptrptr(MUTABLE_SV(f), MUTABLE_SV(t), SAVEt_SAVESWITCHSTACK); \
	SWITCHSTACK((f),(t));				\
	PL_curstackinfo->si_stack = (t);		\
    } STMT_END

#define SAVECOPARYBASE(c) save_pushi32ptr(CopARYBASE_get(c), c, SAVEt_COP_ARYBASE);

/* Need to do the cop warnings like this, rather than a "SAVEFREESHAREDPV",
   because realloc() means that the value can actually change. Possibly
   could have done savefreesharedpvREF, but this way actually seems cleaner,
   as it simplifies the code that does the saves, and reduces the load on the
   save stack.  */
#define SAVECOMPILEWARNINGS() save_pushptr(PL_compiling.cop_warnings, SAVEt_COMPILE_WARNINGS)

#define SAVESTACK_CXPOS() \
    STMT_START {                                  \
        SSCHECK(3);                               \
        SSPUSHINT(cxstack[cxstack_ix].blk_oldsp); \
        SSPUSHINT(cxstack_ix);                    \
        SSPUSHUV(SAVEt_STACK_CXPOS);              \
    } STMT_END

#define SAVEPARSER(p) save_pushptr((p), SAVEt_PARSER)

#ifdef USE_ITHREADS
#  define SAVECOPSTASH(c)	SAVEPPTR(CopSTASHPV(c))
#  define SAVECOPSTASH_FREE(c)	SAVESHAREDPV(CopSTASHPV(c))
#  define SAVECOPFILE(c)	SAVEPPTR(CopFILE(c))
#  define SAVECOPFILE_FREE(c)	SAVESHAREDPV(CopFILE(c))
#else
#  define SAVECOPSTASH(c)	SAVESPTR(CopSTASH(c))
#  define SAVECOPSTASH_FREE(c)	SAVECOPSTASH(c)	/* XXX not refcounted */
#  define SAVECOPFILE(c)	SAVESPTR(CopFILEGV(c))
#  define SAVECOPFILE_FREE(c)	SAVEGENERICSV(CopFILEGV(c))
#endif

#define SAVECOPLINE(c)		SAVEI32(CopLINE(c))

/* SSNEW() temporarily allocates a specified number of bytes of data on the
 * savestack.  It returns an integer index into the savestack, because a
 * pointer would get broken if the savestack is moved on reallocation.
 * SSNEWa() works like SSNEW(), but also aligns the data to the specified
 * number of bytes.  MEM_ALIGNBYTES is perhaps the most useful.  The
 * alignment will be preserved through savestack reallocation *only* if
 * realloc returns data aligned to a size divisible by "align"!
 *
 * SSPTR() converts the index returned by SSNEW/SSNEWa() into a pointer.
 */

#define SSNEW(size)             Perl_save_alloc(aTHX_ (size), 0)
#define SSNEWt(n,t)             SSNEW((n)*sizeof(t))
#define SSNEWa(size,align)	Perl_save_alloc(aTHX_ (size), \
    (I32)(align - ((size_t)((caddr_t)&PL_savestack[PL_savestack_ix]) % align)) % align)
#define SSNEWat(n,t,align)	SSNEWa((n)*sizeof(t), align)

#define SSPTR(off,type)         ((type)  ((char*)PL_savestack + off))
#define SSPTRt(off,type)        ((type*) ((char*)PL_savestack + off))

#define save_freesv(op)		save_pushptr((void *)(op), SAVEt_FREESV)
#define save_mortalizesv(op)	save_pushptr((void *)(op), SAVEt_MORTALIZESV)
#define save_freeop(op)		save_pushptr((void *)(op), SAVEt_FREEOP)
#define save_freepv(pv)		save_pushptr((void *)(pv), SAVEt_FREEPV)
#define save_op()		save_pushptr((void *)(PL_op), SAVEt_OP)

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
