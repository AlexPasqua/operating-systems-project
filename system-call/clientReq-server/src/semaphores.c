#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include "../inc/semaphores.h"
#include "../inc/errExit.h"

// implementazione funzioni ----------------------------------------------------
int get_semaphores(char *calling_proc, short type){
  /* controllo se il processo chiamante ha diritto di usare questa funzione,
  o se sia stato inserito un nome errato*/
  {
    const char *allowed_calling_procs[] = {"./server", "./clientReq", "./clientExec"};
    const long unsigned length = sizeof(allowed_calling_procs) / sizeof(const char *);

    for (int i = 0; i <= length; i++){
      if (i == length)
        errExit("Bad calling process' name");

      if (strcmp(calling_proc, allowed_calling_procs[i]) == 0)
        break;
    }
  }


  char key_pathname[100];
  char key_proj;
  unsigned short nsems, *values;
  int semget_flags = IPC_CREAT | S_IRUSR | S_IWUSR;

  // in caso non sia necessario CREARE i semafori, toglo IPC_CREAT da semget_flags
  if (strcmp(calling_proc, "./server") != 0)
    semget_flags = S_IRUSR | S_IWUSR;

  sprintf(key_pathname, "%s", "");
  if (strcmp(calling_proc, "./clientExec") == 0)
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
    errExit(strcat(calling_proc, ": ftok failed (in semaphores.c)"));

  int semid = semget(key, nsems, semget_flags);
  if (semid == -1)
    errExit(strcat(calling_proc, ": semget failed"));

  union semun arg;
  arg.array = values;
  if (semctl(semid, 0, SETALL, arg) == -1)
    errExit(strcat(calling_proc, ": semctl failed"));

  free(values);
  return semid;
}

//------------------------------------------------------------------------------
void semOp (int semid, unsigned short sem_num, short sem_op) {
  struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

  if (semop(semid, &sop, 1) == -1)
    errExit("semop failed");
}
