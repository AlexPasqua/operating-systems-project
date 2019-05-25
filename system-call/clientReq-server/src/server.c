#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "errExit.h"
#include "myfifo.h"
#include "semaphores.h"


int main (int argc, char *argv[]) {
  // blocco tutti i signal tranne SIGTERM
  sigset_t signal_set;
  if (sigfillset(&signal_set) == -1)
    errExit("sigfillset failed");

  if (sigdelset(&signal_set, SIGTERM) == -1)
    errExit("sigdelset failed");

  if (sigprocmask(SIG_SETMASK, &signal_set, NULL) == -1)
    errExit("sigprocmask failed");


  // creo un insieme di semafori per gestire la comunicaz su FIFO
  key_t sem_key = ftok("semaphores.c", 'a');
  if (sem_key == -1)
    errExit("Server failed to create a key for the semaphores set");

  int semid = semget(sem_key, 2, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
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
  while (1){
    // apro FIFOCLIENT
    int fifoclient = open(fifocli_pathname, O_WRONLY);
    if (fifoclient == -1)
      errExit("Server failed to open FIFOCLIENT in write-only mode");

    // TEST
    char buf[USRID_STRDIM];
    read(fifoserver, buf, USRID_STRDIM);
    printf("%s\n", buf);

     // chiudo FIFOCLIENT
     if (close(fifoclient) == -1)
       errExit("Server failed to close FIFOCLIENT");

      //TEST
      sleep(10);
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
