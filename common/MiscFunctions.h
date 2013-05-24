/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef MISCFUNCTIONS_H
#define MISCFUNCTIONS_H

#include "types.h"
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <time.h>


#ifndef ERRBUF_SIZE
#define ERRBUF_SIZE		1024
#endif

// These are helper macros for dealing with packets of variable length, typically those that contain
// variable length strings where it is not convenient to use a fixed length struct.
//
#define VARSTRUCT_DECODE_TYPE(Type, Buffer) *(Type *)Buffer; Buffer += sizeof(Type);
#define VARSTRUCT_DECODE_STRING(String, Buffer) strcpy(String, Buffer); Buffer += strlen(String)+1;
#define VARSTRUCT_ENCODE_STRING(Buffer, String) { sprintf(Buffer, "%s", String); Buffer += strlen(String) + 1; }
#define VARSTRUCT_ENCODE_INTSTRING(Buffer, Number) { sprintf(Buffer, "%i", Number); Buffer += strlen(Buffer) + 1; }
#define VARSTRUCT_ENCODE_TYPE(Type, Buffer, Value) { *(Type *)Buffer = Value; Buffer += sizeof(Type); }
#define VARSTRUCT_SKIP_TYPE(Type, Buffer) Buffer += sizeof(Type);

#define VERIFY_PACKET_LENGTH(OPCode, Packet, StructName) \
	if(Packet->size != sizeof(StructName)) \
	{ \
		LogFile->write(EQEMuLog::Debug, "Size mismatch in " #OPCode " expected %i got %i", sizeof(StructName), Packet->size); \
		DumpPacket(Packet); \
		return; \
	}

// Definitions for WELLRNG
//
#define W 32
#define R 624
#define DISCARD 31
#define MASKU (0xffffffffU>>(W-DISCARD))
#define MASKL (~MASKU)
#define M1 70
#define M2 179
#define M3 449

#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define MAT1(v) v
#define MAT3POS(t,v) (v>>t)

#define V0				STATE[state_i]
#define VM1Over			STATE[state_i+M1-R]
#define VM1				STATE[state_i+M1]
#define VM2Over			STATE[state_i+M2-R]
#define VM2				STATE[state_i+M2]
#define VM3Over			STATE[state_i+M3-R]
#define VM3				STATE[state_i+M3]
#define VRm1			STATE[state_i-1]
#define VRm1Under		STATE[state_i+R-1]
#define VRm2			STATE[state_i-2]
#define VRm2Under		STATE[state_i+R-2]

#define newV0			STATE[state_i-1]
#define newV0Under		STATE[state_i-1+R]
#define newV1			STATE[state_i]
#define newVRm1			STATE[state_i-2]
#define newVRm1Under	STATE[state_i-2+R]

#define newVM2Over		STATE[state_i+M2-R+1]
#define newVM2			STATE[state_i+M2+1]

#define BITMASK 0x41180000



int32	filesize(FILE* fp);
uint32	ResolveIP(const char* hostname, char* errbuf = 0);
bool	ParseAddress(const char* iAddress, uint32* oIP, uint16* oPort, char* errbuf = 0);
void	CoutTimestamp(bool ms = true);
int	MakeRandomInt(int low, int high);
double	MakeRandomFloat(double low, double high);
float EQ13toFloat(int d);
float NewEQ13toFloat(int d);
float EQ19toFloat(int d);
float EQHtoFloat(int d);
int FloatToEQ13(float d);
int NewFloatToEQ13(float d);
int FloatToEQ19(float d);
int FloatToEQH(float d);



#define _ITOA_BUFLEN	25
const char *itoa(int num);	//not thread safe
#ifndef _WINDOWS
const char *itoa(int num, char* a,int b);
#endif

class InitWinsock {
public:
	InitWinsock();
	~InitWinsock();
};

template<class T> class AutoDelete {
public:
	AutoDelete(T** iVar, T* iSetTo = 0) {
		init(iVar, iSetTo);
	}
	AutoDelete() { pVar = nullptr; }
	void init(T** iVar, T* iSetTo = 0)
	{
		pVar = iVar;
		if (iSetTo)
			*pVar = iSetTo;
	}
	~AutoDelete() {
		if(pVar != nullptr)
			safe_delete(*pVar);
	}
	void ReallyClearIt() {
		pVar = nullptr;
	}
private:
	T** pVar;
};

#endif

