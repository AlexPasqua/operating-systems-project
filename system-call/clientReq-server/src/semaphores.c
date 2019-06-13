#include <sys/sem.h>
#include <errno.h>

#include "semaphore.h"
#include "../inc/errExit.h"

// implementazione funzioni ----------------------------------------------------
void crt_semaphores(short type, int *semid){
  switch (type){
    case SEMTYPE_FIFO: break;


    case SEMTYPE_SHM: break;


    default: break;
  }
}

//------------------------------------------------------------------------------
void semOp (int semid, unsigned short sem_num, short sem_op) {
  struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

  if (semop(semid, &sop, 1) == -1)
    errExit("semop failed");
}
