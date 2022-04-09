#pragma once

// Defining PERLBIND_STRICT_NUMERIC_TYPES will enable strict type checks
// for integers and floats.This is required for overloads that depend on
// int and float type differences.
// #define PERLBIND_STRICT_NUMERIC_TYPES

// Defining PERLBIND_NO_STRICT_SCALAR_TYPES will disable strict type checks
// for all int, float, and string function arguments. These types will only
// be checked for scalar validity and converted to the function's expected
// paramter type. This will break overloads that depend on distinct types.
// This option overrides PERLBIND_STRICT_NUMERIC_TYPES.
//#define PERLBIND_NO_STRICT_SCALAR_TYPES

// defining PERL_NO_GET_CONTEXT gets context from local variable "my_perl"
// instead of calling Perl_get_context() in macros
#define PERL_NO_GET_CONTEXT

#define WIN32IO_IS_STDIO
#if _MSC_VER
#define __inline__ __inline
// perl 5.30+ defines HAS_BUILTIN_EXPECT for msvc which breaks builds
#define __builtin_expect(expr,val) (expr)
// avoid INT64_C and UINT64_C redefinition warnings
#if PERL_VERSION < 28
#include <cstdint>
#endif
#endif

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

// short name perl macros that cause issues
#undef Move
#undef Copy
#undef Zero
#undef list
#undef seed
#undef do_open
#undef do_close

#include <perlbind/version.h>
#include <perlbind/forward.h>
#include <perlbind/util.h>
#include <perlbind/traits.h>
#include <perlbind/hash.h>
#include <perlbind/typemap.h>
#include <perlbind/scalar.h>
#include <perlbind/array.h>
#include <perlbind/stack.h>
#include <perlbind/subcaller.h>
#include <perlbind/function.h>
#include <perlbind/package.h>
#include <perlbind/interpreter.h>
