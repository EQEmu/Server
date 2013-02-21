#ifndef _EQSTREAM_H
#define _EQSTREAM_H

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <deque>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "EQStreamType.h"
#include "EQPacket.h"
#include "EQStreamIntf.h"
#include "Mutex.h"
#include "../common/opcodemgr.h"
#include "../common/misc.h"
#include "../common/Condition.h"
#include "../common/timer.h"

using namespace std;

#define FLAG_COMPRESSED	0x01
#define FLAG_ENCODED	0x04

#define RATEBASE	1048576 // 1 MB
#define DECAYBASE	78642	// RATEBASE/10

#pragma pack(1)
struct SessionRequest {
	uint32 UnknownA;
	uint32 Session;
	uint32 MaxLength;
};

struct SessionResponse {
	uint32 Session;
	uint32 Key;
	uint8 UnknownA;
	uint8 Format;
	uint8 UnknownB;
	uint32 MaxLength;
	uint32 UnknownD;
};

//Deltas are in ms, representing round trip times
struct SessionStats {
/*000*/	uint16 RequestID;
/*002*/	uint32 last_local_delta;
/*006*/	uint32 average_delta;
/*010*/	uint32 low_delta;
/*014*/	uint32 high_delta;
/*018*/	uint32 last_remote_delta;
/*022*/	uint64 packets_sent;
/*030*/	uint64 packets_received;
/*038*/
};
	
#pragma pack()

class OpcodeManager;    
//extern OpcodeManager *EQNetworkOpcodeManager;

//class EQStreamFactory;
class EQStreamPair;
class EQRawApplicationPacket;

class EQStream : public EQStreamInterface {
	friend class EQStreamPair;	//for collector.
	protected:
		typedef enum {
			SeqPast,
			SeqInOrder,
			SeqFuture
		} SeqOrder;
		
		uint32 remote_ip;
		uint16 remote_port;
		uint8 buffer[8192];
		unsigned char *oversize_buffer;
		uint32 oversize_offset,oversize_length;
		uint8 app_opcode_size;
		EQStreamType StreamType;
		bool compressed,encoded;
		uint32 retransmittimer;
		uint32 retransmittimeout;

		//uint32 buffer_len;

		uint32 Session, Key;
		uint16 NextInSeq;
		uint32  MaxLen;
		uint16 MaxSends;

		uint8 active_users;	//how many things are actively using this
		Mutex MInUse;

		EQStreamState State;
		Mutex MState;

		uint32 LastPacket;
		Mutex MVarlock;

		// Ack sequence tracking.
		long NextAckToSend;
		long LastAckSent;
		long GetNextAckToSend();
		long GetLastAckSent();
		void AckPackets(uint16 seq);
		void SetNextAckToSend(uint32);
		void SetLastAckSent(uint32);

		Mutex MAcks;

		// Packets waiting to be sent (all protected by MOutboundQueue)
		queue<EQProtocolPacket *> NonSequencedQueue;
		deque<EQProtocolPacket *> SequencedQueue;
		uint16 NextOutSeq;
		uint16 SequencedBase;	//the sequence number of SequencedQueue[0]
		long NextSequencedSend;	//index into SequencedQueue
		Mutex MOutboundQueue;
		
		//a buffer we use for compression/decompression
		unsigned char _tempBuffer[2048];
		
		// Packets waiting to be processed
		vector<EQRawApplicationPacket *> InboundQueue;
		map<unsigned short,EQProtocolPacket *> PacketQueue;		//not mutex protected, only accessed by caller of Process()
		Mutex MInboundQueue;

		static uint16 MaxWindowSize;

		int32 BytesWritten;

		Mutex MRate;
		int32 RateThreshold;
		int32 DecayRate;

		
		OpcodeManager **OpMgr;
		
//		EQStreamFactory *const Factory;

		EQRawApplicationPacket *MakeApplicationPacket(EQProtocolPacket *p);
		EQRawApplicationPacket *MakeApplicationPacket(const unsigned char *buf, uint32 len);
		EQProtocolPacket *MakeProtocolPacket(const unsigned char *buf, uint32 len);
		void SendPacket(uint16 opcode, EQApplicationPacket *p);
		
		void SetState(EQStreamState state);
		
		void SendSessionResponse();
		void SendSessionRequest();
		void SendAck(uint16 seq);
		void SendOutOfOrderAck(uint16 seq);
		void QueuePacket(EQProtocolPacket *p);
		void SendPacket(EQProtocolPacket *p);
		void NonSequencedPush(EQProtocolPacket *p);
		void SequencedPush(EQProtocolPacket *p);
		void WritePacket(int fd,EQProtocolPacket *p);
		

		uint32 GetKey() { return Key; }
		void SetKey(uint32 k) { Key=k; }
		void SetSession(uint32 s) { Session=s; }

		void ProcessPacket(EQProtocolPacket *p);
//		virtual void DispatchPacket(EQApplicationPacket *p) { p->DumpRaw(); }


