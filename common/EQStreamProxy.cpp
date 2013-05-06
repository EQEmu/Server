
#include "debug.h"
#include "EQStreamProxy.h"
#include "EQStream.h"
#include "StructStrategy.h"


EQStreamProxy::EQStreamProxy(EQStream *&stream, const StructStrategy *structs, OpcodeManager **opcodes)
: m_stream(stream),
  m_structs(structs),
  m_opcodes(opcodes)
{
	stream = nullptr;	//take the stream.
	m_stream->SetOpcodeManager(m_opcodes);
}

EQStreamProxy::~EQStreamProxy() {
	//delete m_stream;	//released by the stream factory.
}

std::string EQStreamProxy::Describe() const {
	return(m_structs->Describe());
}

const EQClientVersion EQStreamProxy::ClientVersion() const
{
	return m_structs->ClientVersion();
}

void EQStreamProxy::QueuePacket(const EQApplicationPacket *p, bool ack_req) {
	if(p == nullptr)
		return;
	
	EQApplicationPacket *newp = p->Copy();
	FastQueuePacket(&newp, ack_req);
}

void EQStreamProxy::FastQueuePacket(EQApplicationPacket **p, bool ack_req) {
	if(p == nullptr || *p == nullptr)
		return;
	m_structs->Encode(p, m_stream, ack_req);
}

EQApplicationPacket *EQStreamProxy::PopPacket() {
	EQApplicationPacket *pack = m_stream->PopPacket();
	if(pack == nullptr)
		return(nullptr);
	
	//pass this packet through the struct strategy.
	m_structs->Decode(pack);
	return(pack);
}

void EQStreamProxy::Close() {
	m_stream->Close();
}

uint32 EQStreamProxy::GetRemoteIP() const {
	return(m_stream->GetRemoteIP());
}

uint16 EQStreamProxy::GetRemotePort() const {
	return(m_stream->GetRemotePort());
}

const uint32 EQStreamProxy::GetBytesSent() const
{
	return(m_stream->GetBytesSent());
}

const uint32 EQStreamProxy::GetBytesRecieved() const
{
	return(m_stream->GetBytesRecieved());
}

const uint32 EQStreamProxy::GetBytesSentPerSecond() const
{
	return(m_stream->GetBytesSentPerSecond());
}

const uint32 EQStreamProxy::GetBytesRecvPerSecond() const
{
	return(m_stream->GetBytesRecvPerSecond());
}

void EQStreamProxy::ReleaseFromUse() {
	m_stream->ReleaseFromUse();

	//this is so ugly, but I cant think of a better way to deal with
	//it right now...
	if(!m_stream->IsInUse()) {
		delete this;
	}
}

void EQStreamProxy::RemoveData() {
	m_stream->RemoveData();
}

bool EQStreamProxy::CheckState(EQStreamState state) {
	if(m_stream)
		return(m_stream->CheckState(state));

	return false;
}

