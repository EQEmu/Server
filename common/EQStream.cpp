/* 
	Copyright (C) 2005 Michael S. Finger

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "debug.h"
#include <string>
#include <iomanip>
#include <iostream>
#include <vector>
#include <time.h>
#include <sys/types.h>
#ifdef _WINDOWS
	#include <time.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <arpa/inet.h>
#endif
#include "EQPacket.h"
#include "EQStream.h"
//#include "EQStreamFactory.h"
#include "misc.h"
#include "Mutex.h"
#include "op_codes.h"
#include "CRC16.h"

#if defined(ZONE) || defined(WORLD)
	#define RETRANSMITS
#endif
#ifdef RETRANSMITS
	#include "rulesys.h"
#endif

//for logsys
#define _L "%s:%d: "
#define __L , long2ip(remote_ip).c_str(), ntohs(remote_port)

uint16 EQStream::MaxWindowSize=2048;

void EQStream::init() {
	active_users = 0;
	Session=0;
	Key=0;
	MaxLen=0;
	NextInSeq=0;
	NextOutSeq=0;
	NextAckToSend=-1;
	LastAckSent=-1;
	MaxSends=5;
	LastPacket=0;
	oversize_buffer=NULL;
	oversize_length=0;
	oversize_offset=0;
	RateThreshold=RATEBASE/250;
	DecayRate=DECAYBASE/250;
	BytesWritten=0;
	SequencedBase = 0;
	NextSequencedSend = 0;
#ifdef RETRANSMITS
	retransmittimer = Timer::GetCurrentTime();
	retransmittimeout = 500 * RuleR(EQStream, RetransmitTimeoutMult); //use 500ms as base before we have connection stats
#endif
	OpMgr = NULL;
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "init Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "init Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
}

EQRawApplicationPacket *EQStream::MakeApplicationPacket(EQProtocolPacket *p)
{
	EQRawApplicationPacket *ap=NULL;
	_log(NET__APP_CREATE, _L "Creating new application packet, length %d" __L, p->size);
	_raw(NET__APP_CREATE_HEX, 0xFFFF, p);
	ap = p->MakeAppPacket();
	return ap;
}

EQRawApplicationPacket *EQStream::MakeApplicationPacket(const unsigned char *buf, uint32 len)
{
	EQRawApplicationPacket *ap=NULL;
	_log(NET__APP_CREATE, _L "Creating new application packet, length %d" __L, len);
	_hex(NET__APP_CREATE_HEX, buf, len);
	ap = new EQRawApplicationPacket(buf, len);
	return ap;
}

EQProtocolPacket *EQStream::MakeProtocolPacket(const unsigned char *buf, uint32 len) {
	uint16 proto_opcode = ntohs(*(const uint16 *)buf);
	
	//advance over opcode.
	buf += 2;
	len -= 2;
	
	return(new EQProtocolPacket(proto_opcode, buf, len));
}

void EQStream::ProcessPacket(EQProtocolPacket *p)
{
uint32 processed=0,subpacket_length=0;
	if (p == NULL)
		return;
	// Raw Application packet
	if (p->opcode > 0xff) {
		p->opcode = htons(p->opcode);  //byte order is backwards in the protocol packet
		EQRawApplicationPacket *ap=MakeApplicationPacket(p);
		if (ap)
			InboundQueuePush(ap);
		return;
	}

	if (!Session && p->opcode!=OP_SessionRequest && p->opcode!=OP_SessionResponse) {
		_log(NET__DEBUG, _L "Session not initialized, packet ignored" __L);
		_raw(NET__DEBUG, 0xFFFF, p);
		return;
	}

	switch (p->opcode) {
		case OP_Combined: {
			processed=0;
			while(processed < p->size) {
				subpacket_length=*(p->pBuffer+processed);
				EQProtocolPacket *subp=MakeProtocolPacket(p->pBuffer+processed+1,subpacket_length);
				_log(NET__NET_CREATE, _L "Extracting combined packet of length %d" __L, subpacket_length);
				_raw(NET__NET_CREATE_HEX, 0xFFFF, subp);
				subp->copyInfo(p);
				ProcessPacket(subp);
				delete subp;
				processed+=subpacket_length+1;
			}
		}
		break;
		
		case OP_AppCombined: {
			processed=0;
			while(processed<p->size) {
				EQRawApplicationPacket *ap=NULL;
				if ((subpacket_length=(unsigned char)*(p->pBuffer+processed))!=0xff) {
					_log(NET__NET_CREATE, _L "Extracting combined app packet of length %d, short len" __L, subpacket_length);
					ap=MakeApplicationPacket(p->pBuffer+processed+1,subpacket_length);
					processed+=subpacket_length+1;
				} else {
					subpacket_length=ntohs(*(uint16 *)(p->pBuffer+processed+1));
					_log(NET__NET_CREATE, _L "Extracting combined app packet of length %d, short len" __L, subpacket_length);
					ap=MakeApplicationPacket(p->pBuffer+processed+3,subpacket_length);
					processed+=subpacket_length+3;
				}
				if (ap) {
					ap->copyInfo(p);
					InboundQueuePush(ap);
				}
			}
		}
		break;
		
		case OP_Packet: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				_log(NET__ERROR, _L "Received OP_Packet that was of malformed size" __L);
				break;
			}
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			SeqOrder check=CompareSequence(NextInSeq,seq);
			if (check == SeqFuture) {
					_log(NET__DEBUG, _L "Future OP_Packet: Expecting Seq=%d, but got Seq=%d" __L, NextInSeq, seq);
					_raw(NET__DEBUG, seq, p);

					PacketQueue[seq]=p->Copy();
					_log(NET__APP_TRACE, _L "OP_Packet Queue size=%d" __L, PacketQueue.size());

				//SendOutOfOrderAck(seq);

			} else if (check == SeqPast) {
				_log(NET__DEBUG, _L "Duplicate OP_Packet: Expecting Seq=%d, but got Seq=%d" __L, NextInSeq, seq);
				_raw(NET__DEBUG, seq, p);
				SendOutOfOrderAck(seq);  //we already got this packet but it was out of order
			} else {
				// In case we did queue one before as well.
				EQProtocolPacket *qp=RemoveQueue(seq);
				if (qp) {
					_log(NET__NET_TRACE, "OP_Packet: Removing older queued packet with sequence %d", seq);
					delete qp;
				}

				SetNextAckToSend(seq);
				NextInSeq++;
				// Check for an embedded OP_AppCombinded (protocol level 0x19)
				if (*(p->pBuffer+2)==0x00 && *(p->pBuffer+3)==0x19) {
					EQProtocolPacket *subp=MakeProtocolPacket(p->pBuffer+2,p->size-2);
					_log(NET__NET_CREATE, _L "seq %d, Extracting combined packet of length %d" __L, seq, subp->size);
					_raw(NET__NET_CREATE_HEX, seq, subp);
					subp->copyInfo(p);
					ProcessPacket(subp);
					delete subp;
				} else {
					EQRawApplicationPacket *ap=MakeApplicationPacket(p->pBuffer+2,p->size-2);
					if (ap) {
						ap->copyInfo(p);
						InboundQueuePush(ap);
					}
				}
			}
		}
		break;
		
		case OP_Fragment: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				_log(NET__ERROR, _L "Received OP_Fragment that was of malformed size" __L);
				break;
			}
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			SeqOrder check=CompareSequence(NextInSeq,seq);
			if (check == SeqFuture) {
				_log(NET__DEBUG, _L "Future OP_Fragment: Expecting Seq=%d, but got Seq=%d" __L, NextInSeq, seq);
				_raw(NET__DEBUG, seq, p);

				PacketQueue[seq]=p->Copy();
				_log(NET__APP_TRACE, _L "OP_Fragment Queue size=%d" __L, PacketQueue.size());

				//SendOutOfOrderAck(seq);

			} else if (check == SeqPast) {
				_log(NET__DEBUG, _L "Duplicate OP_Fragment: Expecting Seq=%d, but got Seq=%d" __L, NextInSeq, seq);
				_raw(NET__DEBUG, seq, p);
				SendOutOfOrderAck(seq);
			} else {
				// In case we did queue one before as well.
				EQProtocolPacket *qp=RemoveQueue(seq);
				if (qp) {
					_log(NET__NET_TRACE, "OP_Fragment: Removing older queued packet with sequence %d", seq);
					delete qp;
				}
				SetNextAckToSend(seq);
				NextInSeq++;
				if (oversize_buffer) {
					memcpy(oversize_buffer+oversize_offset,p->pBuffer+2,p->size-2);
					oversize_offset+=p->size-2;
					_log(NET__NET_TRACE, _L "Fragment of oversized of length %d, seq %d: now at %d/%d" __L, p->size-2, seq, oversize_offset, oversize_length);
					if (oversize_offset==oversize_length) {
						if (*(p->pBuffer+2)==0x00 && *(p->pBuffer+3)==0x19) {
							EQProtocolPacket *subp=MakeProtocolPacket(oversize_buffer,oversize_offset);
							_log(NET__NET_CREATE, _L "seq %d, Extracting combined oversize packet of length %d" __L, seq, subp->size);
							//_raw(NET__NET_CREATE_HEX, subp);
							subp->copyInfo(p);
							ProcessPacket(subp);
							delete subp;
						} else {
							EQRawApplicationPacket *ap=MakeApplicationPacket(oversize_buffer,oversize_offset);
							_log(NET__NET_CREATE, _L "seq %d, completed combined oversize packet of length %d" __L, seq, ap->size);
							if (ap) {
								ap->copyInfo(p);
								InboundQueuePush(ap);
							}
						}
						delete[] oversize_buffer;
						oversize_buffer=NULL;
						oversize_offset=0;
					}
				} else {
					oversize_length=ntohl(*(uint32 *)(p->pBuffer+2));
					oversize_buffer=new unsigned char[oversize_length];
					memcpy(oversize_buffer,p->pBuffer+6,p->size-6);
					oversize_offset=p->size-6;
					_log(NET__NET_TRACE, _L "First fragment of oversized of seq %d: now at %d/%d" __L, seq, oversize_offset, oversize_length);
				}
			}
		}
		break;
		case OP_KeepAlive: {
#ifndef COLLECTOR
			NonSequencedPush(new EQProtocolPacket(p->opcode,p->pBuffer,p->size));
			_log(NET__NET_TRACE, _L "Received and queued reply to keep alive" __L);
#endif
		}
		break;
		case OP_Ack: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				_log(NET__ERROR, _L "Received OP_Ack that was of malformed size" __L);
				break;
			}
#ifndef COLLECTOR
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			AckPackets(seq);
#ifdef RETRANSMITS
			retransmittimer = Timer::GetCurrentTime();
#endif
#endif
		}
		break;
		case OP_SessionRequest: {
			if(p->Size() < sizeof(SessionRequest))
			{
				_log(NET__ERROR, _L "Received OP_SessionRequest that was of malformed size" __L);
				break;
			}
#ifndef COLLECTOR
			if (GetState()==ESTABLISHED) {
				_log(NET__ERROR, _L "Received OP_SessionRequest in ESTABLISHED state (%d)" __L, GetState());
				
				/*RemoveData();
				init(); 
				State=UNESTABLISHED;*/ 
				_SendDisconnect();
				SetState(CLOSED);
				break;
			}
