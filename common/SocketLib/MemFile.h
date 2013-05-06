/** \file MemFile.h
 **	\date  2005-04-25
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004,2005  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _MEMFILE_H
#define _MEMFILE_H

#include <map>
#include "IFile.h"

#define BLOCKSIZE 32768

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


/** Implements a memory file. 
	\ingroup file */
class MemFile : public IFile
{
public:
	/** File block structure. 
		\ingroup file */
	struct block_t {
		block_t() : next(nullptr) {}
		struct block_t *next;
		char data[BLOCKSIZE];
	};
public:
	MemFile();
	MemFile(const std::string& path);
	~MemFile();

	bool fopen(const std::string& path, const std::string& mode);
	void fclose();

	size_t fread(char *ptr, size_t size, size_t nmemb);
	size_t fwrite(const char *ptr, size_t size, size_t nmemb);

	char *fgets(char *s, int size);
	void fprintf(char *format, ...);

	off_t size();
	bool eof();

private:
	MemFile(const MemFile& ) {} // copy constructor
	MemFile& operator=(const MemFile& ) { return *this; } // assignment operator

static	std::map<std::string,block_t *> m_files;
	std::string m_path;
	bool m_temporary;
	block_t *m_base;
	block_t *m_current_read;
	block_t *m_current_write;
	size_t m_read_ptr;
	size_t m_write_ptr;
};




#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _MEMFILE_H
