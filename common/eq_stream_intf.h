#ifndef EQSTREAMINTF_H_
#define EQSTREAMINTF_H_

//this is the only part of an EQStream that is seen by the application.

#include <string>
#include "emu_versions.h"
#include "eq_packet.h"
#include "net/daybreak_connection.h"
#include "event/event_loop.h"

typedef enum {
	ESTABLISHED,
	CLOSING,		//waiting for pending data to flush.
	DISCONNECTING,	//have sent disconnect, waiting for their disconnect reply.
	CLOSED,			//received a disconnect from remote side.
	UNESTABLISHED
} EQStreamState;

class EQApplicationPacket;
class OpcodeManager;

struct EQStreamManagerInterfaceOptions
{
	EQStreamManagerInterfaceOptions() {
		opcode_size = 2;
		loop = &EQ::EventLoop::GetDefault();
	}

	EQStreamManagerInterfaceOptions(int port, bool encoded, bool compressed) {
		opcode_size = 2;

		//World seems to support both compression and xor zone supports one or the others.
		//Enforce one or the other in the convienence construct
		//Login I had trouble getting to recognize compression at all 
		//but that might be because it was still a bit buggy when i was testing that.
		if (compressed) {
			daybreak_options.encode_passes[0] = EQ::Net::EncodeCompression;
		}
		else if (encoded) {
			daybreak_options.encode_passes[0] = EQ::Net::EncodeXOR;
		}

		daybreak_options.port = port;
		loop = &EQ::EventLoop::GetDefault();
	}

	int opcode_size;
	EQ::Net::DaybreakConnectionManagerOptions daybreak_options;
	EQ::EventLoop *loop;
};

enum EQStreamPriority : int32_t {
	High,
	Normal,
	Low
};

class EQStreamInterface;
class EQStreamManagerInterface
{
public:
	EQStreamManagerInterface(const EQStreamManagerInterfaceOptions &options) { m_options = options; }
	virtual ~EQStreamManagerInterface() { };

	const EQStreamManagerInterfaceOptions& GetOptions() const { return m_options; }
	EQStreamManagerInterfaceOptions& MutateOptions() { return m_options; }

	virtual void OnNewConnection(std::function<void(std::shared_ptr<EQStreamInterface>)> func) = 0;
	virtual void OnConnectionStateChange(std::function<void(std::shared_ptr<EQStreamInterface>, EQ::Net::DbProtocolStatus, EQ::Net::DbProtocolStatus)> func) = 0;
	virtual void SetPriority(EQStreamPriority priority) = 0;
protected:
	EQStreamManagerInterfaceOptions m_options;
};

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

	struct Stats
	{
		EQ::Net::DaybreakConnectionStats DaybreakStats;
		int RecvCount[_maxEmuOpcode];
		int SentCount[_maxEmuOpcode];
	};

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
	virtual const EQEmu::versions::ClientVersion ClientVersion() const { return EQEmu::versions::ClientVersion::Unknown; }
	virtual Stats GetStats() const = 0;
	virtual void ResetStats() = 0;
	virtual EQStreamManagerInterface* GetManager() const = 0;
};

#endif /*EQSTREAMINTF_H_*/