#endif
			//cout << "Got OP_SessionRequest" << endl;
			init();
			OutboundQueueClear();
			SessionRequest *Request=(SessionRequest *)p->pBuffer;
			Session=ntohl(Request->Session);
			SetMaxLen(ntohl(Request->MaxLength));
			_log(NET__NET_TRACE, _L "Received OP_SessionRequest: session %lu, maxlen %d" __L, (unsigned long)Session, MaxLen);
			SetState(ESTABLISHED);
#ifndef COLLECTOR
			Key=0x11223344;
			SendSessionResponse();
#endif
		}
		break;
		case OP_SessionResponse: {
			if(p->Size() < sizeof(SessionResponse))
			{
				_log(NET__ERROR, _L "Received OP_SessionResponse that was of malformed size" __L);
				break;
			}

			init();
			OutboundQueueClear();
			SessionResponse *Response=(SessionResponse *)p->pBuffer;
			SetMaxLen(ntohl(Response->MaxLength));
			Key=ntohl(Response->Key);
			NextInSeq=0;
			SetState(ESTABLISHED);
			if (!Session)
				Session=ntohl(Response->Session);
			compressed=(Response->Format&FLAG_COMPRESSED);
			encoded=(Response->Format&FLAG_ENCODED);

			_log(NET__NET_TRACE, _L "Received OP_SessionResponse: session %lu, maxlen %d, key %lu, compressed? %s, encoded? %s" __L, (unsigned long)Session, MaxLen, (unsigned long)Key, compressed?"yes":"no", encoded?"yes":"no");
			
			// Kinda kludgy, but trie for now
			if (StreamType==UnknownStream) {
				if (compressed) {
					if (remote_port==9000 || (remote_port==0 && p->src_port==9000)) {
						SetStreamType(WorldStream);
					} else {
						SetStreamType(ZoneStream);
					}
				} else if (encoded) {
					SetStreamType(ChatOrMailStream);
				} else {
					SetStreamType(LoginStream);
				}
			}
		}
		break;
		case OP_SessionDisconnect: {
			//NextInSeq=0;
			EQStreamState state = GetState();
			if(state == ESTABLISHED) {
				//client initiated disconnect?
				_log(NET__NET_TRACE, _L "Received unsolicited OP_SessionDisconnect. Treating like a client-initiated disconnect." __L);
				_SendDisconnect();
				SetState(CLOSED);
			} else if(state == CLOSING) {
				//we were waiting for this anyways, ignore pending messages, send the reply and be closed.
				_log(NET__NET_TRACE, _L "Received OP_SessionDisconnect when we have a pending close, they beat us to it. Were happy though." __L);
				_SendDisconnect();
				SetState(CLOSED);
			} else {
				//we are expecting this (or have already gotten it, but dont care either way)
				_log(NET__NET_TRACE, _L "Received expected OP_SessionDisconnect. Moving to closed state." __L);
				SetState(CLOSED);
			}
		}
		break;
		case OP_OutOfOrderAck: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				_log(NET__ERROR, _L "Received OP_OutOfOrderAck that was of malformed size" __L);
				break;
			}
