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
#ifndef _SHAREDLIBRARY_H
#define _SHAREDLIBRARY_H

#ifdef _WINDOWS
#include <windows.h>
#endif

class SharedLibrary {
public:
	SharedLibrary();
	virtual ~SharedLibrary();
	
	//two call styles for GetSym, one returns bool, other NULL for fail
	bool GetSym(const char *name, void **sym);
	void *GetSym(const char *name);
	
	const char *GetError();
	
	virtual bool Load(const char *file);
	virtual void Unload();
	
	inline bool	Loaded() { return (hDLL != 0); }
	
protected:
#ifdef _WINDOWS
	HINSTANCE hDLL;
#else
	void* hDLL;
#endif
};

#endif
