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
// This class will split up a string smartly at the div character (default is space)
// Seperator.arg[i] is a copy of the string chopped at the divs
// Seperator.argplus[i] is a pointer to the original string so it doesnt end at the div

// Written by Quagmire
#ifndef SEPERATOR2_H
#define SEPERATOR2_H
#define arglen 400
#define argnum 100		// Not including 0
#define arglenz 100
#define argnumz 10		// Not including 0
#define arghlenz 400
#define arghnumz 2048	// Not including 0
#define arghlenza 100
#define arghnumza 10	// Not including 0

class Seperator2
{
public:
	Seperator2(char* messagez, char divz = '|') {
		int iz;
		for (iz=0; iz <= argnumz; iz++) {
			memset(argz[iz], 0, sizeof(argz[iz]));
			argplusz[iz] = argz[iz];
		}

		int lenz = strlen(messagez);
		int az = 0, sz = 0, lz = 0;
		bool inargz = (!(messagez[0] == divz));
		argplusz[0] = messagez;
		for (iz=0; iz <= lenz; iz++) {
			if (inargz) {
				if (messagez[iz] == divz) {
					lz = iz-sz;
					if (lz >= (arglenz-1))
						lz = (arglenz-1);
					memcpy(argz[az], argplusz[az], lz);
					memset(&argz[az][lz], 0, 1);
					az++;
					inargz = false;
				}
			}
			else {
				sz = iz;
				argplusz[az] = &messagez[iz];
				if (!(messagez[iz] == divz)) {
					inargz = true;
				}
			}
			if (az > argnumz)
				break;
		}
		if (inargz)
			memcpy(argz[az], argplusz[az], (iz-sz) - 1);
	}
	~Seperator2() {}
	char argz[argnumz+1][arglenz];
	char* argplusz[argnumz+1];
	bool IsNumberz(int numz) {
		bool SeenDecz = false;
		int lenz = strlen(argz[numz]);
		if (lenz == 0) {
			return false;
		}
		int iz;
		for (iz = 0; iz < lenz; iz++) {
			if (argz[numz][iz] < '0' || argz[numz][iz] > '9') {
				if (argz[numz][iz] == '.' && !SeenDecz) {
					SeenDecz = true;
				}
				else if (iz == 0 && (argz[numz][iz] == '-' || argz[numz][iz] == '+') && !argz[numz][iz+1] == 0) {
					// this is ok, do nothin
				}
				else {
					return false;
				}
			}
		}
		return true;
	}
};

class Seperator3
{
public:
	Seperator3(char* messagez, char divz = 10) {
		int iz;
		for (iz=0; iz <= arghnumz; iz++) {
			memset(arghz[iz], 0, sizeof(arghz[iz]));
			arghplusz[iz] = arghz[iz];
		}

		int lenz = strlen(messagez);
		int az = 0, sz = 0, lz = 0;
		bool inarghz = (!(messagez[0] == divz));
		arghplusz[0] = messagez;
		for (iz=0; iz <= lenz; iz++) {
			if (inarghz) {
				if (messagez[iz] == divz) {
					lz = iz-sz;
					if (lz >= (arghlenz-1))
						lz = (arghlenz-1);
					memcpy(arghz[az], arghplusz[az], lz);
					memset(&arghz[az][lz], 0, 1);
					az++;
					inarghz = false;
				}
			}
			else {
				sz = iz;
				arghplusz[az] = &messagez[iz];
				if (!(messagez[iz] == divz)) {
					inarghz = true;
				}
			}
			if (az > arghnumz)
				break;
		}
		if (inarghz)
			memcpy(arghz[az], arghplusz[az], (iz-sz) - 1);
	}
	~Seperator3() {}
	char arghz[arghnumz+1][arghlenz];
	char* arghplusz[arghnumz+1];
	bool IsNumberz(int numz) {
		bool SeenDecz = false;
		int lenz = strlen(arghz[numz]);
		if (lenz == 0) {
			return false;
		}
		int iz;
		for (iz = 0; iz < lenz; iz++) {
			if (arghz[numz][iz] < '0' || arghz[numz][iz] > '9') {
				if (arghz[numz][iz] == '.' && !SeenDecz) {
					SeenDecz = true;
				}
				else if (iz == 0 && (arghz[numz][iz] == '-' || arghz[numz][iz] == '+') && !arghz[numz][iz+1] == 0) {
					// this is ok, do nothin
				}
				else {
					return false;
				}
			}
		}
		return true;
	}
};

class Seperator4
{
public:
	Seperator4(char* messageza, char divza = ':') {
		int iza;
		for (iza=0; iza <= arghnumza; iza++) {
			memset(arghza[iza], 0, sizeof(arghza[iza]));
			arghplusza[iza] = arghza[iza];
		}

		int lenza = strlen(messageza);
		int aza = 0, sza = 0, lza = 0;
		bool inarghza = (!(messageza[0] == divza));
		arghplusza[0] = messageza;
		for (iza=0; iza <= lenza; iza++) {
			if (inarghza) {
				if (messageza[iza] == divza) {
					lza = iza-sza;
					if (lza >= (arghlenza-1))
						lza = (arghlenza-1);
					memcpy(arghza[aza], arghplusza[aza], lza);
					memset(&arghza[aza][lza], 0, 1);
					aza++;
					inarghza = false;
				}
			}
			else {
				sza = iza;
				arghplusza[aza] = &messageza[iza];
				if (!(messageza[iza] == divza)) {
					inarghza = true;
				}
			}
			if (aza > arghnumza)
				break;
		}
		if (inarghza)
			memcpy(arghza[aza], arghplusza[aza], (iza-sza) - 1);
	}
	~Seperator4() {}
	char arghza[arghnumza+1][arghlenza];
	char* arghplusza[arghnumza+1];
	bool IsNumberza(int numza) {
		bool SeenDecza = false;
		int lenza = strlen(arghza[numza]);
		if (lenza == 0) {
			return false;
		}
		int iza;
		for (iza = 0; iza < lenza; iza++) {
			if (arghza[numza][iza] < '0' || arghza[numza][iza] > '9') {
				if (arghza[numza][iza] == '.' && !SeenDecza) {
					SeenDecza = true;
				}
				else if (iza == 0 && (arghza[numza][iza] == '-' || arghza[numza][iza] == '+') && !arghza[numza][iza+1] == 0) {
					// this is ok, do nothin
				}
				else {
					return false;
				}
			}
		}
		return true;
	}
};


#endif