		bool Stale(uint32 now, uint32 timeout=30) { return  (LastPacket && (now-LastPacket) > timeout); }

		void InboundQueuePush(EQRawApplicationPacket *p);
		EQRawApplicationPacket *PeekPacket();	//for collector.
		EQRawApplicationPacket *PopRawPacket();	//for collector.
		
		void InboundQueueClear();
		void OutboundQueueClear();
		void PacketQueueClear();
		
		void ProcessQueue();
		EQProtocolPacket *RemoveQueue(uint16 seq);
		
		void _SendDisconnect();
		
		void init();
	public:
		EQStream() { init(); remote_ip = 0; remote_port = 0; State=UNESTABLISHED; StreamType=UnknownStream; compressed=true; encoded=false; app_opcode_size=2; bytes_sent=0; bytes_recv=0; create_time=Timer::GetTimeSeconds(); }
		EQStream(sockaddr_in addr) { init(); remote_ip=addr.sin_addr.s_addr; remote_port=addr.sin_port; State=UNESTABLISHED; StreamType=UnknownStream; compressed=true; encoded=false; app_opcode_size=2; bytes_sent=0; bytes_recv=0; create_time=Timer::GetTimeSeconds(); }
		virtual ~EQStream() { RemoveData(); SetState(CLOSED); }
//		inline void SetFactory(EQStreamFactory *f) { Factory=f; }
		void SetMaxLen(uint32 length) { MaxLen=length; }

		//interface used by application (EQStreamInterface)
		virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req=true);
		virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req=true);
		virtual EQApplicationPacket *PopPacket();
		virtual void Close();
		virtual uint32 GetRemoteIP() const { return remote_ip; }
		virtual uint16 GetRemotePort() const { return remote_port; }
		virtual void ReleaseFromUse() { MInUse.lock(); if(active_users > 0) active_users--; MInUse.unlock(); }
		virtual void RemoveData() { InboundQueueClear(); OutboundQueueClear(); PacketQueueClear(); /*if (CombinedAppPacket) delete CombinedAppPacket;*/ }
		virtual bool CheckState(EQStreamState state) { return GetState() == state; }
		virtual std::string Describe() const { return("Direct EQStream"); }
		
		void SetOpcodeManager(OpcodeManager **opm) { OpMgr = opm; }
		
		void CheckTimeout(uint32 now, uint32 timeout=30);
		bool HasOutgoingData();
		void Process(const unsigned char *data, const uint32 length);
		void SetLastPacketTime(uint32 t) {LastPacket=t;}
		void Write(int eq_fd);
		
		//
		inline bool IsInUse() { bool flag; MInUse.lock(); flag=(active_users>0); MInUse.unlock(); return flag; }
		inline void PutInUse() { MInUse.lock(); active_users++; MInUse.unlock(); }
		
		inline EQStreamState GetState() { EQStreamState s; MState.lock(); s=State; MState.unlock(); return s; }
		
//		static EQProtocolPacket *Read(int eq_fd, sockaddr_in *from);
		static SeqOrder CompareSequence(uint16 expected_seq , uint16 seq);

//		void Close() { SendDisconnect(); }
		bool CheckActive() { return GetState()==ESTABLISHED; }
		bool CheckClosed() { return GetState()==CLOSED; }
		void SetOpcodeSize(uint8 s) { app_opcode_size = s; }
		void SetStreamType(EQStreamType t);
		inline const EQStreamType GetStreamType() const { return StreamType; }
		static const char *StreamTypeString(EQStreamType t);

		void Decay();
		void AdjustRates(uint32 average_delta);

		uint32 bytes_sent;
		uint32 bytes_recv;
		uint32 create_time;

		void AddBytesSent(uint32 bytes)
		{
			bytes_sent += bytes;
		}

		void AddBytesRecv(uint32 bytes)
		{
			bytes_recv += bytes;
		}

		virtual const uint32 GetBytesSent() const { return bytes_sent; }
		virtual const uint32 GetBytesRecieved() const { return bytes_recv; }
		virtual const uint32 GetBytesSentPerSecond() const
		{
			if((Timer::GetTimeSeconds() - create_time) == 0)
				return 0;
			return bytes_sent / (Timer::GetTimeSeconds() - create_time);
		}

		virtual const uint32 GetBytesRecvPerSecond() const
		{
			if((Timer::GetTimeSeconds() - create_time) == 0)
				return 0;
			return bytes_recv / (Timer::GetTimeSeconds() - create_time);
		}
		
		//used for dynamic stream identification
		class Signature {
		public:
			//this object could get more complicated if needed...
			uint16 ignore_eq_opcode;        //0=dont ignore
			uint16 first_eq_opcode;
			uint32 first_length;            //0=dont check length
		};
		typedef enum {
			MatchNotReady,
			MatchSuccessful,
			MatchFailed
		} MatchState;
		MatchState CheckSignature(const Signature *sig);
		
};


#endif
