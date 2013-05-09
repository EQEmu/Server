// ---
//
// $Id: missing.cpp,v 1.4 2008/07/15 20:33:31 hartwork Exp $
//
// CppTest - A C++ Unit Testing Framework
// Copyright (c) 2003 Niklas Lundell
//
// ---
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// ---

#if (defined(__WIN32__) || defined(WIN32))
# include "winconfig.h"
#else
# include "config.h"
#endif

#include "missing.h"

#ifdef HAVE_GETTICKCOUNT
	#include <windows.h>
#endif

#ifndef __BORLANDC__
	#include <cmath>
#else
	#include <math.h>
#endif

#include <cassert>
#include <ctime>

namespace Test
{
#ifndef HAVE_GETTIMEOFDAY

	int
	gettimeofday(timeval* tv, void*)
	{
		assert(tv);

	#ifdef HAVE_GETTICKCOUNT
		long now = GetTickCount();
		tv->tv_sec  = now / 1000;
		tv->tv_usec = (now % 1000) * 1000;
	#else
		tv->tv_sec  = time(0);
		tv->tv_usec = 0;
	#endif // #ifdef HAVE_GETTICKCOUNT

		return 0;
	}

#endif // #ifndef HAVE_GETTIMEOFDAY

#ifndef HAVE_ROUND
	double
	round(double d)
	{
		return d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5);
	}
#endif

} // namespace Test

