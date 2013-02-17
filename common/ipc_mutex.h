#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <string>

namespace EQEmu {
    
    //! Interprocess Named Binary Semaphore (Mutex)
    /*!
        Important to note: while this can be used to synchronize processes, it is not in itself re-entrant or thread-safe 
        and thus should be used from one thread and non-recursively.  It was intended to be a simple synchronization method
        for our MemoryMappedFile loading.
    */
    class IPCMutex {
        struct Implementation;
    public:
        //! Constructor
        /*!
            Creates a named binary semaphore, basically a semaphore that is init S <- 1
        \param name The name of this mutex.
        */
        IPCMutex(std::string name);
        
        //! Destructor
        ~IPCMutex();
        
        //! Lock the mutex
        /*!
            Same basic function as P(): for(;;) { if(S > 0) { S -= 1; break; } }
        */
        bool Lock();
        
        //! Unlocks the mutex
        /*!
            Same basic function as V(): S += 1;
        */
        bool Unlock();
    private:
        IPCMutex(const IPCMutex&);
        const IPCMutex& operator=(const IPCMutex&);
        
        bool locked_; //!< Whether this mutex is locked or not
        Implementation *imp_;
    };
}

#endif
