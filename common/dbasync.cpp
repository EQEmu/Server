#include "debug.h"
#ifdef _WINDOWS
	#include <windows.h>
	#include <process.h>
	#include <winsock2.h>
#endif
#include <iostream>
using namespace std;
#include "dbasync.h"
#include "database.h"
#include <errmsg.h>
#include <mysqld_error.h>
#include <limits.h>
#include "dbcore.h"
#include "common_profile.h"
#include <string.h>
#include "../common/MiscFunctions.h"
#define ASYNC_LOOP_GRANULARITY 4 //# of ms between checking our work

bool DBAsyncCB_LoadVariables(DBAsyncWork* iWork) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* result = 0;
	DBAsyncQuery* dbaq = iWork->PopAnswer();
	if (dbaq->GetAnswer(errbuf, &result))
		iWork->GetDB()->LoadVariables_result(result);
	else
		cout << "Error: DBAsyncCB_LoadVariables failed: !GetAnswer: '" << errbuf << "'" << endl;
	return true;
}

void AsyncLoadVariables(DBAsync *dba, Database *db) {
	char* query = 0;
	DBAsyncWork* dbaw = new DBAsyncWork(db, &DBAsyncCB_LoadVariables, 0, DBAsync::Read);
	dbaw->AddQuery(0, &query, db->LoadVariables_MQ(&query));
	dba->AddWork(&dbaw);
}


//we only need to do anything when somebody puts work on the queue
//so instead of checking all the time, we will wait on a condition
//which will get signaled when somebody puts something on the queue
ThreadReturnType DBAsyncLoop(void* tmp) {
	DBAsync* dba = (DBAsync*) tmp;

#ifndef WIN32
	_log(COMMON__THREADS, "Starting DBAsyncLoop with thread ID %d", pthread_self());
#endif

	dba->MLoopRunning.lock();
	while (dba->RunLoop()) {
		//wait before working so we check the loop condition
		//as soon as were done working
		dba->CInList.Wait();
		//we could check dba->RunLoop() again to see if we
		//got turned off while we were waiting
		{
			_CP(DBAsyncLoop_loop);
			dba->Process();
		}
//		Sleep(ASYNC_LOOP_GRANULARITY);
	}
	dba->MLoopRunning.unlock();

#ifndef WIN32
	_log(COMMON__THREADS, "Ending DBAsyncLoop with thread ID %d", pthread_self());
#endif

	THREAD_RETURN(nullptr);
}

DBAsync::DBAsync(DBcore* iDBC)
: Timeoutable(10000)
{
	pDBC = iDBC;
	pRunLoop = true;
	pNextID = 1;
#ifdef _WINDOWS
	_beginthread(DBAsyncLoop, 0, this);
#else
	pthread_t thread;
	pthread_create(&thread, nullptr, DBAsyncLoop, this);
#endif
}

DBAsync::~DBAsync() {
	StopThread();
}

bool DBAsync::StopThread() {
	bool ret;
	MRunLoop.lock();
	ret = pRunLoop;
	pRunLoop = false;
	MRunLoop.unlock();

	//signal the condition so we exit the loop if were waiting
	CInList.Signal();

	//this effectively waits for the processing thread to finish
	MLoopRunning.lock();
	MLoopRunning.unlock();

	return ret;
}

uint32 DBAsync::AddWork(DBAsyncWork** iWork, uint32 iDelay) {
	MInList.lock();
	uint32 ret = GetNextID();
	if (!(*iWork)->SetWorkID(ret)) {
		MInList.unlock();
		return 0;
	}
	InList.Append(*iWork);
	(*iWork)->SetStatus(Queued);
	if (iDelay)
		(*iWork)->pExecuteAfter = Timer::GetCurrentTime() + iDelay;
#if DEBUG_MYSQL_QUERIES >= 2
	cout << "Adding AsyncWork #" << (*iWork)->GetWorkID() << endl;
	cout << "ExecuteAfter = " << (*iWork)->pExecuteAfter << " (" << Timer::GetCurrentTime() << " + " << iDelay << ")" << endl;
#endif
	*iWork = 0;
	MInList.unlock();

	//wake up the processing thread and tell it to get to work.
	CInList.Signal();

	return ret;
}

