/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "eqemu_error.h"
#include "linked_list.h"
#include "mutex.h"
#include "misc_functions.h"
#include <stdio.h>
#include <string.h>
#ifdef _WINDOWS
	#include <conio.h>
#endif

void UpdateWindowTitle(char* iNewTitle = 0);
void CatchSignal(int sig_num);

const char* EQEMuErrorText[EQEMuError_MaxErrorID] = { "ErrorID# 0, No Error",
	"MySQL Error #1405 or #2001 means your mysql server rejected the username and password you presented it.",
	"MySQL Error #2003 means you were unable to connect to the mysql server.",
	"MySQL Error #2005 means you there are too many connections on the mysql server. The server is overloaded.",
	"MySQL Error #2007 means you the server is out of memory. The server is overloaded.",
	};

LinkedList<char*>* EQEMuErrorList;
Mutex* MEQEMuErrorList;
AutoDelete< LinkedList<char*> > ADEQEMuErrorList(&EQEMuErrorList);
AutoDelete<Mutex> ADMEQEMuErrorList(&MEQEMuErrorList);

const char* GetErrorText(uint32 iError) {
	if (iError >= EQEMuError_MaxErrorID)
		return "ErrorID# out of range";
	else
		return EQEMuErrorText[iError];
}

void AddEQEMuError(eEQEMuError iError, bool iExitNow) {
	if (!iError)
		return;
	if (!EQEMuErrorList) {
		EQEMuErrorList = new LinkedList<char*>;
		MEQEMuErrorList = new Mutex;
	}
	LockMutex lock(MEQEMuErrorList);

	LinkedListIterator<char*> iterator(*EQEMuErrorList);
	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()[0] == 1) {
//Umm... this gets a big WTF...
//			if (*((uint32*) iterator.GetData()[1]) == iError)
//not sure whats going on, using a character as a pointer....
			if (*((eEQEMuError*) &(iterator.GetData()[1])) == iError)
				return;
		}
		iterator.Advance();
	}

	char* tmp = new char[6];
	tmp[0] = 1;
	tmp[5] = 0;
	*((uint32*) &tmp[1]) = iError;
	EQEMuErrorList->Append(tmp);

	if (iExitNow)
		CatchSignal(2);
}

void AddEQEMuError(char* iError, bool iExitNow) {
	if (!iError)
		return;
	if (!EQEMuErrorList) {
		EQEMuErrorList = new LinkedList<char*>;
		MEQEMuErrorList = new Mutex;
	}
	LockMutex lock(MEQEMuErrorList);
	char* tmp = strcpy(new char[strlen(iError) + 1], iError);
	EQEMuErrorList->Append(tmp);

	if (iExitNow)
		CatchSignal(2);
}

uint32 CheckEQEMuError() {
	if (!EQEMuErrorList)
		return 0;
	uint32 ret = 0;
	char* tmp = 0;
	bool HeaderPrinted = false;
	LockMutex lock(MEQEMuErrorList);

	while ((tmp = EQEMuErrorList->Pop() )) {
		if (!HeaderPrinted) {
			fprintf(stdout, "===============================\nRuntime errors:\n\n");
			HeaderPrinted = true;
		}
		if (tmp[0] == 1) {
			fprintf(stdout, "%s\n", GetErrorText(*((uint32*) &tmp[1])));
			fprintf(stdout, "For more information on this error, visit http://www.eqemu.net/eqemuerror.php?id=%u\n\n", *((uint32*) &tmp[1]));
		}
		else {
			fprintf(stdout, "%s\n\n", tmp);
		}
		safe_delete(tmp);
		ret++;
	}
	return ret;
}

void CheckEQEMuErrorAndPause() {
#ifdef _WINDOWS
	if (CheckEQEMuError()) {
		fprintf(stdout, "Hit any key to exit\n");
		UpdateWindowTitle("Error");
		getch();
	}
#endif
}

