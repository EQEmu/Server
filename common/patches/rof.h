#ifndef ROF_H_
#define ROF_H_

#include "../struct_strategy.h"

class EQStreamIdentifier;

namespace RoF {

	//these are the only public member of this namespace.
	extern void Register(EQStreamIdentifier &into);
	extern void Reload();



	//you should not directly access anything below..
	//I just dont feel like making a seperate header for it.

	class Strategy : public StructStrategy {
	public:
		Strategy();

	protected:

		virtual std::string Describe() const;
		virtual const EQEmu::versions::ClientVersion ClientVersion() const;

		//magic macro to declare our opcode processors
		#include "ss_declare.h"
		#include "rof_ops.h"
	};

};

#endif /*ROF_H_*/