#ifndef COLLECTOR
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			MOutboundQueue.lock();
			
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Pre-OOA Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Pre-OOA Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
			//if the packet they got out of order is between our last acked packet and the last sent packet, then its valid.
			if (CompareSequence(SequencedBase,seq) != SeqPast && CompareSequence(NextOutSeq,seq) == SeqPast) {
				_log(NET__NET_TRACE, _L "Received OP_OutOfOrderAck for sequence %d, starting retransmit at the start of our unacked buffer (seq %d, was %d)." __L,
					seq, SequencedBase, SequencedBase+NextSequencedSend);
#ifdef RETRANSMITS
                        	if (!RuleB(EQStream, RetransmitAckedPackets)) {
#endif
					uint16 sqsize = SequencedQueue.size();
					uint16 index = seq - SequencedBase;
					_log(NET__NET_TRACE, _L "         OP_OutOfOrderAck marking packet acked in queue (queue index = %d, queue size = %d)." __L, index, sqsize);
					if (index < sqsize) {
						deque<EQProtocolPacket *>::iterator sitr;
						sitr = SequencedQueue.begin();
						sitr += index;
						(*sitr)->acked = true;
					}
#ifdef RETRANSMITS
				}
                        	if (RuleR(EQStream, RetransmitTimeoutMult)) { // only choose new behavior if multiplier is set
					retransmittimer = Timer::GetCurrentTime();
				}
#endif
				NextSequencedSend = 0;
			} else {
				_log(NET__NET_TRACE, _L "Received OP_OutOfOrderAck for out-of-window %d. Window (%d->%d)." __L, seq, SequencedBase, NextOutSeq);
			}
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Post-OOA Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Post-OOA Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
			MOutboundQueue.unlock();
#endif
		}
		break;
		case OP_SessionStatRequest: {
			if(p->Size() < sizeof(SessionStats))
			{
				_log(NET__ERROR, _L "Received OP_SessionStatRequest that was of malformed size" __L);
				break;
			}
#ifndef COLLECTOR
			SessionStats *Stats=(SessionStats *)p->pBuffer;
			_log(NET__NET_TRACE, _L "Received Stats: %lu packets received, %lu packets sent, Deltas: local %lu, (%lu <- %lu -> %lu) remote %lu" __L, 
				(unsigned long)ntohl(Stats->packets_received), (unsigned long)ntohl(Stats->packets_sent), (unsigned long)ntohl(Stats->last_local_delta), 
				(unsigned long)ntohl(Stats->low_delta), (unsigned long)ntohl(Stats->average_delta), 
				(unsigned long)ntohl(Stats->high_delta), (unsigned long)ntohl(Stats->last_remote_delta));
			uint64 x=Stats->packets_received;
			Stats->packets_received=Stats->packets_sent;
			Stats->packets_sent=x;
			NonSequencedPush(new EQProtocolPacket(OP_SessionStatResponse,p->pBuffer,p->size));
			AdjustRates(ntohl(Stats->average_delta));
#ifdef RETRANSMITS
			if (RuleR(EQStream, RetransmitTimeoutMult) && ntohl(Stats->average_delta)) {
				//recalculate retransmittimeout using the larger of the last rtt or average rtt, which is multiplied by the rule value
				if((ntohl(Stats->last_local_delta) + ntohl(Stats->last_remote_delta)) > (ntohl(Stats->average_delta) * 2)) {
					retransmittimeout = (ntohl(Stats->last_local_delta) + ntohl(Stats->last_remote_delta)) * RuleR(EQStream, RetransmitTimeoutMult);
				} else {
					retransmittimeout = ntohl(Stats->average_delta) * 2 * RuleR(EQStream, RetransmitTimeoutMult);
				}
				if(retransmittimeout > RuleI(EQStream, RetransmitTimeoutMax))
					retransmittimeout = RuleI(EQStream, RetransmitTimeoutMax);
				_log(NET__NET_TRACE, _L "Retransmit timeout recalculated to %dms" __L, retransmittimeout);
			}
#endif
#endif
		}
		break;
		case OP_SessionStatResponse: {
			_log(NET__NET_TRACE, _L "Received OP_SessionStatResponse. Ignoring." __L);
		}
		break;
		case OP_OutOfSession: {
			_log(NET__NET_TRACE, _L "Received OP_OutOfSession. Ignoring." __L);
		}
		break;
		default:
			EQRawApplicationPacket *ap = MakeApplicationPacket(p);
			if (ap)
				InboundQueuePush(ap);
			break;
	}
}

void EQStream::QueuePacket(const EQApplicationPacket *p, bool ack_req)
{
	if(p == NULL)
		return;
	
	EQApplicationPacket *newp = p->Copy();

	if (newp != NULL)
		FastQueuePacket(&newp, ack_req);
}