bool DBAsync::CancelWork(uint32 iWorkID) {
	if (iWorkID == 0)
		return false;
#if DEBUG_MYSQL_QUERIES >= 2
	cout << "DBAsync::CancelWork: " << iWorkID << endl;
#endif
	MCurrentWork.lock();
	if (CurrentWork && CurrentWork->GetWorkID() == iWorkID) {
		CurrentWork->Cancel();
		MCurrentWork.unlock();
		return true;
	}
	MCurrentWork.unlock();
	MInList.lock();
	LinkedListIterator<DBAsyncWork*> iterator(InList);

	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->GetWorkID() == iWorkID) {
			iterator.RemoveCurrent(true);
			MInList.unlock();
			return true;
		}
		iterator.Advance();
	}
	MInList.unlock();
	return false;
}

bool DBAsync::RunLoop() {
	bool ret;
	MRunLoop.lock();
	ret = pRunLoop;
	MRunLoop.unlock();
	return ret;
}

DBAsyncWork* DBAsync::InListPop() {
	DBAsyncWork* ret = 0;
	MInList.lock();
	LinkedListIterator<DBAsyncWork*> iterator(InList);

	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->pExecuteAfter <= Timer::GetCurrentTime()) {
			ret = iterator.GetData();
#if DEBUG_MYSQL_QUERIES >= 2
			cout << "Poping AsyncWork #" << ret->GetWorkID() << endl;
			cout << ret->pExecuteAfter << " <= " << Timer::GetCurrentTime() << endl;
#endif
			iterator.RemoveCurrent(false);
			break;
		}
		iterator.Advance();
	}
	MInList.unlock();
	return ret;
}

DBAsyncWork* DBAsync::InListPopWrite() {
	MInList.lock();
	LinkedListIterator<DBAsyncWork*> iterator(InList);

	DBAsyncWork* ret = 0;
	DBAsync::Type tmpType;
	iterator.Reset();
	while (iterator.MoreElements()) {
		tmpType = iterator.GetData()->Type();
		if (tmpType == Write || tmpType == Both) {
			ret = iterator.GetData();
			iterator.RemoveCurrent(false);
			break;
		}
		iterator.Advance();
	}
	MInList.unlock();
	return ret;
}

void DBAsync::AddFQ(DBAsyncFinishedQueue* iDBAFQ) {
	MFQList.lock();
	DBAsyncFinishedQueue** tmp = new DBAsyncFinishedQueue*;
	*tmp = iDBAFQ;
	FQList.Append(tmp);
	MFQList.unlock();
}

void DBAsync::Process() {
	DBAsyncWork* tmpWork;
	MCurrentWork.lock();
	while ((CurrentWork = InListPop())) {
		MCurrentWork.unlock();
		//move from queued to executing
		Status tmpStatus = CurrentWork->SetStatus(Executing);
		if (tmpStatus == Queued) {
			//execute the work
			ProcessWork(CurrentWork);
			tmpWork = CurrentWork;
			MCurrentWork.lock();
			CurrentWork = 0;
			MCurrentWork.unlock();
			//move from executing to finished
			tmpStatus = tmpWork->SetStatus(DBAsync::Finished);
			if (tmpStatus != Executing) {
				if (tmpStatus != Canceled) {
					cout << "Error: Unexpected DBAsyncWork->Status in DBAsync::Process #1" << endl;
				}
				MCurrentWork.lock();
				safe_delete(tmpWork);
			}
			else {
				//call callbacks or put results on finished queue
				DispatchWork(tmpWork);
				Sleep(25);
				MCurrentWork.lock();
			}
		}
		else {
			if (tmpStatus != Canceled) {
				cout << "Error: Unexpected DBAsyncWork->Status in DBAsync::Process #2" << endl;
			}
			MCurrentWork.lock();
			safe_delete(CurrentWork);
		}
	}
	MCurrentWork.unlock();
}

void DBAsync::CheckTimeout() {
	try{
		MFQList.lock();
		LinkedListIterator<DBAsyncFinishedQueue**> iterator(FQList);

		iterator.Reset();
		while (iterator.MoreElements()) {
			(*iterator.GetData())->CheckTimeouts();
			iterator.Advance();
		}
		MFQList.unlock();
	}
	catch(...){

	}
}

