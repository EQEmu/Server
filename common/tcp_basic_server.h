#ifndef TCPBASICSERVER_H_
#define TCPBASICSERVER_H_

#include "tcp_server.h"
#include "tcp_connection.h"

class TCPBasicServer : public TCPServer<TCPConnection> {
public:
	inline TCPBasicServer(uint16 iPort = 0) : TCPServer<TCPConnection>(iPort) { }
	inline virtual void CreateNewConnection(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort) {
		TCPConnection *conn = new TCPConnection(ID, in_socket, irIP, irPort);
		AddConnection(conn);
	}
};

#endif /*TCPBASICSERVER_H_*/