void EQStream::FastQueuePacket(EQApplicationPacket **p, bool ack_req)
{
	EQApplicationPacket *pack=*p;
	*p = NULL;		//clear caller's pointer.. effectively takes ownership
	
	if(pack == NULL)
		return;
	
	if(OpMgr == NULL || *OpMgr == NULL) {
		_log(NET__DEBUG, _L "Packet enqueued into a stream with no opcode manager, dropping." __L);
		delete pack;
		return;
	}
	
	uint16 opcode = (*OpMgr)->EmuToEQ(pack->emu_opcode);
	
	//make sure this packet is compatible with this stream
/*	if(StreamType == UnknownStream || StreamType == ChatOrMailStream) {
		_log(NET__DEBUG, _L "Stream type undetermined (%s), packet ignored" __L, StreamTypeString(StreamType));
		return;
	}
	if(pack->GetPacketType() != StreamType) {
		_log(NET__ERROR, _L "Trying to queue a packet of type %s into a stream of type %s, dropping it." __L, StreamTypeString(pack->GetPacketType()), StreamTypeString(StreamType));
		return;
	}*/
	
	_log(NET__APP_TRACE, "Queueing %sacked packet with opcode 0x%x (%s) and length %d", ack_req?"":"non-", opcode, OpcodeManager::EmuToName(pack->emu_opcode), pack->size);
	
	if (!ack_req) {
		NonSequencedPush(new EQProtocolPacket(opcode, pack->pBuffer, pack->size));
		delete pack;
	} else {
		SendPacket(opcode, pack);
	}
}

void EQStream::SendPacket(uint16 opcode, EQApplicationPacket *p)
{
uint32 chunksize,used;
uint32 length;

	// Convert the EQApplicationPacket to 1 or more EQProtocolPackets
	if (p->size>(MaxLen-8)) { // proto-op(2), seq(2), app-op(2) ... data ... crc(2)
		_log(NET__FRAGMENT, _L "Making oversized packet, len %d" __L, p->size);

		unsigned char *tmpbuff=new unsigned char[p->size+3];
		length=p->serialize(opcode, tmpbuff);
		
		EQProtocolPacket *out=new EQProtocolPacket(OP_Fragment,NULL,MaxLen-4);
		*(uint32 *)(out->pBuffer+2)=htonl(p->Size());
		used=MaxLen-10;
		memcpy(out->pBuffer+6,tmpbuff,used);
		_log(NET__FRAGMENT, _L "First fragment: used %d/%d. Put size %d in the packet" __L, used, p->size, p->Size());
		SequencedPush(out);
		
		
		while (used<length) {
			out=new EQProtocolPacket(OP_Fragment,NULL,MaxLen-4);
			chunksize=min(length-used,MaxLen-6);
			memcpy(out->pBuffer+2,tmpbuff+used,chunksize);
			out->size=chunksize+2;
			SequencedPush(out);
			used+=chunksize;
			_log(NET__FRAGMENT, _L "Subsequent fragment: len %d, used %d/%d." __L, chunksize, used, p->size);
		}
		delete p;
		delete[] tmpbuff;
	} else {

		unsigned char *tmpbuff=new unsigned char[p->Size()+3];
		length=p->serialize(opcode, tmpbuff+2) + 2;

		EQProtocolPacket *out=new EQProtocolPacket(OP_Packet,tmpbuff,length);

		delete[] tmpbuff;
		SequencedPush(out);
		delete p;
	}
}

void EQStream::SequencedPush(EQProtocolPacket *p)
{
#ifdef COLLECTOR
	delete p;
#else
	MOutboundQueue.lock();
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Pre-Push Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Pre-Push Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}

	_log(NET__APP_TRACE, _L "Pushing sequenced packet %d of length %d. Base Seq is %d." __L, NextOutSeq, p->size, SequencedBase);
	*(uint16 *)(p->pBuffer)=htons(NextOutSeq);
	SequencedQueue.push_back(p);
	NextOutSeq++;

if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Push Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Push Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
	MOutboundQueue.unlock();
#endif
}

void EQStream::NonSequencedPush(EQProtocolPacket *p)
{
#ifdef COLLECTOR
	delete p;
#else
	MOutboundQueue.lock();
	_log(NET__APP_TRACE, _L "Pushing non-sequenced packet of length %d" __L, p->size);
	NonSequencedQueue.push(p);
	MOutboundQueue.unlock();
#endif
}

void EQStream::SendAck(uint16 seq)
{
uint16 Seq=htons(seq);
	_log(NET__NET_ACKS, _L "Sending ack with sequence %d" __L, seq);
	SetLastAckSent(seq);
	NonSequencedPush(new EQProtocolPacket(OP_Ack,(unsigned char *)&Seq,sizeof(uint16)));
}

void EQStream::SendOutOfOrderAck(uint16 seq)
{
	_log(NET__APP_TRACE, _L "Sending out of order ack with sequence %d" __L, seq);
uint16 Seq=htons(seq);
	NonSequencedPush(new EQProtocolPacket(OP_OutOfOrderAck,(unsigned char *)&Seq,sizeof(uint16)));
}

