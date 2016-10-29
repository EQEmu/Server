#ifndef EQSTREAMINTF_H_
#define EQSTREAMINTF_H_

//this is the only part of an EQStream that is seen by the application.

#include <string>
#include "emu_versions.h"
#include "eq_packet.h"

typedef enum {
	ESTABLISHED,
	CLOSING,		//waiting for pending data to flush.
	DISCONNECTING,	//have sent disconnect, waiting for their disconnect reply.
	CLOSED,			//received a disconnect from remote side.
	UNESTABLISHED
} EQStreamState;

class EQApplicationPacket;
class OpcodeManager;

class EQStreamInterface {
public:
	virtual ~EQStreamInterface() {}

	class Signature {
	public:
		//this object could get more complicated if needed...
		uint16 ignore_eq_opcode;		//0=dont ignore
		uint16 first_eq_opcode;
		uint32 first_length;			//0=dont check length
	};
	typedef enum {
		MatchNotReady,
		MatchSuccessful,
		MatchFailed
	} MatchState;

	virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req=true) = 0;
	virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req=true) = 0;
	virtual EQApplicationPacket *PopPacket() = 0;
	virtual void Close() = 0;
	virtual void ReleaseFromUse() = 0;
	virtual void RemoveData() = 0;
	virtual std::string GetRemoteAddr() const = 0;
	virtual uint32 GetRemoteIP() const = 0;
	virtual uint16 GetRemotePort() const = 0;
	virtual bool CheckState(EQStreamState state) = 0;
	virtual std::string Describe() const = 0;
	virtual void SetActive(bool val) { }
	virtual MatchState CheckSignature(const Signature *sig) { return MatchFailed; }
	virtual EQStreamState GetState() = 0;
	virtual void SetOpcodeManager(OpcodeManager **opm) = 0;

	virtual const uint32 GetBytesSent() const { return 0; }
	virtual const uint32 GetBytesRecieved() const { return 0; }
	virtual const uint32 GetBytesSentPerSecond() const { return 0; }
	virtual const uint32 GetBytesRecvPerSecond() const { return 0; }
	virtual const EQEmu::versions::ClientVersion ClientVersion() const { return EQEmu::versions::ClientVersion::Unknown; }
};

#endif /*EQSTREAMINTF_H_*/
