/*
  EMuShareMem.dll
  by Quagmire
  Released under GPL

  This DLL's purpose it to hold a single shared copy of items, npctypes, spells, and other
  stuff that's normally cached in memory, thus allowing all processes on the server to share
  one copy of the data, greatly reducing the amount of RAM used.
*/
#ifdef _WINDOWS

#include <windows.h>
void CloseMemShare();

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH: {
            break;
		}
        case DLL_THREAD_DETACH: {
            break;
		}
        case DLL_PROCESS_DETACH: {
            break;
		}
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#endif //WIN32