void EQStream::Write(int eq_fd)
{
queue<EQProtocolPacket *> ReadyToSend;
bool SeqEmpty=false,NonSeqEmpty=false;
deque<EQProtocolPacket *>::iterator sitr;

	// Check our rate to make sure we can send more
	MRate.lock();
	int32 threshold=RateThreshold;
	MRate.unlock();
	if (BytesWritten > threshold) {
		//cout << "Over threshold: " << BytesWritten << " > " << threshold << endl;
		return;
	}

	// If we got more packets to we need to ack, send an ack on the highest one
	MAcks.lock();
	if (CompareSequence(LastAckSent, NextAckToSend) == SeqFuture)
		SendAck(NextAckToSend);
	MAcks.unlock();

	// Lock the outbound queues while we process
	MOutboundQueue.lock();

	// Place to hold the base packet t combine into
	EQProtocolPacket *p=NULL;

#ifdef RETRANSMITS
	// if we have a timeout defined and we have not received an ack recently enough, retransmit from beginning of queue
	if (RuleR(EQStream, RetransmitTimeoutMult) && !SequencedQueue.empty() && NextSequencedSend && (GetState()==ESTABLISHED) && ((retransmittimer+retransmittimeout) < Timer::GetCurrentTime())) {
		_log(NET__NET_TRACE, _L "Timeout since last ack received, starting retransmit at the start of our unacked buffer (seq %d, was %d)." __L, SequencedBase, SequencedBase+NextSequencedSend);
		NextSequencedSend = 0;
		retransmittimer = Timer::GetCurrentTime(); // don't want to endlessly retransmit the first packet
	}
#endif

	// Find the next sequenced packet to send from the "queue"
	sitr = SequencedQueue.begin();
	if (sitr!=SequencedQueue.end())
	sitr += NextSequencedSend;
	
	// Loop until both are empty or MaxSends is reached
	while(!SeqEmpty || !NonSeqEmpty)  {

		// See if there are more non-sequenced packets left
		if (!NonSequencedQueue.empty()) {
			if (!p) {
				// If we don't have a packet to try to combine into, use this one as the base
				// And remove it form the queue
				p = NonSequencedQueue.front();
				_log(NET__NET_COMBINE, _L "Starting combined packet with non-seq packet of len %d" __L, p->size);
				NonSequencedQueue.pop();
			} else if (!p->combine(NonSequencedQueue.front())) {
				// Tryint to combine this packet with the base didn't work (too big maybe)
				// So just send the base packet (we'll try this packet again later)
				_log(NET__NET_COMBINE, _L "Combined packet full at len %d, next non-seq packet is len %d" __L, p->size, (NonSequencedQueue.front())->size);
				ReadyToSend.push(p);
				BytesWritten+=p->size;
				p=NULL;
				
				if (BytesWritten > threshold) {
					// Sent enough this round, lets stop to be fair
					_log(NET__RATES, _L "Exceeded write threshold in nonseq (%d > %d)" __L, BytesWritten, threshold);
					break;
				}
			} else {
				// Combine worked, so just remove this packet and it's spot in the queue
				_log(NET__NET_COMBINE, _L "Combined non-seq packet of len %d, yeilding %d combined." __L, (NonSequencedQueue.front())->size, p->size);
				delete NonSequencedQueue.front();
				NonSequencedQueue.pop();
			}
		} else {
			// No more non-sequenced packets
			NonSeqEmpty=true;
		}

		if (sitr!=SequencedQueue.end()) {
//_log(NET__NET_COMBINE, _L "Send Seq with %d seq packets starting at seq %d, next send %d, and %d non-seq packets." __L, 
//	SequencedQueue.size(), SequencedBase, NextSequencedSend, NonSequencedQueue.size());
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Pre-Send Seq NSS=%d Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, NextSequencedSend, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Pre-Send Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
			uint16 seq_send = SequencedBase + NextSequencedSend;	//just for logging...
if(SequencedQueue.empty()) {
_log(NET__ERROR, _L "Tried to write a packet with an empty queue (%d is past next out %d)" __L, seq_send, NextOutSeq);
SeqEmpty=true;
continue;
}
/*if(CompareSequence(NextOutSeq, seq_send) == SeqFuture) {
_log(NET__ERROR, _L "Tried to write a packet beyond the end of the queue! (%d is past next out %d)" __L, seq_send, NextOutSeq);
sitr=SequencedQueue.end();
continue;
}*/
#ifdef RETRANSMITS
			if (!RuleB(EQStream, RetransmitAckedPackets) && (*sitr)->acked) {
				_log(NET__NET_TRACE, _L "Not retransmitting seq packet %d because already marked as acked" __L, seq_send);
				sitr++;
				NextSequencedSend++;
			} else if (!p) {
#else
			if (!p) {
#endif
				// If we don't have a packet to try to combine into, use this one as the base
				// Copy it first as it will still live until it is acked
				p=(*sitr)->Copy();
				_log(NET__NET_COMBINE, _L "Starting combined packet with seq packet %d of len %d" __L, seq_send, p->size);
				sitr++;
				NextSequencedSend++;
			} else if (!p->combine(*sitr)) {
				// Trying to combine this packet with the base didn't work (too big maybe)
				// So just send the base packet (we'll try this packet again later)
				_log(NET__NET_COMBINE, _L "Combined packet full at len %d, next seq packet %d is len %d" __L, p->size, seq_send, (*sitr)->size);
				ReadyToSend.push(p);
				BytesWritten+=p->size;
				p=NULL;

				if (BytesWritten > threshold) {
					// Sent enough this round, lets stop to be fair
					_log(NET__RATES, _L "Exceeded write threshold in seq (%d > %d)" __L, BytesWritten, threshold);
					break;
				}
			} else {
				// Combine worked
				_log(NET__NET_COMBINE, _L "Combined seq packet %d of len %d, yeilding %d combined." __L, seq_send, (*sitr)->size, p->size);
				sitr++;
				NextSequencedSend++;
			}
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Post send Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Post send Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
		} else {
			// No more sequenced packets
			SeqEmpty=true;
		}
	}
	// Unlock the queue
	MOutboundQueue.unlock();

	// We have a packet still, must have run out of both seq and non-seq, so send it
	if (p) {
		_log(NET__NET_COMBINE, _L "Final combined packet not full, len %d" __L, p->size);
		ReadyToSend.push(p);
		BytesWritten+=p->size;
	}

	// Send all the packets we "made"
	while(!ReadyToSend.empty()) {
		p = ReadyToSend.front();
		WritePacket(eq_fd,p);
		delete p;
		ReadyToSend.pop();
	}
	
	//see if we need to send our disconnect and finish our close
	if(SeqEmpty && NonSeqEmpty) {
		//no more data to send
		if(CheckState(CLOSING)) {
			_log(NET__DEBUG, _L "All outgoing data flushed, closing stream." __L );
			//we are waiting for the queues to empty, now we can do our disconnect.
			//this packet will not actually go out until the next call to Write().
			_SendDisconnect();
			SetState(DISCONNECTING);
		}
	}
}

void EQStream::WritePacket(int eq_fd, EQProtocolPacket *p)
{
uint32 length;
sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr=remote_ip;
	address.sin_port=remote_port;
#ifdef NOWAY
	uint32 ip=address.sin_addr.s_addr;
	cout << "Sending to: " 
		<< (int)*(unsigned char *)&ip
		<< "." << (int)*((unsigned char *)&ip+1)
		<< "." << (int)*((unsigned char *)&ip+2)
		<< "." << (int)*((unsigned char *)&ip+3)
		<< "," << (int)ntohs(address.sin_port) << "(" << p->size << ")" << endl;

	p->DumpRaw();
	cout << "-------------" << endl;
#endif
	length=p->serialize(buffer);
	if (p->opcode!=OP_SessionRequest && p->opcode!=OP_SessionResponse) {
		if (compressed) {
			uint32 newlen=EQProtocolPacket::Compress(buffer,length, _tempBuffer, 2048);
			memcpy(buffer,_tempBuffer,newlen);
			length=newlen;
		}
		if (encoded) {
			EQProtocolPacket::ChatEncode(buffer,length,Key);
		}

		*(uint16 *)(buffer+length)=htons(CRC16(buffer,length,Key));
		length+=2;
	}
	//dump_message_column(buffer,length,"Writer: ");
	sendto(eq_fd,(char *)buffer,length,0,(sockaddr *)&address,sizeof(address));
	AddBytesSent(length);
}

/*
commented out since im not sure theres a lot of merit in it.
Really it was bitterness towards allocating a 2k buffer on the stack each call.
Im sure the thought was client side, but even then, they will
likely need a whole thread to call this method, in which case, they should
supply the buffer so we dont re-allocate it each time.
EQProtocolPacket *EQStream::Read(int eq_fd, sockaddr_in *from)
{
int socklen;
int length=0;
EQProtocolPacket *p=NULL;
char temp[15];

	socklen=sizeof(sockaddr);
#ifdef _WINDOWS
	length=recvfrom(eq_fd, (char *)_tempBuffer, 2048, 0, (struct sockaddr*)from, (int *)&socklen);
#else
	length=recvfrom(eq_fd, _tempBuffer, 2048, 0, (struct sockaddr*)from, (socklen_t *)&socklen);
#endif

	if (length>=2) {
		p=new EQProtocolPacket(_tempBuffer[1],&_tempBuffer[2],length-2);

		uint32 ip=from->sin_addr.s_addr;
		sprintf(temp,"%d.%d.%d.%d:%d",
			*(unsigned char *)&ip,
			*((unsigned char *)&ip+1),
			*((unsigned char *)&ip+2),
			*((unsigned char *)&ip+3),
			ntohs(from->sin_port));
		//cout << timestamp() << "Data from: " << temp << " OpCode 0x" << hex << setw(2) << setfill('0') << (int)p->opcode << dec << endl;
		//dump_message(p->pBuffer,p->size,timestamp());
		
	}
	return p;
}*/

void EQStream::SendSessionResponse()
{
EQProtocolPacket *out=new EQProtocolPacket(OP_SessionResponse,NULL,sizeof(SessionResponse));
	SessionResponse *Response=(SessionResponse *)out->pBuffer;
	Response->Session=htonl(Session);
	Response->MaxLength=htonl(MaxLen);
	Response->UnknownA=2;
	Response->Format=0;
	if (compressed)
		Response->Format|=FLAG_COMPRESSED;
	if (encoded)
		Response->Format|=FLAG_ENCODED;
	Response->Key=htonl(Key);
	
	out->size=sizeof(SessionResponse);
	
	_log(NET__NET_TRACE, _L "Sending OP_SessionResponse: session %lu, maxlen=%d, key=0x%x, compressed? %s, encoded? %s" __L,
		(unsigned long)Session, MaxLen, Key, compressed?"yes":"no", encoded?"yes":"no");
	
	NonSequencedPush(out);
}

void EQStream::SendSessionRequest()
{
EQProtocolPacket *out=new EQProtocolPacket(OP_SessionRequest,NULL,sizeof(SessionRequest));
	SessionRequest *Request=(SessionRequest *)out->pBuffer;
	memset(Request,0,sizeof(SessionRequest));
	Request->Session=htonl(time(NULL));
	Request->MaxLength=htonl(512);
	
	_log(NET__NET_TRACE, _L "Sending OP_SessionRequest: session %lu, maxlen=%d" __L, (unsigned long)ntohl(Request->Session), ntohl(Request->MaxLength));
	
	NonSequencedPush(out);
}

void EQStream::_SendDisconnect()
{
	if(GetState() == CLOSED)
		return;
	
	EQProtocolPacket *out=new EQProtocolPacket(OP_SessionDisconnect,NULL,sizeof(uint32));
	*(uint32 *)out->pBuffer=htonl(Session);
	NonSequencedPush(out);
	
	_log(NET__NET_TRACE, _L "Sending OP_SessionDisconnect: session %lu" __L, (unsigned long)Session);
}

void EQStream::InboundQueuePush(EQRawApplicationPacket *p)
{
	MInboundQueue.lock();
	InboundQueue.push_back(p);
	MInboundQueue.unlock();
}

EQApplicationPacket *EQStream::PopPacket()
{
EQRawApplicationPacket *p=NULL;

	MInboundQueue.lock();
	if (InboundQueue.size()) {
		vector<EQRawApplicationPacket *>::iterator itr=InboundQueue.begin();
		p=*itr;
		InboundQueue.erase(itr);
	}
	MInboundQueue.unlock();
	
	//resolve the opcode if we can.
	if(p) {
		if(OpMgr != NULL && *OpMgr != NULL) {
			EmuOpcode emu_op = (*OpMgr)->EQToEmu(p->opcode);
#if EQDEBUG >= 4
			if(emu_op == OP_Unknown) {
				LogFile->write(EQEMuLog::Debug, "Unable to convert EQ opcode 0x%.4x to an Application opcode.", p->opcode);
			}
#endif
			p->SetOpcode(emu_op);
		}
	}
	
	return p;
}

EQRawApplicationPacket *EQStream::PopRawPacket()
{
EQRawApplicationPacket *p=NULL;

	MInboundQueue.lock();
	if (InboundQueue.size()) {
		vector<EQRawApplicationPacket *>::iterator itr=InboundQueue.begin();
		p=*itr;
		InboundQueue.erase(itr);
	}
	MInboundQueue.unlock();
	
	//resolve the opcode if we can.
	if(p) {
		if(OpMgr != NULL && *OpMgr != NULL) {
			EmuOpcode emu_op = (*OpMgr)->EQToEmu(p->opcode);
#if EQDEBUG >= 4
			if(emu_op == OP_Unknown) {
				LogFile->write(EQEMuLog::Debug, "Unable to convert EQ opcode 0x%.4x to an Application opcode.", p->opcode);
			}
#endif
			p->SetOpcode(emu_op);
		}
	}

	return p;
}

EQRawApplicationPacket *EQStream::PeekPacket()
{
EQRawApplicationPacket *p=NULL;

	MInboundQueue.lock();
	if (InboundQueue.size()) {
		vector<EQRawApplicationPacket *>::iterator itr=InboundQueue.begin();
		p=*itr;
	}
	MInboundQueue.unlock();

	return p;
}

void EQStream::InboundQueueClear()
{
EQApplicationPacket *p=NULL;
	
	_log(NET__APP_TRACE, _L "Clearing inbound queue" __L);
	
	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		vector<EQRawApplicationPacket *>::iterator itr;
		for(itr=InboundQueue.begin();itr!=InboundQueue.end();itr++) {
			p=*itr;
			delete p;
		}
		InboundQueue.clear();
	}
	MInboundQueue.unlock();
}

