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
#include "../../clientReq-server/inc/shmem.h"
#include "../../clientReq-server/inc/semaphores.h"

// dichiarazione funzioni
void get_shm_semaphores(void); //ottiene il set di semafori per la memoria condivisa
bool read_user_key(struct Entry*, char*, server_k); //controlla se esiste una certa coppia chiave-utente
server_k str_to_servk(char*); //trasforma una chiave da char* a server_k

//variabili globali
int semid;


//==============================================================================
int main (int argc, char *argv[]) {
  // in questo caso è possibile che il servizio non abbia argomenti -> non farà nulla
  if (argc < 3)
    errExit("Usage: ./clientExec <user_id> <server_key> <args>");


  // get dei semafori per la memoria condivisa
  get_shm_semaphores();
  semOp(semid, 0, -1);

  //get e attach della memoria condivisa--------------------------
  key_t shm_key = ftok("../clientReq-server/src/server.c", 'a');
  if (shm_key == -1)
    errExit("ClientExec: ftok failed");

  int shmid = shmget(shm_key, 0, S_IRUSR | S_IWUSR);
  if (shmid == -1)
    errExit("clientExec: shmget failed");

  struct Entry *shmptr = (struct Entry *) shmat(shmid, NULL, 0);
  if (shmptr == (void *)(-1))
    errExit("clientExec: shmat failed");
  //--------------------------------------------------------------

  // leggi da memoria condivisa
  bool found = false;
  unsigned int entry_idx = 0;
  for ( ; entry_idx < SHM_DIM; entry_idx++){
    if (read_user_key((shmptr + entry_idx), argv[1], str_to_servk(argv[2]))){
      found = true;
      break;
    }
  }

  if (found){
    // rimuovo la entry dalla shm (basta azzerare la chiave perché il controllo è su quella)
    (shmptr + entry_idx)->key = 0;
  }

  // sblocco il semaforo della memoria condivisa
  semOp(semid, 0, 1);

  // detach della mem condivisa
  if (shmdt(shmptr) == -1)
    errExit("clientReq: shmdt failed");

  if (found){
    char *services[] = {"stampa", "salva", "invia"};
    short service_idx = argv[2][9] - 48;
    printf("Coppia chiave-utente corretta! Eseguo %s...\n", services[service_idx]);

    // lancio il programma desiderato --------------------------
    char *passing_args[argc - 1];

    passing_args[0] = (char *) services[service_idx];

    for (int i = 1; i < argc-2; i++)
      passing_args[i] = argv[i + 2];

    passing_args[argc - 2] = (char *)NULL;

    execv(services[service_idx], passing_args);
    errExit("ClientExec: excl failed");
    //----------------------------------------------------------
  }

  else
    printf("Coppia chiave-utente non presente in memoria\n");

  return 0;
}


//==============================================================================
void get_shm_semaphores(){
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
