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
bool read_user_key(struct Entry*, char*, server_k); //controlla se esiste una certa coppia chiave-utente
server_k str_to_servk(char*); //trasforma una chiave da char* a server_k


//==============================================================================
int main (int argc, char *argv[]) {
  // in questo caso è possibile che il servizio non abbia argomenti -> non farà nulla
  if (argc < 3)
    errExit("Usage: ./clientExec <user_id> <server_key> <args>");


  // get dei semafori per la memoria condivisa
  int semid = get_semaphores(argv[0], SEMTYPE_SHM);
  //get_shm_semaphores();
  semOp(semid, 0, -1);

  //get e attach delle memorie condivise--------------------------
  key_t infoshm_key = ftok("../clientReq-server/src/shmem.c", 'a');
  if (infoshm_key == -1) errExit("ClientExec: ftok (infoshm_key) failed");

  int infoshm_id = shmget(infoshm_key, 0, S_IRUSR | S_IWUSR);
  if (infoshm_id == -1) errExit("clientExec: shmget (infoshm_id) failed");

  struct my_shm_info *info_ptr = (struct my_shm_info *) shmat(infoshm_id, NULL, 0);
  if (info_ptr == (void *)(-1)) errExit("clientExec: shmat (info_ptr) failed");

  char proj = info_ptr->key_proj;
  unsigned int SHM_DIM = info_ptr->SHM_DIM;


  key_t shm_key = ftok("../clientReq-server/src/server.c", proj);
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
  if (shmdt(shmptr) == -1 || shmdt(info_ptr) == -1)
    errExit("clientReq: shmdt failed");

  if (found){
    char *services[] = {"stampa", "salva", "invia"};
    short service_idx = argv[2][9] - 48;
    printf("Coppia chiave-utente corretta! Eseguo %s...\n\n", services[service_idx]);

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
