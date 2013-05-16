#ifndef EQSTREAMIDENT_H_
#define EQSTREAMIDENT_H_

#include "EQStream.h"
#include "timer.h"
#include <vector>
#include <string>
#include <queue>

#define STREAM_IDENT_WAIT_MS 10000

class OpcodeManager;
class StructStrategy;

class EQStreamIdentifier {
public:
	~EQStreamIdentifier();

	//registration interface.
	void RegisterPatch(const EQStream::Signature &sig, const char *name, OpcodeManager ** opcodes, const StructStrategy *structs);

	//main processing interface
	void Process();
	void AddStream(EQStream *& eqs);
	EQStreamInterface *PopIdentified();

protected:

	//registered patches..
	class Patch {
	public:
		std::string				name;
		EQStream::Signature		signature;
		OpcodeManager **		opcodes;
		const StructStrategy *structs;
	};
	std::vector<Patch *> m_patches;	//we own these objects.

	//pending streams..
	class Record {
	public:
		Record(EQStream *s);
		EQStream *stream;		//we own this
		Timer expire;
	};
	std::vector<Record *> m_streams;	//we own these objects, and the streams contained in them.
	std::queue<EQStreamInterface *> m_identified;	//we own these objects
};

#endif /*EQSTREAMIDENT_H_*/
