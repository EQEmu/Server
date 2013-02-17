#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <iostream>

int main(int argc, char** argv) {
    using std::cout;
    using std::endl;
    
    key_t shm_key;
    shmid_ds* memuser_ds = new shmid_ds;
    
    bool print = false;
    if (argc >= 2) {
	print = true;
    }
    
    for ( int i = 0; i < 10; i++) {
	memuser_ds->shm_nattch = 0;
		const char *n = "ERROR";
		switch (i) {
			case 0: shm_key = ftok(".", 'I'); n="items"; break;
			case 1: shm_key = ftok(".", 'N'); n="unused"/*npc_types*/; break;
			case 2: shm_key = ftok(".", 'D'); n="unused"/*doors*/; break;
			case 3: shm_key = ftok(".", 'S'); n="spells"; break;
			case 4: shm_key = ftok(".", 'F'); n="factions"; break;
			case 5: shm_key = ftok(".", 'L'); n="loot"; break;
			case 6: shm_key = ftok(".", 'M'); n="unused"/*??*/; break;
			case 7: shm_key = ftok(".", 'O'); n="opcodes"; break;
			case 8: shm_key = ftok(".", 'Z'); n="unused"/*item serialization*/; break;
			case 9: shm_key = ftok(".", 'K'); n="skillcaps"; break;
			default: break;
		}
		int semid = semget(shm_key, 1, 0);
		if(print){
			cout<<"ID="<<semid;
			cout<<" EQEmuShare#:"<<i;
		}
		semctl(semget(shm_key, 1, 0), i, GETVAL, 0);
		int shmid = shmget(shm_key, 0, 0); 
		shmctl(shmid, IPC_STAT, memuser_ds);
		if(memuser_ds->shm_nattch > 0) {
			cout << "Something is still attached to " << shm_key << " (" << n << ")" << endl;
		} else {
			cout << "Successfully removed " << shm_key << " (" << n << ")" << endl;
		}
		if(print){
			cout<<" useing_shm=";
			cout<<memuser_ds->shm_nattch;
		}
		if (memuser_ds->shm_nattch == 0) {
			if(print){
			cout<<" Deleteing stale shares"<<endl;
			}
			shmctl(shmid, IPC_RMID, 0);
			semctl(semget(shm_key, 1, 0), IPC_RMID, 0);
		} else {
			if(print){
			cout<<endl;
			}
		}
	
	
    }
    return 0;
}