void DBAsync::CommitWrites() {
#if DEBUG_MYSQL_QUERIES >= 2
	cout << "DBAsync::CommitWrites() called." << endl;
#endif
	DBAsyncWork* tmpWork;
	while ((tmpWork = InListPopWrite())) {
		Status tmpStatus = tmpWork->SetStatus(Executing);
		if (tmpStatus == Queued) {
			ProcessWork(tmpWork);
			tmpStatus = tmpWork->SetStatus(DBAsync::Finished);
			if (tmpStatus != Executing) {
				if (tmpStatus != Canceled) {
					cout << "Error: Unexpected DBAsyncWork->Status in DBAsync::CommitWrites #1" << endl;
				}
				safe_delete(tmpWork);
			}
			else {
				DispatchWork(tmpWork);
			}
		}
		else {
			if (tmpStatus != Canceled) {
				cout << "Error: Unexpected DBAsyncWork->Status in DBAsync::CommitWrites #2" << endl;
			}
			safe_delete(tmpWork);
		}
	}
}

void DBAsync::ProcessWork(DBAsyncWork* iWork, bool iSleep) {
	_CP(DBAsync_ProcessWork);
	DBAsyncQuery* CurrentQuery;
#if DEBUG_MYSQL_QUERIES >= 2
	cout << "Processing AsyncWork #" << iWork->GetWorkID() << endl;
#endif
	while ((CurrentQuery = iWork->PopQuery())) {
		CurrentQuery->Process(pDBC);
		iWork->PushAnswer(CurrentQuery);
		if (iSleep)
			Sleep(1);
	}
}

void DBAsync::DispatchWork(DBAsyncWork* iWork) {
	_CP(DBAsync_DispatchWork);
	//if this work has a callback, call it
	//otherwise, stick the work on the finish queue
	if (iWork->pCB) {
		if (iWork->pCB(iWork))
			safe_delete(iWork);
	}
	else {
		if (!iWork->pDBAFQ->Push(iWork))
			safe_delete(iWork);
	}
}



DBAsyncFinishedQueue::DBAsyncFinishedQueue(uint32 iTimeout) {
	pTimeout = iTimeout;
}

DBAsyncFinishedQueue::~DBAsyncFinishedQueue() {
}

void DBAsyncFinishedQueue::CheckTimeouts() {
	if (pTimeout == 0xFFFFFFFF)
		return;
	MLock.lock();
	LinkedListIterator<DBAsyncWork*> iterator(list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->CheckTimeout(pTimeout))
			iterator.RemoveCurrent(true);
		iterator.Advance();
	}
	MLock.unlock();
}

DBAsyncWork* DBAsyncFinishedQueue::Pop() {
	DBAsyncWork* ret = 0;
	MLock.lock();
	ret = list.Pop();
	MLock.unlock();
	return ret;
}

DBAsyncWork* DBAsyncFinishedQueue::Find(uint32 iWorkID) {
	DBAsyncWork* ret = 0;
	MLock.lock();
	LinkedListIterator<DBAsyncWork*> iterator(list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->GetWorkID() == iWorkID) {
			ret = iterator.GetData();
			iterator.RemoveCurrent(false);
			break;
		}
		iterator.Advance();
	}
	MLock.unlock();
	return ret;
}

DBAsyncWork* DBAsyncFinishedQueue::PopByWPT(uint32 iWPT) {
	DBAsyncWork* ret = 0;
	MLock.lock();
	LinkedListIterator<DBAsyncWork*> iterator(list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->WPT() == iWPT) {
			ret = iterator.GetData();
			iterator.RemoveCurrent(false);
			break;
		}
		iterator.Advance();
	}
	MLock.unlock();
	return ret;
}

bool DBAsyncFinishedQueue::Push(DBAsyncWork* iDBAW) {
	if (!this)
		return false;
	MLock.lock();
	list.Append(iDBAW);
	MLock.unlock();
	return true;
}



