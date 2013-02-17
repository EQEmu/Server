// start mingw
#ifdef __MINGW32__
#define __try
#define __finally
#endif
// end mingw

#include "MMF.h"
#include <iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef _WINDOWS
	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
#else
    #include "MMFMutex.h"
    #include "../common/unix.h"    
#endif

MMF::MMF() {
	SharedMemory = 0;
	pCanWrite = false;
	#ifdef _WINDOWS
		hMapObject = NULL;
		lpvMem = 0;
	#else
		lpvMem = 0;
		pMMFMutex = 0;
		m_alloc = false;
	#endif
}

MMF::~MMF() {
	Close();
}

bool MMF::Open(const char* iName, uint32 iSize) {
	if (iSize < 1) {
		cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: iSize < 1" << endl;
		return false;
	}
	if (strlen(iName) < 2) {
		cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: strlen(iName) < 2" << endl;
		return false;
	}
	char MMFname[200];
	memset(MMFname, 0, sizeof(MMFname));
	snprintf(MMFname, sizeof(MMFname), "memfilemap_%s", iName);
	uint32 tmpSize = sizeof(MMF_Struct) + iSize;
	
#ifdef _WINDOWS
	char MMFMutexName[200];
	memset(MMFMutexName, 0, sizeof(MMFMutexName));
	snprintf(MMFMutexName, sizeof(MMFMutexName), "MutexToProtectOpenMMF_%s", iName);
	
	HANDLE hMutex;
	hMutex = CreateMutex( 
	NULL,                       // no security attributes
	FALSE,                      // initially not owned
	MMFMutexName);  // name of mutex

	if (hMutex == NULL) {
		cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: hMutex == Null" << endl;
		return false;
	}

	DWORD dwWaitResult;
    // Request ownership of mutex.
	dwWaitResult = WaitForSingleObject( 
		hMutex,   // handle to mutex
		2000L);   // two-second time-out interval

	if (dwWaitResult != WAIT_OBJECT_0) {
		// Mutex not aquired, crap out
		cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: dwWaitResult != WAIT_OBJECT_0" << endl;
		return false;
	}

	// Finally, ready to rock.
	bool fInit = false;
	__try {
		hMapObject = CreateFileMapping( 
			INVALID_HANDLE_VALUE,		// use paging file
			NULL,						// default security attributes
			PAGE_READWRITE,				// read/write access
			0,							// size: high 32-bits
			tmpSize,					// size: low 32-bits
			MMFname);					// name of map object
		if (hMapObject == NULL) {
			cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: hMapObject == Null" << endl;
			return false;
		}

		// The first process to attach initializes memory.

		fInit = (bool) (GetLastError() != ERROR_ALREADY_EXISTS); 

		// Get a pointer to the file-mapped shared memory.

		lpvMem = MapViewOfFile( 
			hMapObject,					// object to map view of
			FILE_MAP_WRITE,				// read/write access
			0,							// high offset:  map from
			0,							// low offset:   beginning
			0);							// default: map entire file
		if (lpvMem == NULL) {
			cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: lpvMem == Null" << endl;
			Close();
			return false;
		}
 
		SharedMemory = (MMF_Struct*) lpvMem;
		// Initialize memory if this is the first process.
		if (fInit) {
			memset(lpvMem, 0, sizeof(MMF_Struct));
			pCanWrite = true;
			SharedMemory->Loaded = false;
			SharedMemory->datasize = iSize;
		}
		else {
			pCanWrite = false;
			if (SharedMemory->datasize != iSize) {
				cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: SharedMemory->datasize != iSize" << endl;
				Close();
				return false;
			}
		}
	} // end of try block

	__finally {
		// Clean up the Mutex stuff
		if (!ReleaseMutex(hMutex)) {
			cout << "Error Loading MMF: " << __FILE__ << ":" << __LINE__ << " OpenMMF: !ReleaseMutex(hMutex)" << endl;
			Close();
			return false;
		}
	}
	CloseHandle(hMutex);

	return true;
#else //else, NOT WINDOWS
	int load_share;
	//int max_share = 7;
	key_t share_key;
	switch (MMFname[16]) {
		case 'I': load_share = 0;  break;
		case 'N': load_share = 1;  break;
		case 'D': load_share = 2;  break;
		case 'S': load_share = 3;  break;
		case 'F': load_share = 4;  break;
		case 'L': load_share = 5;  break;
		case 'M': load_share = 6;  break;
		case 'O': load_share = 7;  break;
		case 'Z': load_share = 8;  break;
		case 'K': load_share = 9;  break;
#ifdef CATCH_CRASH
		default:
		    cerr<<"Failed to load shared memory segment="<<MMFname<<" ("<<MMFname[16]<<")"<<endl;
		    // malloc some memory here or something fancy
		    return false; // and make this return true
		break;
#else
		default: cerr<<"FATAL="<<(char)MMFname[16]<<endl; return false; break;
#endif
	}
	switch (load_share) {
		// Item
		case 0: share_key = ftok(".", 'I'); break;
		// Npctype
		case 1: share_key = ftok(".", 'N'); break;
		// Door
		case 2: share_key = ftok(".", 'D'); break;
		// Spell
		case 3: share_key = ftok(".", 'S'); break;
		// Faction
		case 4: share_key = ftok(".", 'F'); break;
		// Loot
		case 5: share_key = ftok(".", 'L'); break;
		// ??
		case 6: share_key = ftok(".", 'M'); break;
		// Opcodes
		case 7: share_key = ftok(".", 'O'); break;
		// Item Serialization
		case 8: share_key = ftok(".", 'Z'); break;
		// Skills
		case 9: share_key = ftok(".", 'K'); break;
		// ERROR Fatal
		default: cerr<<"Opps!"<<endl; share_key = 0xFF; break;
	}
	pMMFMutex = new MMFMutex(share_key);
	if (!pMMFMutex){
		assert(false);
	}
	//if (!tmpSize) {
		int share_id = shmget(share_key, tmpSize, IPC_CREAT|IPC_EXCL|SHM_R|SHM_W);
		if ( share_id <= 0) {
			share_id = shmget(share_key, tmpSize, IPC_CREAT|IPC_NOWAIT);
			if (share_id <= 0) {
			    shmid_ds mem_size;
			    share_id = shmget(share_key, 1, IPC_CREAT|IPC_NOWAIT|SHM_R|SHM_W);
			    if(share_id == -1) {
				    cerr << "failed to get 0-length shared mem: " << strerror(errno) << endl;
			    }
			    if ((lpvMem = shmat(share_id, NULL,SHM_RDONLY)) == (void *)-1) {
				    cerr << "shmat failed! " << strerror(errno) << endl;
			    }
			    if( (shmctl(share_id, IPC_STAT, &mem_size)) == 0){
				  if (mem_size.shm_segsz != int(tmpSize)){	//comparison between signed and unsigned integer expressions
					cout<<"[Warning] requested shared memory of size:"<<tmpSize<<" but that Key is already in use with size:"<< mem_size.shm_segsz<<endl;
					shmid_ds mem_users;
					if( (shmctl(share_id, IPC_STAT, &mem_users)) == 0 && mem_users.shm_nattch == 1){
					    cout<<"[Warning] Attempting resize"<<endl;
					    shmctl(share_id, IPC_RMID, 0);
					    shmdt(lpvMem);
					    if ((share_id = shmget(share_key, tmpSize, IPC_CREAT|IPC_EXCL|SHM_R|SHM_W)) <= 0) {
						// Failed proceed on malloc
					    	cerr<<"[Error] Failed to resize" << strerror(errno) <<endl;
					    }
					    else{
					    	cerr<<"[Error] Resize successful." << endl;
							// Success
							lpvMem = shmat(share_id, NULL, SHM_R|SHM_W);
							memset(lpvMem, 0, sizeof(MMF_Struct));
							pCanWrite = true;
							SharedMemory = (MMF_Struct*) lpvMem;
							SharedMemory->Loaded = false;
							SharedMemory->datasize = iSize;
							pMMFMutex->Release(this);
							delete pMMFMutex;
							return true;
					    }
					}
					else{
					    cout<<"[Warning] Resize not possible"<<endl;
					}
				  }
			    }
			    // Can not attatch to shared memory we'll malloc it here
				if ((lpvMem == 0 || lpvMem == (void *)-1) && (lpvMem = malloc(tmpSize))) {
					cout<<"[Warning] Could not attach to shared memory proceeding on isolated memory (share_id <= 0)"<<endl;
					// Success!
					m_alloc = true;
					memset(lpvMem, 0, sizeof(MMF_Struct));
					pCanWrite = true;
					SharedMemory = (MMF_Struct*) lpvMem;
					SharedMemory->datasize = iSize;
					SharedMemory->Loaded = false;
					pMMFMutex->Release(this);
					delete pMMFMutex;
					return true;
				}  else if (!lpvMem){
					//LogFile->write(EQEMuLog::Error, "Could not connect to shared memory and allocation of isolated memory failed.");
					cout<<"Could not connect to shared memory and allocation of isolated memory failed."<<endl;
					pMMFMutex->Release(this);
					delete pMMFMutex;
					exit(1);
				}
				pCanWrite = false;
				SharedMemory = (MMF_Struct*) lpvMem;
				if (SharedMemory->datasize != iSize) {
					cerr<<"SharedMemory->datasize("<<SharedMemory->datasize<<") != iSize("<<iSize<<"), We can rebuild him faster better STRONGER!"<<endl;
					cerr<<"Or not.. restart all servers on this machine"<<endl;
					shmctl(share_id, IPC_RMID, 0);
					pMMFMutex->Release(this);
					exit(1);
				}
				pMMFMutex->Release(this);
				delete pMMFMutex;
				return true;
			}
			shmid_ds mem_users;
			if ((shmctl(share_id, IPC_STAT, &mem_users)) != 0) {
				if ((lpvMem = malloc(tmpSize))) {
					// Success!
					cout<<"[Warning] Could not attach to shared memory proceeding on isolated memory"<<endl;
					m_alloc = true;
					memset(lpvMem, 0, sizeof(MMF_Struct));
					pCanWrite = true;
					SharedMemory = (MMF_Struct*) lpvMem;
					SharedMemory->datasize = iSize;
					SharedMemory->Loaded = false;
					pMMFMutex->Release(this);
					delete pMMFMutex;
					return true;
				}  else {
					//LogFile->write(EQEMuLog::Error, "Could not connect to shared memory and allocation of isolated memory failed.");
					cout<<"Could not connect to shared memory and allocation of isolated memory failed."<<endl;
					pMMFMutex->Release(this);
					delete pMMFMutex;
					exit(1);
				}
			}
			lpvMem = shmat(share_id, NULL,SHM_RDONLY);
			pCanWrite = false;
			SharedMemory = (MMF_Struct*) lpvMem;
			//cerr << "lpvMem=" << (int)lpvMem << endl;
			if (lpvMem==(void *)-1 || SharedMemory->datasize != iSize) {
			    cerr<<"SharedMemory->datasize("<<SharedMemory->datasize<<") != iSize("<<iSize<<"), or "<<((void *)lpvMem)<<"==-1, We can rebuild him faster better STRONGER!"<<endl;
			    cerr<<"Or not.. restart all servers on this machine"<<endl;
			    shmctl(share_id, IPC_RMID, 0);
			    pMMFMutex->Release(this);
			    exit(1);
			}
			pMMFMutex->Release(this);
			delete pMMFMutex;
			return true;
		}
		lpvMem = shmat(share_id, NULL, SHM_R|SHM_W);
		memset(lpvMem, 0, sizeof(MMF_Struct));
		pCanWrite = true;
		SharedMemory = (MMF_Struct*) lpvMem;
		SharedMemory->Loaded = false;
		SharedMemory->datasize = iSize;
	//}
	pMMFMutex->Release(this);
	delete pMMFMutex;
	return true;

#endif //end NOT WINDOWS
}

void MMF::Close() {
	SharedMemory = 0;
	pCanWrite = false;
#ifdef _WINDOWS
		if (lpvMem) {
			// Unmap shared memory from the process's address space.
			UnmapViewOfFile(lpvMem); 
			lpvMem = 0;
		}
		if (hMapObject) {
			// Close the process's handle to the file-mapping object.
			CloseHandle(hMapObject);
			hMapObject = NULL;
		}
#else
		if (lpvMem) {
			if (m_alloc == true)
				free(lpvMem);
			else
				if (shmdt(lpvMem) == -1)
					//LogFile->write(EQEMuLog::Error, "Warning something odd happened freeing shared memory");
					cout<<"Warning something odd happened freeing shared memory"<<endl;
			lpvMem = 0;
		}
#endif
}

