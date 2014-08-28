/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef EQEMU_ENCRYPTION_H
#define EQEMU_ENCRYPTION_H

#ifdef WIN32
#include <windows.h>
#include <string>

typedef char*(*DLLFUNC_DecryptUsernamePassword)(const char*, unsigned int, int);
typedef char*(*DLLFUNC_Encrypt)(const char*, unsigned int, unsigned int&);
typedef void(*DLLFUNC_HeapDelete)(char*);

/**
* Basic windows encryption plugin.
* Handles the managment of the plugin.
*/
class Encryption
{
public:
	/**
	* Constructor, sets all member pointers to nullptr.
	*/
	Encryption() : h_dll(nullptr), encrypt_func(nullptr), decrypt_func(nullptr), delete_func(nullptr) { };

	/**
	* Destructor, if it's loaded it unloads this library.
	*/
	~Encryption() { if(Loaded()) { Unload(); } }

	/**
	* Returns true if the dll is loaded, otherwise false.
	*/
	inline bool	Loaded() { return (h_dll != nullptr); }

	/**
	* Loads the plugin.
	* True if there are no errors, false if there was an error.
	*/
	bool LoadCrypto(std::string name);

	/**
	* Wrapper around the plugin's decrypt function.
	*/
	char* DecryptUsernamePassword(const char* encryptedBuffer, unsigned int bufferSize, int mode);

	/**
	* Wrapper around the plugin's encrypt function.
	*/
	char* Encrypt(const char* buffer, unsigned int bufferSize, unsigned int &outSize);

	/**
	* Wrapper around the plugin's delete function.
	*/
	void DeleteHeap(char* buffer);

private:
	/**
	* Loads the named dll into memory.
	* Returns true if there were no errors, otherwise return false.
	*/
	bool Load(const char *name);

	/**
	* Frees the dll from memory if it's loaded.
	*/
	void Unload();

	/**
	* Similar in function to *sym = GetProcAddress(h_dll, name).
	* Returns true if there were no errors, false otherwise.
	*/
	bool GetSym(const char *name, void **sym);

	/**
	* Similar in function to return GetProcAddress(h_dll, name).
	* Returns a pointer to the function if it is found, null on an error.
	*/
	void *GetSym(const char *name);

	HINSTANCE h_dll;
	DLLFUNC_Encrypt encrypt_func;
	DLLFUNC_DecryptUsernamePassword decrypt_func;
	DLLFUNC_HeapDelete delete_func;
};

#endif
#endif