bool EQStream::HasOutgoingData()
{
bool flag;
	
	//once closed, we have nothing more to say
	if(CheckClosed())
		return(false);
	
	MOutboundQueue.lock();
	flag=(!NonSequencedQueue.empty());
	if (!flag) {
		//not only wait until we send it all, but wait until they ack everything.
		flag = !SequencedQueue.empty();
	}
	MOutboundQueue.unlock();

	if (!flag) {
		MAcks.lock();
		flag= (NextAckToSend>LastAckSent);
		MAcks.unlock();
	}

	return flag;
}

void EQStream::OutboundQueueClear()
{
EQProtocolPacket *p=NULL;

	_log(NET__APP_TRACE, _L "Clearing outbound queue" __L);
	
	MOutboundQueue.lock();
	while(!NonSequencedQueue.empty()) {
		delete NonSequencedQueue.front();
		NonSequencedQueue.pop();
	}
	if(!SequencedQueue.empty()) {
		deque<EQProtocolPacket *>::iterator itr;
		for(itr=SequencedQueue.begin();itr!=SequencedQueue.end();itr++) {
			p=*itr;
			delete p;
		}
		SequencedQueue.clear();
	}
	MOutboundQueue.unlock();
	
/*if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Out-bound Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Out-bound Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}*/
	//NOTE: we prolly want to reset counters if we are stupposed to do anything after this.
}

