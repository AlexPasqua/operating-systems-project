#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

#include "errExit.h"
#include "myfifo.h"
#include "semaphores.h"

#define THOUSAND_BILLIONS 1000000000000


int main (int argc, char *argv[]) {
  printf("Server ready!\n\n");

  // blocco tutti i signal tranne SIGTERM
  sigset_t signal_set;
  if (sigfillset(&signal_set) == -1)
    errExit("sigfillset failed");

  if (sigdelset(&signal_set, SIGTERM) == -1)
    errExit("sigdelset failed");

  if (sigprocmask(SIG_SETMASK, &signal_set, NULL) == -1)
    errExit("sigprocmask failed");


  // creo un insieme di semafori per gestire la comunicaz su FIFO
  key_t sem_key = ftok("src/semaphores.c", 'a');
  if (sem_key == -1)
    errExit("Server failed to create a key for the semaphores set");

  int semid = semget(sem_key, 2, IPC_CREAT | /*IPC_EXCL |*/ S_IRUSR | S_IWUSR);
  if (semid == -1)
    errExit("Server failed to perform semget");

  unsigned short sem_values[2] = {0, 1}; /* il primo è per il server,
  il secondo è per mutua esclusione tra i client*/
  union semun arg;
  arg.array = sem_values;
  if (semctl(semid, 0/*ignored*/, SETALL, arg) == -1)
    errExit("Server failed to set semaphores values");


  // creo FIFOSERVER, apro FIFOSERVER
  char *fifoserv_pathname = "/tmp/FIFOSERVER";
  char *fifocli_pathname = "/tmp/FIFOCLIENT";
  if (mkfifo(fifoserv_pathname, S_IRUSR | S_IWUSR) == -1)
    errExit("mkfifo (FIFOSERVER) failed");

  int fifoserver = open(fifoserv_pathname, O_RDONLY);
  if (fifoserver == -1)
    errExit("Server failed to open FIFOSERVER in read-only mode");



  // continua a controllare richieste dei client
  char user[USR_STRDIM], service[SRV_STRDIM];
  struct Response resp;
  int bR;
  while (1){
    // blocco il server finché un client non crea FIFOCLIENT
    semOp(semid, SRVSEM, -1);

    // apro FIFOCLIENT
    int fifoclient = open(fifocli_pathname, O_WRONLY);
    if (fifoclient == -1)
      errExit("Server failed to open FIFOCLIENT in write-only mode");

    // leggo i dati dal server
    bR = read(fifoserver, user, USR_STRDIM);
    if (bR == -1) { errExit("Server failed to perdorm a read from FIFOSERVER"); }
    else if (bR != USR_STRDIM) { errExit("Looks like server didn't received a struct Request correctly"); }

    bR = read(fifoserver, service, SRV_STRDIM);
    if (bR == -1) { errExit("Server failed to perdorm a read from FIFOSERVER"); }
    else if (bR != SRV_STRDIM) { errExit("Looks like server didn't received a struct Request correctly"); }

    printf("%s - %s, sto generando una chiave di utilizzo...\n", user, service);

    /* genero la chiave:
     *  prendo il timestamp, accodo il numero corrispondente all'iniziale
     *  dello user, una cifra per il servizio (0=stampa, 1=salva, 2=invia)
     *  e una cifra casuale. Dopodiché elimino le prime 3 cifre (che sono sempre uguali)
     *
     *  per il servizio controllo solo i primi 2 caratteri di service
     *  (sono già sicuro che le stringhe siano corrette)
     */
    srand(time(NULL));
    /*resp.key = ((time(NULL) * 10000) + (user[0] * 1000) +
               (service[0] == 'i') ? 100 : ((service[1] == 't') ? 0 : 10) +
               (rand() % 10))
               % TEN_BILLIONS;*/
     resp.key = ((time(NULL) * 100000) + (user[0] * 100) +
                ((service[0] == 'i') ? 20 : ((service[1] == 't') ? 0 : 10)) + (rand() % 10))
                % THOUSAND_BILLIONS;

    if (write(fifoclient, &resp, sizeof(struct Response)) != sizeof(struct Response))
      errExit("Server failed to write on FIFOCLIENT");

    printf("KEY = %lu\n\n", resp.key);

    // chiudo FIFOCLIENT
    if (close(fifoclient) == -1)
      errExit("Server failed to close FIFOCLIENT");

    // sblocco un client in attesa
    semOp(semid, CLIMUTEX, 1);
  }



  // chiudo ed elimino FIFOSERVER
  if (close(fifoserver) == -1)
    errExit("Server failed to close FIFOSERVER");

  if (unlink(fifoserv_pathname) != 0)
    errExit("Server failed to unlink FIFOSERVER");

  // elimino il set di semafori
  if (semctl(semid, 0/*ignored*/, IPC_RMID, NULL) == -1)
    errExit("Server failed to remove semaphores set");

  return 0;
}
