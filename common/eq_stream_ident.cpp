#include <utility>

#include "global_define.h"
#include "eqemu_logsys.h"
#include "eq_stream_ident.h"
#include "eq_stream_proxy.h"


EQStreamIdentifier::~EQStreamIdentifier() {
	while(!m_identified.empty()) {
		m_identified.front()->ReleaseFromUse();
		m_identified.pop();
	}
	std::vector<Record>::iterator cur, end;
	cur = m_streams.begin();
	end = m_streams.end();
	for(; cur != end; ++cur) {
		Record &r = *cur;
		r.stream->ReleaseFromUse();
	}
	std::vector<Patch *>::iterator curp, endp;
	curp = m_patches.begin();
	endp = m_patches.end();
	for(; curp != endp; ++curp) {
		delete *curp;
	}
}

void EQStreamIdentifier::RegisterPatch(const EQStream::Signature &sig, const char *name, OpcodeManager ** opcodes, const StructStrategy *structs) {
	auto p = new Patch;
	p->signature = sig;
	p->name = name;
	p->opcodes = opcodes;
	p->structs = structs;
	m_patches.push_back(p);
}

void EQStreamIdentifier::Process() {
	std::vector<Record>::iterator cur;
	std::vector<Patch *>::iterator curp, endp;

	//foreach pending stream.
	cur = m_streams.begin();
	while(cur != m_streams.end()) {
		Record &r = *cur;

		//first see if this stream has expired
		if(r.expire.Check(false)) {
			//this stream has failed to match any pattern in our timeframe.
			Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Unable to identify stream from %s:%d before timeout.", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()));
			r.stream->ReleaseFromUse();
			cur = m_streams.erase(cur);
			continue;
		}

		//then make sure the stream is still active
		//if stream hasn't finished initializing then continue;
		if(r.stream->GetState() == UNESTABLISHED)
		{
			++cur;
			continue;
		}
		if(r.stream->GetState() != ESTABLISHED) {
			//the stream closed before it was identified.
			Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Unable to identify stream from %s:%d before it closed.", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()));
			switch(r.stream->GetState())
			{
			case ESTABLISHED:
				Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Stream state was Established");
				break;
			case CLOSING:
				Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Stream state was Closing");
				break;
			case DISCONNECTING:
				Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Stream state was Disconnecting");
				break;
			case CLOSED:
				Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Stream state was Closed");
				break;
			default:
				Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Stream state was Unestablished or unknown");
				break;
			}
			r.stream->ReleaseFromUse();
			cur = m_streams.erase(cur);
			continue;
		}

		//not expired, check against all patch signatures

		bool found_one = false;		//"we found a matching patch for this stream"
		bool all_ready = true;		//"all signatures were ready to check the stream"

		//foreach possbile patch...
		curp = m_patches.begin();
		endp = m_patches.end();
		for(; !found_one && curp != endp; ++curp) {
			Patch *p = *curp;

			//ask the stream to see if it matches the supplied signature
			EQStream::MatchState res = r.stream->CheckSignature(&p->signature);
			switch(res) {
			case EQStream::MatchNotReady:
				//the stream has not received enough packets to compare with this signature
//				Log.LogDebugType(Logs::General, Logs::Netcode, "[IDENT_TRACE] %s:%d: Tried patch %s, but stream is not ready for it.", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()), p->name.c_str());
				all_ready = false;
				break;
			case EQStream::MatchSuccessful: {
				//yay, a match.

				Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Identified stream %s:%d with signature %s", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()), p->name.c_str());

				// before we assign the eqstream to an interface, let the stream recognize it is in use and the session should not be reset any further
				r.stream->SetActive(true);

				//might want to do something less-specific here... some day..
				EQStreamInterface *s = new EQStreamProxy(r.stream, p->structs, p->opcodes);
				m_identified.push(s);

				found_one = true;
				break;
			}
			case EQStream::MatchFailed:
				//do nothing...
				Log.Out(Logs::General, Logs::Netcode, "[IDENT_TRACE] %s:%d: Tried patch %s, and it did not match.", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()), p->name.c_str());
				break;
			}
		}

		//if we checked all patches and did not find a match.
		if(all_ready && !found_one) {
			//the stream cannot be identified.
			Log.Out(Logs::General, Logs::Netcode, "[IDENTIFY] Unable to identify stream from %s:%d, no match found.", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()));
			r.stream->ReleaseFromUse();
		}

		//if we found a match, or were not able to identify it
		if(found_one || all_ready) {
			//cannot print ip/port here. r.stream is invalid.
			cur = m_streams.erase(cur);
		} else {
			++cur;
		}
	}	//end foreach stream
}

void EQStreamIdentifier::AddStream(std::shared_ptr<EQStream> &eqs) {
	m_streams.push_back(Record(eqs));
	eqs = nullptr;
}

EQStreamInterface *EQStreamIdentifier::PopIdentified() {
	if(m_identified.empty())
		return(nullptr);
	EQStreamInterface *res = m_identified.front();
	m_identified.pop();
	return(res);
}

EQStreamIdentifier::Record::Record(std::shared_ptr<EQStream> s)
:	stream(std::move(s)),
	expire(STREAM_IDENT_WAIT_MS)
{
}