void EQStream::PacketQueueClear()
{
EQProtocolPacket *p=NULL;

	_log(NET__APP_TRACE, _L "Clearing future packet queue" __L);
	
	if(!PacketQueue.empty()) {
		map<unsigned short,EQProtocolPacket *>::iterator itr;
		for(itr=PacketQueue.begin();itr!=PacketQueue.end();itr++) {
			p=itr->second;
			delete p;
		}
		PacketQueue.clear();
	}
}

void EQStream::Process(const unsigned char *buffer, const uint32 length)
{
static unsigned char newbuffer[2048];
uint32 newlength=0;
	if (EQProtocolPacket::ValidateCRC(buffer,length,Key)) {
		if (compressed) {
			newlength=EQProtocolPacket::Decompress(buffer,length,newbuffer,2048);
		} else {
			memcpy(newbuffer,buffer,length);
			newlength=length;
			if (encoded)
				EQProtocolPacket::ChatDecode(newbuffer,newlength-2,Key);
		}
		if (buffer[1]!=0x01 && buffer[1]!=0x02 && buffer[1]!=0x1d)
			newlength-=2;
		EQProtocolPacket *p = MakeProtocolPacket(newbuffer,newlength);
		ProcessPacket(p);
		delete p;
		ProcessQueue();
	} else {
		_log(NET__DEBUG, _L "Incoming packet failed checksum" __L);
		_hex(NET__NET_CREATE_HEX, buffer, length);
	}
}

long EQStream::GetNextAckToSend()
{
	MAcks.lock();
	long l=NextAckToSend;
	MAcks.unlock();

	return l;
}

long EQStream::GetLastAckSent()
{
	MAcks.lock();
	long l=LastAckSent;
	MAcks.unlock();

	return l;
}

void EQStream::AckPackets(uint16 seq)
{
deque<EQProtocolPacket *>::iterator itr, tmp;
	
	MOutboundQueue.lock();
//do a bit of sanity checking.
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Pre-Ack Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Pre-Ack Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
	
	SeqOrder ord = CompareSequence(SequencedBase, seq);
	if(ord == SeqInOrder) {
		//they are not acking anything new...
		_log(NET__NET_ACKS, _L "Received an ack with no window advancement (seq %d)." __L, seq);
	} else if(ord == SeqPast) {
		//they are nacking blocks going back before our buffer, wtf?
		_log(NET__NET_ACKS, _L "Received an ack with backward window advancement (they gave %d, our window starts at %d). This is bad." __L, seq, SequencedBase);
	} else {
		_log(NET__NET_ACKS, _L "Received an ack up through sequence %d. Our base is %d." __L, seq, SequencedBase);


		//this is a good ack, we get to ack some blocks.
		seq++;	//we stop at the block right after their ack, counting on the wrap of both numbers.
		while(SequencedBase != seq) {
if(SequencedQueue.empty()) {
_log(NET__ERROR, _L "OUT OF PACKETS acked packet with sequence %lu. Next send is %d before this." __L, (unsigned long)SequencedBase, NextSequencedSend);
	SequencedBase = NextOutSeq;
	NextSequencedSend = 0;
	break;
}
			_log(NET__NET_ACKS, _L "Removing acked packet with sequence %lu. Next send is %d before this." __L, (unsigned long)SequencedBase, NextSequencedSend);
			//clean out the acked packet
			delete SequencedQueue.front();
			SequencedQueue.pop_front();
			//adjust our "next" pointer
			if(NextSequencedSend > 0)
				NextSequencedSend--;
			//advance the base sequence number to the seq of the block after the one we just got rid of.
			SequencedBase++;
		}
if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
	_log(NET__ERROR, _L "Post-Ack on %d Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L, seq, SequencedBase, SequencedQueue.size(), NextOutSeq);
}
if(NextSequencedSend > SequencedQueue.size()) {
	_log(NET__ERROR, _L "Post-Ack Next Send Sequence is beyond the end of the queue NSS %d > SQ %d" __L, NextSequencedSend, SequencedQueue.size());
}
	}
	
	MOutboundQueue.unlock();
}

void EQStream::SetNextAckToSend(uint32 seq)
{
	MAcks.lock();
	_log(NET__NET_ACKS, _L "Set Next Ack To Send to %lu" __L, (unsigned long)seq);
	NextAckToSend=seq;
	MAcks.unlock();
}

void EQStream::SetLastAckSent(uint32 seq)
{
	MAcks.lock();
	_log(NET__NET_ACKS, _L "Set Last Ack Sent to %lu" __L, (unsigned long)seq);
	LastAckSent=seq;
	MAcks.unlock();
}

void EQStream::ProcessQueue()
{
	if(PacketQueue.empty()) {
		return;
	}
	
	EQProtocolPacket *qp=NULL;
	while((qp=RemoveQueue(NextInSeq))!=NULL) {
		_log(NET__DEBUG, _L "Processing Queued Packet: Seq=%d" __L, NextInSeq);
		ProcessPacket(qp);
		delete qp;
		_log(NET__APP_TRACE, _L "OP_Packet Queue size=%d" __L, PacketQueue.size());
	}
}

EQProtocolPacket *EQStream::RemoveQueue(uint16 seq)
{
map<unsigned short,EQProtocolPacket *>::iterator itr;
EQProtocolPacket *qp=NULL;
	if ((itr=PacketQueue.find(seq))!=PacketQueue.end()) {
		qp=itr->second;
		PacketQueue.erase(itr);
		_log(NET__APP_TRACE, _L "OP_Packet Queue size=%d" __L, PacketQueue.size());
	}
	return qp;
}

