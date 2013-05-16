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
#include "../common/debug.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>

using namespace std;

#include "packet_dump.h"
#include "EQPacket.h"
#include "../common/servertalk.h"

void DumpPacketAscii(const uchar* buf, uint32 size, uint32 cols, uint32 skip) {
	// Output as ASCII
	for(uint32 i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0)
		{
			cout << endl << setw(3) << setfill(' ') << i-skip << ":";
		}
		else if ((i-skip)%(cols/2)==0)
		{
			cout << " - ";
		}
		if (buf[i] > 32 && buf[i] < 127)
		{
			cout << buf[i];
		}
		else
		{
			cout << '.';
		}
	}
	cout << endl << endl;
}

void DumpPacketHex(const uchar* buf, uint32 size, uint32 cols, uint32 skip) {
	if (size == 0 || size > 39565)
		return;
	// Output as HEX
	char output[4];
	int j = 0;
	char* ascii = new char[cols+1];
	memset(ascii, 0, cols+1);
	uint32 i;
	for(i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0) {
			if (i != skip)
				cout << " | " << ascii << endl;
			cout << setw(4) << setfill(' ') << i-skip << ": ";
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			cout << "- ";
		}
		sprintf(output, "%02X ", (unsigned char)buf[i]);
		cout << output;

		if (buf[i] >= 32 && buf[i] < 127) {
			ascii[j++] = buf[i];
		}
		else {
			ascii[j++] = '.';
		}
//		cout << setfill(0) << setw(2) << hex << (int)buf[i] << " ";
	}
	uint32 k = ((i-skip)-1)%cols;
	if (k < 8)
		cout << "  ";
	for (uint32 h = k+1; h < cols; h++) {
		cout << "   ";
	}
	cout << " | " << ascii << endl;
	safe_delete_array(ascii);
}

void DumpPacket(const uchar* buf, uint32 size)
{
	DumpPacketHex(buf, size);
//	DumpPacketAscii(buf,size);
}

void DumpPacket(const ServerPacket* pack, bool iShowInfo) {
	if (iShowInfo) {
		cout << "Dumping ServerPacket: 0x" << hex << setfill('0') << setw(4) << pack->opcode << dec;
		cout << " size:" << pack->size << endl;
	}
	DumpPacketHex(pack->pBuffer, pack->size);
}

void DumpPacketBin(const ServerPacket* pack) {
	DumpPacketBin(pack->pBuffer, pack->size);
}

void DumpPacketBin(uint32 data) {
	DumpPacketBin((uchar*)&data, sizeof(uint32));
}

void DumpPacketBin(uint16 data) {
	DumpPacketBin((uchar*)&data, sizeof(uint16));
}

void DumpPacketBin(uint8 data) {
	DumpPacketBin((uchar*)&data, sizeof(uint8));
}


void DumpPacketBin(const void* iData, uint32 len) {
	if (!len)
		return;
	const uint8* data = (const uint8*) iData;
	uint32 k=0;
	for (k=0; k<len; k++) {
		if (k % 4 == 0) {
			if (k != 0) {
				cout << " | " << hex << setw(2) << setfill('0') << (int) data[k-4] << dec;
				cout << " " << hex << setw(2) << setfill('0') << (int) data[k-3] << dec;
				cout << " " << hex << setw(2) << setfill('0') << (int) data[k-2] << dec;
				cout << " " << hex << setw(2) << setfill('0') << (int) data[k-1] << dec;
				cout << endl;
			}
			cout << setw(4) << setfill('0') << k << ":";
		}
		else if (k % 2 == 0)
			cout << " ";
		cout << " ";
		if (data[k] & 1)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 2)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 4)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 8)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 16)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 32)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 64)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 128)
			cout << "1";
		else
			cout << "0";
	}
	uint8 tmp = (k % 4);
	if (!tmp)
		tmp = 4;
	if (tmp <= 3)
		cout << "         ";
	if (tmp <= 2)
		cout << "          ";
	if (tmp <= 1)
		cout << "         ";
	cout << " | " << hex << setw(2) << setfill('0') << (int) data[k-4] << dec;
	if (tmp > 1)
		cout << " " << hex << setw(2) << setfill('0') << (int) data[k-3] << dec;
	if (tmp > 2)
		cout << " " << hex << setw(2) << setfill('0') << (int) data[k-2] << dec;
	if (tmp > 3)
		cout << " " << hex << setw(2) << setfill('0') << (int) data[k-1] << dec;
	cout << endl;
}

