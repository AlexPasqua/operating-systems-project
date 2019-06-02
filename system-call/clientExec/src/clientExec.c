#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "../../clientReq-server/inc/errExit.h"
#include "../../clientReq-server/inc/sha_mem.h"
#include "../../clientReq-server/inc/semaphores.h"

//variabili globali
int semid;


//==============================================================================
void get_shm_semaphores(void){
  key_t sem_key = ftok("../clientReq-server/src/server.c", 'b');
  if (sem_key == -1)
    errExit("clientExec: ftok failed");

  semid = semget(sem_key, 1, S_IRUSR | S_IWUSR);
  if (semid == -1)
    errExit("clientExec: semget failed");
}

//==============================================================================
bool read_user_key(struct Entry *entry, char *cmp_user, server_k cmp_key){
  if (strcmp(entry->user, cmp_user) == 0 && entry->key == cmp_key)
    return true;

  return false;
}

//==============================================================================
server_k str_to_servk(char *str){
  server_k result = 0;

  for (int i = 0; str[i] != '\0'; i++){
    result += str[i] - 48;
    result *= 10;
  }

  return result / 10;
}

//==============================================================================
void close_all(void){
  // forse dovrai metterci il detach della shm che sta in fondo al main
  exit(EXIT_SUCCESS);
}

//==============================================================================
int main (int argc, char *argv[]) {
  // in questo caso è possibile che il servizio non abbia argomenti -> non farà nulla
  if (argc < 3)
    errExit("Usage: ./clientExec <user_id> <server_key> <args>");

  // imposto un exit handler
  if (atexit(close_all) != 0)
    _exit(EXIT_FAILURE);


  // get dei semafori per la memoria condivisa
  get_shm_semaphores();
  semOp(semid, 0, -1); //mi blocco (perché server ha già azzerato il semaforo)

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

  // leggi ma memoria condivisa
  bool found = false;
  unsigned int entry_idx = 0;
  for ( ; entry_idx < SHM_DIM; entry_idx++){
    if (read_user_key((shmptr + entry_idx), argv[1], str_to_servk(argv[2]))){
      found = true;
      break;
    }
  }

  switch (found){
    case true: {
      // rimuovo la entry dalla memoria condivisa
      strcpy((shmptr + entry_idx)->user, "");
      (shmptr + entry_idx)->key = (shmptr + entry_idx)->timestamp = 0;

      // sblocco il semaforo della memoria condivisa
      semOp(semid, 0, 1);
      break;
    }

    default: {
      printf("Coppia chiave-utente non valida\n");

      // sblocco il semaforo della memoria condivisa
      semOp(semid, 0, 1);

      // detach della mem condivisa
      if (shmdt(shmptr) == -1)
        errExit("clientReq: shmdt failed");

      break;
    }
  }

  return 0;
}