void EQStream::SetStreamType(EQStreamType type)
{
	_log(NET__NET_TRACE, _L "Changing stream type from %s to %s" __L, StreamTypeString(StreamType), StreamTypeString(type));
	StreamType=type;
	switch (StreamType) {
		case LoginStream:
			app_opcode_size=1;
			compressed=false;
			encoded=false;
			_log(NET__NET_TRACE, _L "Login stream has app opcode size %d, is not compressed or encoded." __L, app_opcode_size);
			break;
		case ChatOrMailStream:
		case ChatStream:
		case MailStream:
			app_opcode_size=1;
			compressed=false;
			encoded=true;
			_log(NET__NET_TRACE, _L "Chat/Mail stream has app opcode size %d, is not compressed, and is encoded." __L, app_opcode_size);
			break;
		case ZoneStream:
		case WorldStream:
		default:
			app_opcode_size=2;
			compressed=true;
			encoded=false;
			_log(NET__NET_TRACE, _L "World/Zone stream has app opcode size %d, is compressed, and is not encoded." __L, app_opcode_size);
			break;
	}
}

const char *EQStream::StreamTypeString(EQStreamType t)
{
	switch (t) {
		case LoginStream:
			return "Login";
			break;
		case WorldStream:
			return "World";
			break;
		case ZoneStream:
			return "Zone";
			break;
		case ChatOrMailStream:
			return "Chat/Mail";
			break;
		case ChatStream:
			return "Chat";
			break;
		case MailStream:
			return "Mail";
			break;
		case UnknownStream:
			return "Unknown";
			break;
	}
	return "UnknownType";
}

//returns SeqFuture if `seq` is later than `expected_seq`
EQStream::SeqOrder EQStream::CompareSequence(uint16 expected_seq , uint16 seq)
{
	if (expected_seq==seq) {
		// Curent
		return SeqInOrder;
	}  else if ((seq > expected_seq && (uint32)seq < ((uint32)expected_seq + EQStream::MaxWindowSize)) || seq < (expected_seq - EQStream::MaxWindowSize)) {
		// Future
		return SeqFuture;
	} else {
		// Past
		return SeqPast;
	}
}

void EQStream::SetState(EQStreamState state) {
	MState.lock();
	_log(NET__NET_TRACE, _L "Changing state from %d to %d" __L, State, state);
	State=state;
	MState.unlock();
}


void EQStream::CheckTimeout(uint32 now, uint32 timeout) {
	
	bool outgoing_data = HasOutgoingData();	//up here to avoid recursive locking
	
	EQStreamState orig_state = GetState();
	if (orig_state == CLOSING && !outgoing_data) {
		_log(NET__NET_TRACE, _L "Out of data in closing state, disconnecting." __L);
		_SendDisconnect();
		SetState(DISCONNECTING);
	} else if (LastPacket && (now-LastPacket) > timeout) {
		switch(orig_state) {
		case CLOSING:
			//if we time out in the closing state, they are not acking us, just give up
			_log(NET__DEBUG, _L "Timeout expired in closing state. Moving to closed state." __L);
			_SendDisconnect();
			SetState(CLOSED);
			break;
		case DISCONNECTING:
			//we timed out waiting for them to send us the disconnect reply, just give up.
			_log(NET__DEBUG, _L "Timeout expired in disconnecting state. Moving to closed state." __L);
			SetState(CLOSED);
			break;
		case CLOSED:
			_log(NET__DEBUG, _L "Timeout expired in closed state??" __L);
			break;
		case ESTABLISHED:
			//we timed out during normal operation. Try to be nice about it.
			//we will almost certainly time out again waiting for the disconnect reply, but oh well.
			_log(NET__DEBUG, _L "Timeout expired in established state. Closing connection." __L);
			_SendDisconnect();
			SetState(DISCONNECTING);
		}
	}
}

void EQStream::Decay()
{
	MRate.lock();
	uint32 rate=DecayRate;
	MRate.unlock();
	if (BytesWritten>0) {
		BytesWritten-=rate;
		if (BytesWritten<0)
			BytesWritten=0;
	}
}

void EQStream::AdjustRates(uint32 average_delta)
{
#ifdef RETRANSMITS
	if (average_delta && (average_delta <= RuleI(EQStream, AverageDeltaMax))) {
#else
	if (average_delta) {
#endif
		MRate.lock();
		RateThreshold=RATEBASE/average_delta;
		DecayRate=DECAYBASE/average_delta;
		_log(NET__RATES, _L "Adjusting data rate to thresh %d, decay %d based on avg delta %d" __L, RateThreshold, DecayRate, average_delta);
		MRate.unlock();
#ifdef RETRANSMITS
	} else {
		_log(NET__RATES, _L "Not adjusting data rate because avg delta over max (%d > %d)" __L, average_delta, RuleI(EQStream, AverageDeltaMax));
#endif
	}
}

void EQStream::Close() {
	if(HasOutgoingData()) {
		//there is pending data, wait for it to go out.
		_log(NET__DEBUG, _L "Stream requested to Close(), but there is pending data, waiting for it." __L);
		SetState(CLOSING);
	} else {
		//otherwise, we are done, we can drop immediately.
		_SendDisconnect();
		_log(NET__DEBUG, _L "Stream closing immediate due to Close()" __L);
		SetState(DISCONNECTING);
	}
}


//this could be expanded to check more than the fitst opcode if
//we needed more complex matching
EQStream::MatchState EQStream::CheckSignature(const Signature *sig) {
	EQRawApplicationPacket *p = NULL;
	MatchState res = MatchNotReady;
	
	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		//this is already getting hackish...
		p = InboundQueue.front();
		if(sig->ignore_eq_opcode != 0 && p->opcode == sig->ignore_eq_opcode) {
			if(InboundQueue.size() > 1) {
				p = InboundQueue[1];
			} else {
				p = NULL;
			}
		}
		if(p == NULL) {
			//first opcode is ignored, and nothing else remains... keep waiting
		} else if(p->opcode == sig->first_eq_opcode) {
			//opcode matches, check length..
			if(p->size == sig->first_length) {
				_log(NET__IDENT_TRACE, "%s:%d: First opcode matched 0x%x and length matched %d", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), sig->first_eq_opcode, p->size);
				res = MatchSuccessful;
			} else if(sig->first_length == 0) {
				_log(NET__IDENT_TRACE, "%s:%d: First opcode matched 0x%x and length (%d) is ignored", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), sig->first_eq_opcode, p->size);
				res = MatchSuccessful;
			} else {
				//opcode matched but length did not.
				_log(NET__IDENT_TRACE, "%s:%d: First opcode matched 0x%x, but length %d did not match expected %d", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), sig->first_eq_opcode, p->size, sig->first_length);
				res = MatchFailed;
			}
		} else {
			//first opcode did not match..
			_log(NET__IDENT_TRACE, "%s:%d: First opcode 0x%x did not match expected 0x%x", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), p->opcode, sig->first_eq_opcode);
			res = MatchFailed;
		}
	}
	MInboundQueue.unlock();
	
	return(res);
}




