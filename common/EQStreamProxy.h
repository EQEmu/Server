#ifndef EQSTREAMPROXY_H_
#define EQSTREAMPROXY_H_


#include "types.h"
#include "EQStreamIntf.h"

class EQStream;
class StructStrategy;
class OpcodeManager;
class EQApplicationPacket;

class EQStreamProxy : public EQStreamInterface {
public:
	//takes ownership of the stream.
	EQStreamProxy(EQStream *&stream, const StructStrategy *structs, OpcodeManager **opcodes);
	virtual ~EQStreamProxy();

	//EQStreamInterface:
	virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req=true);
	virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req=true);
	virtual EQApplicationPacket *PopPacket();
	virtual void Close();
	virtual uint32 GetRemoteIP() const;
	virtual uint16 GetRemotePort() const;
	virtual void ReleaseFromUse();
	virtual void RemoveData();
	virtual bool CheckState(EQStreamState state);
	virtual std::string Describe() const;
	virtual const EQClientVersion ClientVersion() const;

	virtual const uint32 GetBytesSent() const;
	virtual const uint32 GetBytesRecieved() const;
	virtual const uint32 GetBytesSentPerSecond() const;
	virtual const uint32 GetBytesRecvPerSecond() const;

protected:
	EQStream *const					m_stream;	//we own this stream object.
	const StructStrategy *const		m_structs;	//we do not own this object.
	//this is a pointer to a pointer to make it less likely that a packet will
	//reference an invalid opcode manager when they are being reloaded.
	OpcodeManager **const			m_opcodes;	//we do not own this object.
};

#endif /*EQSTREAMPROXY_H_*/

