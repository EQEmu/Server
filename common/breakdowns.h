/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#ifndef BREAKDOWNS_H_
#define BREAKDOWNS_H_

#include "types.h"


#pragma pack(1)
struct uint16_breakdown {
	union {
		uint16 all;
		struct {
			uint8 b1;
			uint8 b2;
		} bytes;
	};
	inline uint16&	operator=(const uint16& val) { return (all=val); }
	inline uint16*	operator&() { return &all; }
	inline operator	uint16&() { return all; }
	inline uint8&	b1()	{ return bytes.b1; }
	inline uint8&	b2()	{ return bytes.b2; }
};

struct uint32_breakdown {
	union {
		uint32 all;
		struct {
			uint16 w1;
			uint16 w2;
		} words;
		struct {
			uint8 b1;
			union {
				struct {
					uint8 b2;
					uint8 b3;
				} middle;
				uint16 w2_3; // word bytes 2 to 3
			};
			uint8 b4;
		} bytes;
	};
	inline uint32&	operator=(const uint32& val) { return (all=val); }
	inline uint32*	operator&() { return &all; }
	inline operator	uint32&() { return all; }

	inline uint16&	w1()	{ return words.w1; }
	inline uint16&	w2()	{ return words.w2; }
	inline uint16&	w2_3()	{ return bytes.w2_3; }
	inline uint8&	b1()	{ return bytes.b1; }
	inline uint8&	b2()	{ return bytes.middle.b2; }
	inline uint8&	b3()	{ return bytes.middle.b3; }
	inline uint8&	b4()	{ return bytes.b4; }
};
/*
struct uint64_breakdown {
	union {
		uint64	all;
		struct {
			uint16	w1;	// 1 2
			uint16	w2;	// 3 4
			uint16	w3; // 5 6
			uint16	w4; // 7 8
		};
		struct {
			uint32	dw1; // 1 4
			uint32	dw2; // 5 6
		};
		struct {
			uint8	b1;
			union {
				struct {
					uint16	w2_3;
					uint16	w4_5;
					uint16	w6_7;
				};
				uint32	dw2_5;
				struct {
					uint8	b2;
					union {
						uint32	dw3_6;
						struct {
							uint8	b3;
							union {
								uint32	dw4_7;
								struct {
									uint8	b4;
									uint8	b5;
									uint8	b6;
									uint8	b7;
								};
							};
						};
					};
				};
			};
		};
	};
	inline uint64* operator&() { return &all; }
	inline operator uint64&() { return all; }
};
*/
#pragma pack()














#endif /*BREAKDOWNS_H_*/
