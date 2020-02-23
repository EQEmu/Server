
#include "global_define.h"
#include "eq_stream_proxy.h"
#include "struct_strategy.h"
#include "eqemu_logsys.h"
#include "opcodemgr.h"


EQStreamProxy::EQStreamProxy(std::shared_ptr<EQStreamInterface> &stream, const StructStrategy *structs, OpcodeManager **opcodes)
:	m_stream(stream),
	m_structs(structs),
	m_opcodes(opcodes)
{
	stream = nullptr;	//take the stream.
	m_stream->SetOpcodeManager(m_opcodes);
}

EQStreamProxy::~EQStreamProxy() {
}

std::string EQStreamProxy::Describe() const {
	return(m_structs->Describe());
}

const EQEmu::versions::ClientVersion EQStreamProxy::ClientVersion() const
{
	return m_structs->ClientVersion();
}

EQStreamState EQStreamProxy::GetState()
{
	return m_stream->GetState();
}

void EQStreamProxy::SetOpcodeManager(OpcodeManager **opm)
{
	return m_stream->SetOpcodeManager(opm);
}

void EQStreamProxy::QueuePacket(const EQApplicationPacket *p, bool ack_req) {
	if(p == nullptr)
		return;

	if (p->GetOpcode() != OP_SpecialMesg) {
		Log(Logs::General, Logs::PacketServerClient, "[%s - 0x%04x] [Size: %u]", OpcodeManager::EmuToName(p->GetOpcode()), p->GetOpcode(), p->Size());
		Log(Logs::General, Logs::PacketServerClientWithDump, "[%s - 0x%04x] [Size: %u] %s", OpcodeManager::EmuToName(p->GetOpcode()), p->GetOpcode(), p->Size(), DumpPacketToString(p).c_str());
	}

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

std::string EQStreamProxy::GetRemoteAddr() const {
	return(m_stream->GetRemoteAddr());
}

uint32 EQStreamProxy::GetRemoteIP() const {
	return(m_stream->GetRemoteIP());
}

uint16 EQStreamProxy::GetRemotePort() const {
	return(m_stream->GetRemotePort());
}

void EQStreamProxy::ReleaseFromUse() {
	m_stream->ReleaseFromUse();
}

void EQStreamProxy::RemoveData() {
	m_stream->RemoveData();
}

EQStreamInterface::Stats EQStreamProxy::GetStats() const
{
	return m_stream->GetStats();
}

void EQStreamProxy::ResetStats()
{
	m_stream->ResetStats();
}

EQStreamManagerInterface *EQStreamProxy::GetManager() const
{
	return m_stream->GetManager();
}

bool EQStreamProxy::CheckState(EQStreamState state) {
	if(m_stream)
		return(m_stream->CheckState(state));

	return false;
}

