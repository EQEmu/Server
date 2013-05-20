#ifndef DBASYNC_H
#define DBASYNC_H
#include "../common/dbcore.h"
#include "../common/timeoutmgr.h"


class DBAsyncFinishedQueue;
class DBAsyncWork;
class DBAsyncQuery;
class Database;

// Big daddy that owns the threads and does the work
class DBAsync : private Timeoutable {
public:
	enum Status { AddingWork, Queued, Executing, Finished, Canceled };
	enum Type { Read, Write, Both };

	DBAsync(DBcore* iDBC);
	~DBAsync();
	bool	StopThread();

	uint32	AddWork(DBAsyncWork** iWork, uint32 iDelay = 0);
	bool	CancelWork(uint32 iWorkID);
	void	CommitWrites();

	void	AddFQ(DBAsyncFinishedQueue* iDBAFQ);
protected:
	//things related to the processing thread:
	friend ThreadReturnType DBAsyncLoop(void* tmp);
	Mutex	MLoopRunning;
	Condition CInList;
	bool	RunLoop();
	void	Process();

private:
	virtual void CheckTimeout();

	void	ProcessWork(DBAsyncWork* iWork, bool iSleep = true);
	void	DispatchWork(DBAsyncWork* iWork);
	inline	uint32	GetNextID()		{ return pNextID++; }
	DBAsyncWork*	InListPop();
	DBAsyncWork*	InListPopWrite();	// Ignores delay
	void			OutListPush(DBAsyncWork* iDBAW);

	Mutex	MRunLoop;
	bool	pRunLoop;

	DBcore*	pDBC;
	uint32	pNextID;
	Mutex	MInList;
	LinkedList<DBAsyncWork*> InList;

	Mutex	MFQList;
	LinkedList<DBAsyncFinishedQueue**> FQList;

	// Mutex for outside access to current work & when current work is being changed.
	// NOT locked when CurrentWork is being accessed by the DBAsync thread.
	// Never change pointer from outside DBAsync thread!
	// Only here for access to thread-safe DBAsyncWork functions.
	Mutex	MCurrentWork;
	DBAsyncWork* CurrentWork;

};

/*
	DB Work Complete Callback:
		This will be called under the DBAsync thread! Never access any non-threadsafe
		data/functions/classes. (ie: zone, entitylist, client, etc are not threadsafe)
	Function prototype:
		return value:	true if we should delete the data, false if we should keep it
*/
typedef bool(*DBWorkCompleteCallBack)(DBAsyncWork*);

class DBAsyncFinishedQueue {
public:
	DBAsyncFinishedQueue(uint32 iTimeout = 90000);
	~DBAsyncFinishedQueue();

	DBAsyncWork*	Pop();
	DBAsyncWork*	PopByWPT(uint32 iWPT);
	DBAsyncWork*	Find(uint32 iWPT);
	bool			Push(DBAsyncWork* iDBAW);

	void			CheckTimeouts();
private:
	Mutex MLock;
	uint32 pTimeout;
	LinkedList<DBAsyncWork*> list;
};

// Container class for multiple queries
class DBAsyncWork {
public:
	DBAsyncWork(Database *db, DBAsyncFinishedQueue* iDBAFQ, uint32 iWPT = 0, DBAsync::Type iType = DBAsync::Both, uint32 iTimeout = 0);
	DBAsyncWork(Database *db, DBWorkCompleteCallBack iCB, uint32 iWPT = 0, DBAsync::Type iType = DBAsync::Both, uint32 iTimeout = 0);
	~DBAsyncWork();

	bool			AddQuery(DBAsyncQuery** iDBAQ);
	bool			AddQuery(uint32 iQPT, char** iQuery, uint32 iQueryLen = 0xFFFFFFFF, bool iGetResultSet = true, bool iGetErrbuf = true);
	uint32			WPT();
	DBAsync::Type	Type();

	// Pops finished queries off the work
	DBAsyncQuery*	PopAnswer();
	uint32			QueryCount();

	Database *GetDB() const { return(m_db); }

	bool			CheckTimeout(uint32 iFQTimeout);
	bool			SetWorkID(uint32 iWorkID);
	uint32			GetWorkID();
protected:
	friend class DBAsync;
	DBAsync::Status	SetStatus(DBAsync::Status iStatus);
	bool			Cancel();
	bool			IsCancled();
	DBAsyncQuery*	PopQuery();	// Get query to be run
	void			PushAnswer(DBAsyncQuery* iDBAQ);	// Push answer back into workset

	// not mutex'd cause only to be accessed from dbasync class
	uint32	pExecuteAfter;
private:
	Mutex	MLock;
	uint32	pQuestionCount;
	uint32	pAnswerCount;
	uint32	pWorkID;
	uint32	pWPT;
	uint32	pTimeout;
	uint32	pTSFinish; // timestamp when finished
	DBAsyncFinishedQueue*	pDBAFQ;		//we do now own this pointer
	DBWorkCompleteCallBack	pCB;
	DBAsync::Status			pstatus;
	DBAsync::Type			pType;
	MyQueue<DBAsyncQuery>	todo;
	MyQueue<DBAsyncQuery>	done;
	MyQueue<DBAsyncQuery>	todel;
	Database *const			m_db;		//we do now own this pointer
};

// Container class for the query information
class DBAsyncQuery {
public:
	DBAsyncQuery(uint32 iQPT, char** iQuery, uint32 iQueryLen = 0xFFFFFFFF, bool iGetResultSet = true, bool iGetErrbuf = true);
	DBAsyncQuery(uint32 iQPT, const char* iQuery, uint32 iQueryLen = 0xFFFFFFFF, bool iGetResultSet = true, bool iGetErrbuf = true);
	~DBAsyncQuery();

	bool	GetAnswer(char* errbuf = 0, MYSQL_RES** result = 0, uint32* affected_rows = 0, uint32* last_insert_id = 0, uint32* errnum = 0);
	inline uint32 QPT()	{ return pQPT; }
protected:
	friend class DBAsyncWork;
	uint32		pQPT;

	friend class DBAsync;
	void		Process(DBcore* iDBC);

	void		Init(uint32 iQPT, bool iGetResultSet, bool iGetErrbuf);
	DBAsync::Status pstatus;
	char*		pQuery;
	uint32		pQueryLen;
	bool		pGetResultSet;
	bool		pGetErrbuf;

	bool		pmysqlsuccess;
	char*		perrbuf;
	uint32		perrnum;
	uint32		paffected_rows;
	uint32		plast_insert_id;
	MYSQL_RES*	presult;
};


void AsyncLoadVariables(DBAsync *dba, Database *db);


#endif

