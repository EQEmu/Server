/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "SharedLibrary.h"
#include <stdio.h>

#ifdef _WINDOWS
	#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp

	#define EmuLibName "EMuShareMem"
#else
	#define EmuLibName "libEMuShareMem.so"

	#include "../common/unix.h"
	#include <dlfcn.h>
    #define GetProcAddress(a,b) dlsym(a,b)
	#define LoadLibrary(a) dlopen(a, RTLD_NOW) 
	#define  FreeLibrary(a) dlclose(a)
	#define GetLastError() dlerror()
#endif

SharedLibrary::SharedLibrary() {
	hDLL = NULL;
}

SharedLibrary::~SharedLibrary() {
	Unload();
}

bool SharedLibrary::Load(const char *name)
{
#ifdef _WINDOWS
	SetLastError(0);
#endif
	
	hDLL = LoadLibrary(name);
	
	if(!hDLL) {
		const char *load_error = GetError();
		fprintf(stderr, "[Error] Load Shared Library '%s' failed.  Error=%s\n", name, load_error?load_error:"Null Return, no error");
		return false;
	}
#ifdef _WINDOWS
    else { SetLastError(0); } // Clear the win9x error
#endif
	
	return(true);
}

void SharedLibrary::Unload() {
	if (hDLL != NULL) {
		FreeLibrary(hDLL);
#ifndef WIN32
		const char* error;
		if ((error = GetError()) != NULL)
			fprintf(stderr, "FreeLibrary() error = %s", error);
#endif
		hDLL = NULL;
	}
}

void *SharedLibrary::GetSym(const char *name) {
	if (!Loaded())
		return(NULL);
	
	void *r = GetProcAddress(hDLL, name);

	if(GetError() != NULL)
		r = NULL;

	return(r);
}

bool SharedLibrary::GetSym(const char *name, void **sym)
{
	bool result=false;
	if (Loaded()) {
		*sym = GetProcAddress(hDLL, name);
		result= (GetError() == NULL);
	}

	return result;
}

const char *SharedLibrary::GetError()
{
#ifdef _WINDOWS
	//not thread safe, dont care.
	static char ErrBuf[128];
	unsigned long err = GetLastError();
	if(err == 0)
		return(NULL);
	sprintf(ErrBuf, "Error #%lu", (unsigned long)err);
	return(ErrBuf);
#else
	return GetLastError();
#endif
}