DBAsyncWork::DBAsyncWork(Database *db, DBAsyncFinishedQueue* iDBAFQ, uint32 iWPT, DBAsync::Type iType, uint32 iTimeout)
: m_db(db)
{
	pstatus = DBAsync::AddingWork;
	pType = iType;
	pExecuteAfter = 0;
	pWorkID = 0;
	pDBAFQ = iDBAFQ;
	pCB = 0;
	pWPT = iWPT;
	pQuestionCount = 0;
	pAnswerCount = 0;
	pTimeout = iTimeout;
	pTSFinish = 0;
}

DBAsyncWork::DBAsyncWork(Database *db, DBWorkCompleteCallBack iCB, uint32 iWPT, DBAsync::Type iType, uint32 iTimeout)
: m_db(db)
{
	pstatus = DBAsync::AddingWork;
	pType = iType;
	pExecuteAfter = 0;
	pWorkID = 0;
	pDBAFQ = 0;
	pCB = iCB;
	pWPT = iWPT;
	pQuestionCount = 0;
	pAnswerCount = 0;
	pTimeout = iTimeout;
	pTSFinish = 0;
}

DBAsyncWork::~DBAsyncWork() {
	DBAsyncQuery* dbaq = 0;
	while ((dbaq = todo.pop()))
		safe_delete(dbaq);
	while ((dbaq = done.pop()))
		safe_delete(dbaq);
	while ((dbaq = todel.pop()))
		safe_delete(dbaq);
}

bool DBAsyncWork::AddQuery(DBAsyncQuery** iDBAQ) {
	bool ret;
	MLock.lock();
	if (pstatus != DBAsync::AddingWork)
		ret = false;
	else {
		ret = true;
		pQuestionCount++;
		todo.push(*iDBAQ);
		(*iDBAQ)->pstatus = DBAsync::Queued;
		*iDBAQ = 0;
	}
	MLock.unlock();
	return ret;
}

bool DBAsyncWork::AddQuery(uint32 iQPT, char** iQuery, uint32 iQueryLen, bool iGetResultSet, bool iGetErrbuf) {
	DBAsyncQuery* DBAQ = new DBAsyncQuery(iQPT, iQuery, iQueryLen, iGetResultSet, iGetErrbuf);
	if (AddQuery(&DBAQ))
		return true;
	else {
		safe_delete(DBAQ);
		return false;
	}
}

bool DBAsyncWork::SetWorkID(uint32 iWorkID) {
	bool ret = true;
	MLock.lock();
	if (pWorkID)
		ret = false;
	else
		pWorkID = iWorkID;
	MLock.unlock();
	return ret;
}

uint32 DBAsyncWork::GetWorkID() {
	uint32 ret;
	MLock.lock();
	ret = pWorkID;
	MLock.unlock();
	return ret;
}

uint32 DBAsyncWork::WPT() {
	uint32 ret;
	MLock.lock();
	ret = pWPT;
	MLock.unlock();
	return ret;
}

DBAsync::Type DBAsyncWork::Type() {
	DBAsync::Type ret;
	MLock.lock();
	ret = pType;
	MLock.unlock();
	return ret;
}

DBAsyncQuery* DBAsyncWork::PopAnswer() {
	DBAsyncQuery* ret;
	MLock.lock();
	ret = done.pop();
	if (ret)
		pAnswerCount--;
	todel.push(ret);
	MLock.unlock();
	return ret;
}

bool DBAsyncWork::CheckTimeout(uint32 iFQTimeout) {
	if (pTimeout == 0xFFFFFFFF)
		return false;
	bool ret = false;
	MLock.lock();
	if (pTimeout > iFQTimeout)
		iFQTimeout = pTimeout;
	if (Timer::GetCurrentTime() > (pTSFinish + iFQTimeout))
		ret = true;
	MLock.unlock();
	return ret;
}

//sets the work's status to the supplied value and returns
//the revious status
DBAsync::Status DBAsyncWork::SetStatus(DBAsync::Status iStatus) {
	DBAsync::Status ret;
	MLock.lock();
	if (iStatus == DBAsync::Finished)
		pTSFinish = Timer::GetCurrentTime();
	ret = pstatus;
	pstatus = iStatus;
	MLock.unlock();
	return ret;
}

