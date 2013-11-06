#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include "types.h"

#include <vector>
#include <queue>

#define TCPServer_ErrorBufferSize	1024

//this is the non-connection type specific server.
class BaseTCPServer {
public:
	BaseTCPServer(uint16 iPort = 0);
	virtual ~BaseTCPServer();

	bool	Open(uint16 iPort = 0, char* errbuf = 0);			// opens the port
	void	Close();						// closes the port
	bool	IsOpen();
	inline uint16	GetPort()		{ return pPort; }
	inline uint32	GetNextID() { return NextID++; }

protected:
	static ThreadReturnType TCPServerLoop(void* tmp);

	//factory method:
	virtual void CreateNewConnection(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort) = 0;


	virtual void	Process();
	bool	RunLoop();
	Mutex	MLoopRunning;

	void StopLoopAndWait();

	void	ListenNewConnections();

	uint32	NextID;

	Mutex	MRunLoop;
	bool	pRunLoop;

	Mutex	MSock;
	SOCKET	sock;
	uint16	pPort;

};

template<class T>
class TCPServer : public BaseTCPServer {
protected:
	typedef typename std::vector<T *> vstore;
	typedef typename std::vector<T *>::iterator vitr;
public:
	TCPServer(uint16 iPort = 0)
	: BaseTCPServer(iPort) {
	}

	virtual ~TCPServer() {
		StopLoopAndWait();

		//im not sure what the right thing to do here is...
		//we are freeing a connection which somebody likely has a pointer to..
		//but, we really shouldent ever get called anyhow..
		vitr cur, end;
		cur = m_list.begin();
		end = m_list.end();
		for(; cur != end; cur++) {
			delete *cur;
		}
	}

	T * NewQueuePop() {
		T * ret = nullptr;
		MNewQueue.lock();
		if(!m_NewQueue.empty()) {
			ret = m_NewQueue.front();
			m_NewQueue.pop();
		}
		MNewQueue.unlock();
		return ret;
	}

protected:
	virtual void Process() {
		BaseTCPServer::Process();

		vitr cur;
		cur = m_list.begin();
		while(cur != m_list.end()) {
			T *data = *cur;
			if (data->IsFree() && (!data->CheckNetActive())) {
				#if EQN_DEBUG >= 4
					std::cout << "TCPConnection Connection deleted." << std::endl;
				#endif
				delete data;
				cur = m_list.erase(cur);
			} else {
				if (!data->Process())
					data->Disconnect();
				cur++;
			}
		}
	}

	void AddConnection(T *con) {
		m_list.push_back(con);
		MNewQueue.lock();
		m_NewQueue.push(con);
		MNewQueue.unlock();
	}

	//queue of new connections, for the app to pull from
	Mutex	MNewQueue;
	std::queue<T *> m_NewQueue;

	vstore m_list;
};


#endif /*TCPSERVER_H_*/














