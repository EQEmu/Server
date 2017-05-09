
#include "global_define.h"
#include "eqemu_logsys.h"
#include "struct_strategy.h"

#include "eq_stream_intf.h"
#include "opcodemgr.h"
#include <map>
#include <memory>


//note: all encoders and decoders must be valid functions.
//so if you specify set_defaults=false
StructStrategy::StructStrategy() {
	int r;
	for(r = 0; r < _maxEmuOpcode; r++) {
		encoders[r] = PassEncoder;
		decoders[r] = PassDecoder;
	}
}

void StructStrategy::Encode(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) const {
	if((*p)->GetOpcodeBypass() != 0) {
		PassEncoder(p, dest, ack_req);
		return;
	}

	EmuOpcode op = (*p)->GetOpcode();
	Encoder proc = encoders[op];
	proc(p, dest, ack_req);
}

void StructStrategy::Decode(EQApplicationPacket *p) const {
	EmuOpcode op = p->GetOpcode();
	Decoder proc = decoders[op];
	proc(p);
}


void StructStrategy::ErrorEncoder(EQApplicationPacket **in_p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) {
	EQApplicationPacket *p = *in_p;
	*in_p = nullptr;

	Log(Logs::General, Logs::Netcode, "[STRUCTS] Error encoding opcode %s: no encoder provided. Dropping.", OpcodeManager::EmuToName(p->GetOpcode()));

	delete p;
}

void StructStrategy::ErrorDecoder(EQApplicationPacket *p) {
	Log(Logs::General, Logs::Netcode, "[STRUCTS] Error decoding opcode %s: no decoder provided. Invalidating.", OpcodeManager::EmuToName(p->GetOpcode()));
	p->SetOpcode(OP_Unknown);
}

void StructStrategy::PassEncoder(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) {
	dest->FastQueuePacket(p, ack_req);
}

void StructStrategy::PassDecoder(EQApplicationPacket *p) {
	//do nothing since we decode in place
}




//effectively a singleton, but I decided to do it this way for no apparent reason.
namespace StructStrategyFactory {

	static std::map<EmuOpcode, const StructStrategy *> strategies;

	void RegisterPatch(EmuOpcode first_opcode, const StructStrategy *structs) {
		strategies[first_opcode] = structs;
	}

	const StructStrategy *FindPatch(EmuOpcode first_opcode) {
		std::map<EmuOpcode, const StructStrategy *>::const_iterator res;
		res = strategies.find(first_opcode);
		if(res == strategies.end())
			return(nullptr);
		return(res->second);
	}

};

























