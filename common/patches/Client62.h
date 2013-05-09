#ifndef CLIENT62_H_
#define CLIENT62_H_

#include "../StructStrategy.h"
#include "../Item.h"

class EQStreamIdentifier;

namespace Client62 {

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
		virtual const EQClientVersion ClientVersion() const;

		//magic macro to declare our opcode processors
		#include "SSDeclare.h"
		#include "Client62_ops.h"

	};

};

#endif /*CLIENT62_H_*/
