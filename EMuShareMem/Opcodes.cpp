#include "../common/debug.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include "../common/unix.h"
#endif

#include <memory.h>
#include <iostream>
using namespace std;
#include "Opcodes.h"
#include "../common/timer.h"
#include "MMF.h"

MMF OpcodesMMF;
const MMFOpcodes_Struct* MMFOpcodesData = 0;
MMFOpcodes_Struct* MMFOpcodesData_Writable = 0;
const uint16 *MMFOpcodesData_emu_to_eq = NULL;
uint16 *MMFOpcodesData_emu_to_eq_write = NULL;

//we choose to store all opcodes as 16 bits, so if they are a different
//size in emu, they are gunna get casted to 16 bits... prolly will never
//be a problem, but I figured it was noteworthy


DLLFUNC uint16 GetEQOpcode(uint16 emu_op) {
	if (MMFOpcodesData == 0 || (!OpcodesMMF.IsLoaded()) || emu_op >= MMFOpcodesData->EmuOpcodeCount )
		return 0;
	return MMFOpcodesData_emu_to_eq[emu_op];
}

DLLFUNC uint16 GetEmuOpcode(uint16 eq_op) {
	if (MMFOpcodesData == 0 || (!OpcodesMMF.IsLoaded()) || eq_op >= MMFOpcodesData->EQOpcodeCount )
		return 0;
	return MMFOpcodesData->eq_to_emu[eq_op];
}

DLLFUNC bool SetOpcodePair(uint16 emu_op, uint16 eq_op) {
	if (!MMFOpcodesData_Writable || !MMFOpcodesData_emu_to_eq_write)
		return false;
	if (emu_op >= MMFOpcodesData_Writable->EmuOpcodeCount || eq_op >= MMFOpcodesData_Writable->EQOpcodeCount)
		return false;
	
	MMFOpcodesData_emu_to_eq_write[emu_op] = eq_op;
	MMFOpcodesData_Writable->eq_to_emu[eq_op] = emu_op;
	
	return true;
}

DLLFUNC void ClearEQOpcodes() {
	if (!MMFOpcodesData_Writable)
		return;
	
	memset(MMFOpcodesData_Writable->eq_to_emu, 0, sizeof(uint16)*MMFOpcodesData->EQOpcodeCount);
	
}

DLLFUNC bool DLLLoadOpcodes(CALLBACK_DBLoadOpcodes cb, uint32 opsize, uint32 eq_count, uint32 emu_count, const char *filename) {
	if(opsize != sizeof(uint16)) {
		cout << "Error: EMuShareMem: DLLLoadOpcodes: opsize != sizeof(uint16)" << endl;
		cout << "Opcode size has changed, EMuShareMem.dll needs to be recompiled." << endl;
		return false;
	}
	uint32 tmpMemSize = sizeof(MMFOpcodes_Struct) + opsize * (eq_count+emu_count);
	if (OpcodesMMF.Open("EQEMuOpcodes", tmpMemSize)) {
		if (OpcodesMMF.CanWrite()) {
			MMFOpcodesData_Writable = (MMFOpcodes_Struct*) OpcodesMMF.GetWriteableHandle();
			if (!MMFOpcodesData_Writable) {
				cout << "Error: EMuShareMem: DLLLoadOpcodes: !MMFOpcodesData_Writable" << endl;
				return false;
			}
			
			//emu_to_eq is right after eq_to_emu
			MMFOpcodesData_emu_to_eq = MMFOpcodesData_Writable->eq_to_emu + eq_count;
			MMFOpcodesData_emu_to_eq_write = MMFOpcodesData_Writable->eq_to_emu + eq_count;
			
			//we need to memset the eq opcodes
			memset(MMFOpcodesData_Writable->eq_to_emu, 0, sizeof(uint16)*eq_count);
			
			MMFOpcodesData_Writable->EQOpcodeCount = eq_count;
			MMFOpcodesData_Writable->EmuOpcodeCount = emu_count;
			// use a callback so the DB functions are done in the main exe
			// this way the DLL doesnt have to open a connection to mysql
			if (!cb(filename)) {
				cout << "Error: EMuShareMem: DLLLoadOpcodes: !cbDBLoadOpcodes" << endl;
				return false;
			}
			
			//we dont disable the write handle here, so we can reload them
			//MMFOpcodesData_Writable = 0;
			
			OpcodesMMF.SetLoaded();
			MMFOpcodesData = (const MMFOpcodes_Struct*) OpcodesMMF.GetHandle();
			if (!MMFOpcodesData) {
				cout << "Error: EMuShareMem: DLLLoadOpcodes: !MMFOpcodesData (CanWrite=true)" << endl;
				return false;
			}
			return true;
		} else {
			if (!OpcodesMMF.IsLoaded()) {
				Timer::SetCurrentTime();
				uint32 starttime = Timer::GetCurrentTime();
				while ((!OpcodesMMF.IsLoaded()) && ((Timer::GetCurrentTime() - starttime) < 300000)) {
					Sleep(10);
					Timer::SetCurrentTime();
				}
				if (!OpcodesMMF.IsLoaded()) {
					cout << "Error: EMuShareMem: DLLLoadOpcodes: !OpcodesMMF.IsLoaded() (timeout)" << endl;
					return false;
				}
			}
			MMFOpcodesData = (const MMFOpcodes_Struct*) OpcodesMMF.GetHandle();
			if (!MMFOpcodesData) {
				cout << "Error: EMuShareMem: DLLLoadOpcodes: !MMFOpcodesData (CanWrite=false)" << endl;
				return false;
			}
			
			//emu_to_eq is right after eq_to_emu
			MMFOpcodesData_emu_to_eq = MMFOpcodesData->eq_to_emu + MMFOpcodesData->EQOpcodeCount;
			
			//cheat a little so we can retain writeable handles for reloading
			MMFOpcodesData_Writable = const_cast<MMFOpcodes_Struct*>(MMFOpcodesData);
			MMFOpcodesData_emu_to_eq_write = MMFOpcodesData_Writable->eq_to_emu + eq_count;
			
			
			return true;
		}
	}
	else {
		cout << "Error Loading Opcodes: Opcodes.cpp: pDLLLoadOpcodes: ret == 0, size = " << tmpMemSize << endl;
		return false;
	}
	return false;
}





