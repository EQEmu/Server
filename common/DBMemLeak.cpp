#ifdef _EQDEBUG
#include "../common/debug.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../common/Mutex.h"
#include "DBMemLeak.h"

#include <crtdbg.h>

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

DBMemLeak dbmemleak;
LinkedList<DBMemLeakStruct*>* list = 0;
Mutex MDBMemLeak;

DBMemLeak::DBMemLeak() {
	list = new LinkedList<DBMemLeakStruct*>;
}

DBMemLeak::~DBMemLeak() {
	LinkedListIterator<DBMemLeakStruct*> iterator(*list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		char tmp[200];
		snprintf(tmp, sizeof(tmp) - 3, "DB Mem Leak: Block=%6d, Query=%s", iterator.GetData()->memblock, iterator.GetData()->query);
		snprintf(tmp, sizeof(tmp), "%s\n", tmp);
		OutputDebugString(tmp);
		iterator.Advance();
	}
	safe_delete(list);
}

void DBMemLeak::Alloc(const void* result, const char* query) {
	LockMutex lock(&MDBMemLeak);
	long requestNumber;
	uint8* tmp2 = new uint8;
	_CrtIsMemoryBlock( tmp2, 1, &requestNumber, 0, 0 );
	safe_delete(tmp2);
	DBMemLeakStruct* tmp = (DBMemLeakStruct*) new uchar[sizeof(DBMemLeakStruct) + strlen(query) + 1];
	tmp->result = result;
	tmp->memblock = requestNumber;
	strcpy(tmp->query, query);
	list->Append(tmp);
}

void DBMemLeak::Free(const void* result) {
	LockMutex lock(&MDBMemLeak);
	LinkedListIterator<DBMemLeakStruct*> iterator(*list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		if (result == iterator.GetData()->result)
			iterator.RemoveCurrent();
		else
			iterator.Advance();
	}
}
#endif
