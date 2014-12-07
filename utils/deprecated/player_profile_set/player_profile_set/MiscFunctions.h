/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

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
#ifndef MISCFUNCTIONS_H
#define MISCFUNCTIONS_H

#include "types.h"
#include <stdio.h>
#include <ctype.h>


#ifndef ERRBUF_SIZE
#define ERRBUF_SIZE		1024
#endif

// These are helper macros for dealing with packets of variable length, typically those that contain
// variable length strings where it is not convenient to use a fixed length struct.
//
#define VARSTRUCT_DECODE_TYPE(Type, Buffer) *(Type *)Buffer; Buffer += sizeof(Type);
#define VARSTRUCT_DECODE_STRING(String, Buffer) strcpy(String, Buffer); Buffer += strlen(String)+1;
#define VARSTRUCT_ENCODE_STRING(Buffer, String) sprintf(Buffer, String); Buffer += strlen(String) + 1;
#define VARSTRUCT_ENCODE_INTSTRING(Buffer, Number) sprintf(Buffer, "%i", Number); Buffer += strlen(Buffer) + 1;
#define VARSTRUCT_ENCODE_TYPE(Type, Buffer, Value) *(Type *)Buffer = Value; Buffer += sizeof(Type);

//////////////////////////////////////////////////////////////////////
//
//  MakeUpperString
//   i     : source - allocated null-terminated string
//   return: pointer to static buffer with the target string
const char *MakeUpperString(const char *source);
const char *MakeLowerString(const char *source);
//////////////////////////////////////////////////////////////////////
//
//  MakeUpperString
//   i : source - allocated null-terminated string
//   io: target - allocated buffer, at least of size strlen(source)+1
void MakeUpperString(const char *source, char *target);
void MakeLowerString(const char *source, char *target);


int		MakeAnyLenString(char** ret, const char* format, ...);
int32	AppendAnyLenString(char** ret, int32* bufsize, int32* strlen, const char* format, ...);
int32	hextoi(char* num);
int64	hextoi64(char* num);
bool	atobool(char* iBool);
void	CoutTimestamp(bool ms = true);
char*	strn0cpy(char* dest, const char* source, int32 size);
		// return value =true if entire string(source) fit, false if it was truncated
bool	strn0cpyt(char* dest, const char* source, int32 size);
int	MakeRandomInt(int low, int high);
double	MakeRandomFloat(double low, double high);
char *CleanMobName(const char *in, char *out);
const char *ConvertArray(int input, char *returnchar);
const char *ConvertArrayF(float input, char *returnchar);
float EQ13toFloat(int d);
float NewEQ13toFloat(int d);
float EQ19toFloat(int d);
float EQHtoFloat(int d);
int FloatToEQ13(float d);
int NewFloatToEQ13(float d);
int FloatToEQ19(float d);
int FloatToEQH(float d);

#endif

