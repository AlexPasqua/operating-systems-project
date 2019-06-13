#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "semaphores.h"
#include "../inc/errExit.h"

// implementazione funzioni ----------------------------------------------------
int crt_semaphores(char *calling_prog, short type){
  char key_proj, *key_pathname = "";
  unsigned short nsems, *values;

  // in caso la funz sia chiamata da clientExec devo aggiungere un pezzo al path
  if (strcmp(calling_prog, "clientExec") == 0)
    strcat(key_pathname, "../clientReq-server/");


  switch (type){
    case SEMTYPE_FIFO:
      strcat(key_pathname, "src/semaphores.c");
      key_proj = 'a';
      nsems = 2;
      values = (unsigned short *) malloc(nsems * sizeof(unsigned short));
      values[0] = 0;
      values[1] = 1;
      break;

    case SEMTYPE_SHM:
      strcat(key_pathname, "src/server.c");
      key_proj = 'b';
      nsems = 1;
      values = (unsigned short *) malloc(sizeof(unsigned short));
      *values = 1;
      break;

    default: errExit("Bad semaphore type");
  }


  key_t key = ftok(key_pathname, key_proj);
  if (key == -1)
    errExit(strcat(calling_prog, ": ftok failed (in semaphores.c)"));

  int semid = semget(key, nsems, IPC_CREAT | S_IRUSR | S_IWUSR);
  if (semid == -1)
    errExit(strcat(calling_prog, ": semget failed"));

  union semun arg;
  arg.array = values;
  if (semctl(semid, 0, SETALL, arg) == -1)
    errExit(strcat(calling_prog, ": semctl failed"));

  free(values);
  return semid;
}

//------------------------------------------------------------------------------
void semOp (int semid, unsigned short sem_num, short sem_op) {
  struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

  if (semop(semid, &sop, 1) == -1)
    errExit("semop failed");
}
