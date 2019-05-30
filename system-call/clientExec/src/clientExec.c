#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include "../../clientReq-server/inc/errExit.h"
#include "../../clientReq-server/inc/sha_mem.h"

//variabili globali
int semid;


//==============================================================================
void get_shm_semaphores(){
  key_t sem_key = ftok("../../clientReq-server/src/server.c", 'b');
  if (sem_key == -1)
    errExit("clientExec: ftok failed");

  semid = semget(sem_key, 1, S_IRUSR | S_IWUSR);
  if (semid == -1)
    errExit("clientExec: semget failed");
}

//==============================================================================
int main (int argc, char *argv[]) {
  // in questo caso è possibile che il servizio non abbia argomenti -> non farà nulla
  if (argc < 3)
    errExit("Usage: ./clientExec <user_id> <server_key> <args>");


  //get e attach della memoria condivisa--------------------------
  key_t shm_key = ftok("../clientReq-server/src/server.c", 'a');
  if (shm_key == -1)
    errExit("ClientExec: ftok failed");

  int shmid = shmget(shm_key, 0/*ignored*/, S_IRUSR | S_IWUSR);
  if (shmid == -1)
    errExit("clientExec: shmget failed");

  struct Entry *shmptr = (struct Entry *) shmat(shmid, NULL, 0);
  if (shmptr == (void *)(-1))
    errExit("clientExec: shmat failed");
  //--------------------------------------------------------------


  //TO_DO -> get dei semafori per la memoria







  // FOR NOW -> close della mem condivisa
  if (shmdt(shmptr) == -1)
    errExit("clientReq: shmdt failed");

  return 0;
}
