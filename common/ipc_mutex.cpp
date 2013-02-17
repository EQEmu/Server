#include "ipc_mutex.h"
#ifdef _WINDOWS
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#endif
#include "types.h"
#include "eqemu_exception.h"


namespace EQEmu {
    struct IPCMutex::Implementation {
#ifdef _WINDOWS
        HANDLE mut_;
#else
        sem_t *sem_;
#endif
    };

    IPCMutex::IPCMutex(std::string name) : locked_(false) {
        imp_ = new Implementation;
#ifdef _WINDOWS
        std::string final_name = "EQEmuMutex_";
        final_name += name;

        imp_->mut_ = CreateMutex(NULL,
            FALSE,
            final_name.c_str());

        if(!imp_->mut_) {
            EQ_EXCEPT("IPC Mutex", "Could not create mutex.");
        }
#else
        std::string final_name = "/EQEmuMutex_";
        final_name += name;
        
        imp_->sem_ = sem_open(final_name.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
        if(imp_->sem_ == SEM_FAILED) {
                EQ_EXCEPT("IPC Mutex", "Could not create mutex.");
        }
#endif
    }

    IPCMutex::~IPCMutex() {
#ifdef _WINDOWS
        if(imp_->mut_) {
            if(locked_) {
                ReleaseMutex(imp_->mut_);
            }
            CloseHandle(imp_->mut_);
        }
#else
        if(imp_->sem_) {
            if(locked_) {
                sem_post(imp_->sem_);
            }
            sem_close(imp_->sem_);
        }
#endif
        delete imp_;
    }

    bool IPCMutex::Lock() {
        if(locked_) {
            return false;
        }

#ifdef _WINDOWS
        DWORD wait_result = WaitForSingleObject(imp_->mut_, INFINITE);
        if(wait_result != WAIT_OBJECT_0) {
            return false;
        }
#else
        if(sem_wait(imp_->sem_) == -1) {
            return false;
        }
#endif
        locked_ = true;
        return true;
    }

    bool IPCMutex::Unlock() {
        if(!locked_) {
            return false;
        }
#ifdef _WINDOWS
        if(!ReleaseMutex(imp_->mut_)) {
            return false;
        }
#else
        if(sem_post(imp_->sem_) == -1) {
            return false;
        }
#endif
        locked_ = false;
        return true;
    }
}
