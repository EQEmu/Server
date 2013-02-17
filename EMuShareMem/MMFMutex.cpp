#include "MMFMutex.h"

#ifndef WIN32

MMFMutex::MMFMutex( int key )
 {
    m_key = key;

    if( m_key == 0 )
    {
        // initialize POSIX semaphore
        sem_init( &m_semaphore, 0, 1);
    }
    else
    {
        // currently, this thread does not own the semaphore
        m_owner = 0;
        m_recursive_count = 0;

        // try to get an existing semaphore. the access permissions are "full access for everyone"
        m_id = semget(m_key, 1, 0x1b6); 
        if( m_id == -1 )
        {
            // it doesn't exist yet, try to create a new one
            m_id = semget(m_key, 1, IPC_CREAT | 0x1b6);
            if( m_id != -1 )
            {
                // initialize it to 1
                semun data;
                data.val = 1;
                semctl(m_id, 0, SETVAL, data);
            }
        }
    }
}

MMFMutex::~MMFMutex()
{
    if( m_key == 0 )
    {
        sem_destroy(&m_semaphore);
    }
    else
    {
        semctl(m_id, 0, IPC_RMID, 0);
    }
}

bool MMFMutex::Lock( uint32 dwTimeout )
 {
    if( m_owner == pthread_self() )
    {
        m_recursive_count++;
        return true;
    }
    bool bUseTimeout = (dwTimeout != 0);
    while(true) {
        bool bGotSemaphore = false;
        if( m_key == 0 )
        {
            bGotSemaphore = (sem_trywait(&m_semaphore) == 0);
        }
        else
        {
            struct sembuf operations[1];
            operations[0].sem_num = 0;
            operations[0].sem_op = -1;
            operations[0].sem_flg = SEM_UNDO|IPC_NOWAIT;
            bGotSemaphore = (semop(m_id, operations, 1) >= 0);
        }
        if( bGotSemaphore )
        {
            m_owner = pthread_self();
            m_recursive_count = 1;
            return true;
        }
        sleep(1);
        if( bUseTimeout )
        {
            if( dwTimeout > 1000 )
                dwTimeout -= 1000;
            else
                dwTimeout = 0;

            if( dwTimeout == 0 )
                return false;
        }
    }
}
void MMFMutex::Release(const MMF* pMMF)
{
    if( m_owner != pthread_self() && pMMF->m_alloc != true  )
    {
        //We're supposed to explode here with an assert
        //assert(false);
    }
    else if ( pMMF->m_alloc == true ){
	// Just do it nothing is useing but us
	return;
    }
    else if( m_recursive_count > 1 )
    {
        m_recursive_count--;
    }
    else
    {
        if( m_key == 0 )
        {
            sem_post(&m_semaphore);
        }

        else
        {
            struct sembuf operations[1];
            operations[0].sem_num = 0;
            operations[0].sem_op = 1;
            operations[0].sem_flg = SEM_UNDO;
            semop(m_id, operations, 1);
        }
        m_recursive_count = 0;
        m_owner = 0;
    }
}
#endif //!WIN32