bool DBAsyncWork::Cancel() {
	bool ret;
	MLock.lock();
	if (pstatus != DBAsync::Finished) {
		pstatus = DBAsync::Canceled;
		ret = true;
	}
	else
		ret = false;
	MLock.unlock();
	return ret;
}

bool DBAsyncWork::IsCancled() {
	bool ret;
	MLock.lock();
	ret = (bool) (pstatus == DBAsync::Canceled);
	MLock.unlock();
	return ret;
}

DBAsyncQuery* DBAsyncWork::PopQuery() {
	DBAsyncQuery* ret = 0;
	MLock.lock();
	ret = todo.pop();
	if (ret)
		pQuestionCount--;
	MLock.unlock();
	return ret;
}

void DBAsyncWork::PushAnswer(DBAsyncQuery* iDBAQ) {
	MLock.lock();
	done.push(iDBAQ);
	pAnswerCount++;
	MLock.unlock();
}


DBAsyncQuery::DBAsyncQuery(uint32 iQPT, char** iQuery, uint32 iQueryLen, bool iGetResultSet, bool iGetErrbuf) {
	if (iQueryLen == 0xFFFFFFFF)
		pQueryLen = strlen(*iQuery);
	else
		pQueryLen = iQueryLen;
	pQuery = *iQuery;
	*iQuery = 0;
	Init(iQPT, iGetResultSet, iGetErrbuf);
}

DBAsyncQuery::DBAsyncQuery(uint32 iQPT, const char* iQuery, uint32 iQueryLen, bool iGetResultSet, bool iGetErrbuf) {
	if (iQueryLen == 0xFFFFFFFF)
		pQueryLen = strlen(iQuery);
	else
		pQueryLen = iQueryLen;
	pQuery = strn0cpy(new char[pQueryLen+1], iQuery, pQueryLen+1);
	Init(iQPT, iGetResultSet, iGetErrbuf);
}

void DBAsyncQuery::Init(uint32 iQPT, bool iGetResultSet, bool iGetErrbuf) {
	pstatus = DBAsync::AddingWork;
	pQPT = iQPT;
	pGetResultSet = iGetResultSet;
	pGetErrbuf = iGetErrbuf;

	pmysqlsuccess = false;
	perrbuf = 0;
	perrnum = 0;
	presult = 0;
	paffected_rows = 0;
	plast_insert_id = 0;
}

DBAsyncQuery::~DBAsyncQuery() {
	safe_delete_array(perrbuf);
	safe_delete_array(pQuery);
	if (presult)
		mysql_free_result(presult);
}

bool DBAsyncQuery::GetAnswer(char* errbuf, MYSQL_RES** result, uint32* affected_rows, uint32* last_insert_id, uint32* errnum) {
	if (pstatus != DBAsync::Finished) {
		if (errbuf)
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "Error: Query not finished.");
		if (errnum)
			*errnum = UINT_MAX;
		return false;
	}
	if (errbuf) {
		if (pGetErrbuf) {
			if (perrbuf)
				strn0cpy(errbuf, perrbuf, MYSQL_ERRMSG_SIZE);
			else
				snprintf(errbuf, MYSQL_ERRMSG_SIZE, "Error message should've been saved, but hasnt. errno: %u", perrnum);
		}
		else
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "Error message not saved. errno: %u", perrnum);
	}
	if (errnum)
		*errnum = perrnum;
	if (affected_rows)
		*affected_rows = paffected_rows;
	if (last_insert_id)
		*last_insert_id = plast_insert_id;
	if (result)
		*result = presult;
	return pmysqlsuccess;
}

void DBAsyncQuery::Process(DBcore* iDBC) {
	pstatus = DBAsync::Executing;
	if (pGetErrbuf)
		perrbuf = new char[MYSQL_ERRMSG_SIZE];
	MYSQL_RES** resultPP = 0;
	if (pGetResultSet)
		resultPP = &presult;
	pmysqlsuccess = iDBC->RunQuery(pQuery, pQueryLen, perrbuf, resultPP, &paffected_rows, &plast_insert_id, &perrnum);
	pstatus = DBAsync::Finished;
}









