/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

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
#ifndef EQEMU_LOGINSTRUCTURES_H
#define EQEMU_LOGINSTRUCTURES_H

#pragma pack(1)

struct LoginChatMessage_Struct {
	short Unknown0;
	uint32 Unknown1;
	uint32 Unknown2;
	uint32 Unknown3;
	uint8 Unknown4;
	char ChatMessage[1];
};

struct LoginLoginRequest_Struct {
	short unknown1;
	short unknown2;
	short unknown3;
	short unknown4;
	short unknown5;
	char unknown6[16];
};

struct LoginAccepted_Struct {
	short unknown1;
	short unknown2;
	short unknown3;
	short unknown4;
	short unknown5;
	char encrypt[80];
};

struct LoginFailedAttempts_Struct
{
	char message; //0x01
	char unknown2[7]; //0x00
	uint32 lsid;
	char key[11]; //10 char + null term;
	uint32 failed_attempts;
	char unknown3[4];	//0x00, 0x00, 0x00, 0x03
	char unknown4[4];	//0x00, 0x00, 0x00, 0x02
	char unknown5[4];	//0xe7, 0x03, 0x00, 0x00
	char unknown6[4];	//0xff, 0xff, 0xff, 0xff
	char unknown7[4];	//0xa0, 0x05, 0x00, 0x00
	char unknown8[4];	//0x00, 0x00, 0x00, 0x02
	char unknown9[4];	//0xff, 0x03, 0x00, 0x00
	char unknown10[4];	//0x00, 0x00, 0x00, 0x00
	char unknown11[4];	//0x63, 0x00, 0x00, 0x00
	char unknown12[4];	//0x01, 0x00, 0x00, 0x00
	char unknown13[4];	//0x00, 0x00, 0x00, 0x00
	char unknown14[4];	//0x00, 0x00, 0x00, 0x00
};

struct LoginLoginFailed_Struct {
	short unknown1;
	short unknown2;
	short unknown3;
	short unknown4;
	short unknown5;
	char unknown6[74];
};

struct ServerListHeader_Struct {

	uint32 Unknown1;
	uint32 Unknown2;
	uint32 Unknown3;
	uint32 Unknown4;
	uint32 NumberOfServers;
};

struct PlayEverquestRequest_Struct
{
	uint16 Sequence;
	uint32 Unknown1;
	uint32 Unknown2;
	uint32 ServerNumber;
};

struct PlayEverquestResponse_Struct {
	uint8 Sequence;
	uint8 Unknown1[9];
	uint8 Allowed;
	uint16 Message;
	uint8 Unknown2[3];
	uint32 ServerNumber;
};

static const unsigned char FailedLoginResponseData[] = {
	0xf6, 0x85, 0x9c, 0x23, 0x57, 0x7e, 0x3e, 0x55, 0xb3, 0x4c, 0xf8, 0xc8, 0xcb, 0x77, 0xd5, 0x16,
	0x09, 0x7a, 0x63, 0xdc, 0x57, 0x7e, 0x3e, 0x55, 0xb3, 0x4c, 0xf8, 0xc8, 0xcb, 0x77, 0xd5, 0x16,
	0x09, 0x7a, 0x63, 0xdc, 0x57, 0x7e, 0x3e, 0x55, 0xb3 };


#pragma pack()

#endif

