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
#ifndef _EQPACKET_H
#define _EQPACKET_H

#include "BasePacket.h"
#include "EQStreamType.h"
#include "op_codes.h"
#include "platform.h"

#ifdef STATIC_OPCODE
	typedef unsigned short EmuOpcode;
	static const EmuOpcode OP_Unknown = 0;
#else
#include "emu_opcodes.h"
#endif

using namespace std;

class EQStream;
class EQStreamPair;

class EQPacket : public BasePacket {
	friend class EQStream;
public:
	virtual ~EQPacket() {}
	
	uint32 Size() const { return size+2; }
	
	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

	void SetOpcode(EmuOpcode op) { emu_opcode = op; }
	const EmuOpcode GetOpcode() const { return(emu_opcode); }
//	const char *GetOpcodeName() const;
	
protected:
	//this is just a cache so we dont look it up several times on Get()
	//and it is mutable so we can store the cached copy even on a const object
	EmuOpcode emu_opcode;

	EQPacket(EmuOpcode opcode, const unsigned char *buf, const uint32 len);
//	EQPacket(const EQPacket &p) { }
	EQPacket() { emu_opcode=OP_Unknown; pBuffer=nullptr; size=0; }

};

class EQRawApplicationPacket;

class EQProtocolPacket : public BasePacket {
	friend class EQStream;
	friend class EQStreamPair;
public:
	EQProtocolPacket(uint16 op, const unsigned char *buf, uint32 len) : BasePacket(buf,len), opcode(op) { acked = false; } 
//	EQProtocolPacket(const unsigned char *buf, uint32 len);
	bool combine(const EQProtocolPacket *rhs);
	uint32 serialize (unsigned char *dest) const;
	EQProtocolPacket *Copy() { return new EQProtocolPacket(opcode,pBuffer,size); }
	EQRawApplicationPacket *MakeAppPacket() const;
	
	bool acked;
	
	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;
	
protected:
	
	static bool ValidateCRC(const unsigned char *buffer, int length, uint32 Key);
	static uint32 Decompress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize);
	static uint32 Compress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize);
	static void ChatDecode(unsigned char *buffer, int size, int DecodeKey);
	static void ChatEncode(unsigned char *buffer, int size, int EncodeKey);
	
	uint16 GetRawOpcode() const { return(opcode); }
	
	uint32 Size() const { return size+2; }
	
	//the actual raw EQ opcode
	uint16 opcode;
};

class EQApplicationPacket : public EQPacket {
//	friend class EQProtocolPacket;
	friend class EQStream;
public:
	EQApplicationPacket() : EQPacket(OP_Unknown,nullptr,0) 
        { app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op) : EQPacket(op,nullptr,0) 
        { app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op, const uint32 len) : EQPacket(op,nullptr,len) 
        { app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op, const unsigned char *buf, const uint32 len) : EQPacket(op,buf,len) 
        { app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	bool combine(const EQApplicationPacket *rhs);
	uint32 serialize (uint16 opcode, unsigned char *dest) const;
	uint32 Size() const { return size+app_opcode_size; }
	
	virtual EQApplicationPacket *Copy() const;
	
	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;
	
protected:

	uint8 app_opcode_size;
private:

	EQApplicationPacket(const EQApplicationPacket &p) : EQPacket(p.emu_opcode, p.pBuffer, p.size) { app_opcode_size = p.app_opcode_size; }

};

class EQRawApplicationPacket : public EQApplicationPacket {
	friend class EQStream;
public:
	EQRawApplicationPacket(uint16 opcode, const unsigned char *buf, const uint32 len);
	uint16 GetRawOpcode() const { return(opcode); }
	
	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

protected:
	
	//the actual raw EQ opcode
	uint16 opcode;
	
	EQRawApplicationPacket(const unsigned char *buf, const uint32 len);
};

extern void DumpPacket(const EQApplicationPacket* app, bool iShowInfo = false);


#endif
