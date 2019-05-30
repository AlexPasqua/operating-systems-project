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
#include <sys/shm.h>
#include <sys/wait.h>

#include "errExit.h"
#include "myfifo.h"
#include "semaphores.h"
#include "sha_mem.h"

#define HUNDREAD_THOUSANDS 100000
#define THOUSAND_BILLIONS 1000000000000

// variabili globali
int fifoserver, fifoclient, fifosem_id, shmsem_id, shmid;
char *fifoserv_pathname;
struct Entry *shmptr;
/*sono glibali perché servono in "close_all()", ma non posso passarle per
argomento perché la funzione è chiamata da un signal handler che per costruzione
ha un solo argomento (il signal)*/


//==============================================================================
// imposta la mask dei signal del processo
void set_sigprocmask(sigset_t *signal_set, int sig_to_allow){

  if (sigfillset(signal_set) == -1)
    errExit("sigfillset failed");

  if (sigdelset(signal_set, SIGTERM) == -1)
    errExit("sigdelset failed");

  if (sigprocmask(SIG_SETMASK, signal_set, NULL) == -1)
    errExit("sigprocmask failed");

}

//==============================================================================
// crea il segmento di memoria condivisa e fa l'attach
void crt_shm_segment(void){
  key_t key = ftok("src/server.c", 'a');
  if (key == -1)
    errExit("Server failed to create a key for the shared mem segment");

  // "shmid" e "shmptr" sono variabili globali

  shmid = shmget(key, SHM_DIM * sizeof(struct Entry), IPC_CREAT | S_IRUSR | S_IWUSR);
  if (shmid == -1)
    errExit("Server: shmget failed");

  // "attach" della memoria condivisa
  shmptr = (struct Entry *) shmat(shmid, NULL, 0);
  if (shmptr == (void *)(-1))
    errExit("Server: shmat failed");
}

//==============================================================================
void crt_fifo_semaphores(void){
  key_t key = ftok("src/semaphores.c", 'a');
  if (key == -1)
    errExit("Server failed to create a key for the fifo semaphores set");

  // "fifosem_id" è una variabile globale
  fifosem_id = semget(key, 2, IPC_CREAT | S_IRUSR | S_IWUSR);
  if (fifosem_id == -1)
    errExit("Server failed to perform semget (fifo sems)");

  unsigned short sem_values[2] = {0, 1}; /* il primo è per il server,
  il secondo è per mutua esclusione tra i client*/
  union semun arg;
  arg.array = sem_values;
  if (semctl(fifosem_id, 0/*ignored*/, SETALL, arg) == -1)
    errExit("Server failed to set fifoes' semaphores values");
}

//==============================================================================
void generate_key(struct Response *response, struct Request *client_data){
  /* genero la chiave:
   *  prendo il timestamp, accodo il numero corrispondente all'iniziale
   *  dello user, una cifra per il servizio (0=stampa, 1=salva, 2=invia)
   *  e una cifra casuale. Dopodiché elimino le prime 3 cifre (che sono sempre uguali
   *  perché cambiano al passare di mesi/anni)
   *
   *  per il servizio controllo solo i primi 2 caratteri di service
   *  (sono già sicuro che le stringhe siano corrette)
   */
  unsigned long timestamp = time(NULL);
  srand(timestamp);
  response->key = ((timestamp * HUNDREAD_THOUSANDS) + (client_data->user[0] * 100) +
             ((client_data->service[0] == 'i') ? 20 : ((client_data->service[1] == 't') ? 0 : 10)) +
             (rand() % 10)) % THOUSAND_BILLIONS;
}

//==============================================================================
void crt_shm_semaphores(){
  key_t key = ftok("src/server.c", 'b');
  if (key == -1)
    errExit("Server failed to create a key for the shm semaphores set");

  // (shmsem_id var globale)
  shmsem_id = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
  if (shmsem_id == -1)
    errExit("Server failed to perform semget (shm sems)");

  union semun arg;
  arg.val = 0;
  if (semctl(shmsem_id, 0, SETVAL, arg) == -1)
    errExit("Server failed to set shm's semaphore values");
}

//==============================================================================
// funz per le operazioni pre-chiusura del processo (signal handler)
void close_all(int sig){
  // chiudo FIFOCLIENT
  close(fifoclient);  //niente controllo perché potrebbe essere già stata chiusa nel while

  // chiudo ed elimino FIFOSERVER
  if (close(fifoserver) == -1)
    errExit("Server failed to close FIFOSERVER");

  if (unlink(fifoserv_pathname) != 0)
    errExit("Server failed to unlink FIFOSERVER");


  // elimino il set di semafori per le FIFO
  if (semctl(fifosem_id, 0/*ignored*/, IPC_RMID, NULL) == -1)
    errExit("Server failed to remove fifoes' semaphores set");

  // aspetto la terminazione di KeyManager per non farlo diverntare orfano
  /*if (waitpid(km_pid, NULL, 0) == -1)
    errExit("Server: waitpid() failed");*/

  // detach & delete memoria condivisa
  if (shmdt(shmptr) != 0)
    errExit("Server: shmdt failed");

  if (shmctl(shmid, IPC_RMID, NULL) != 0)
    errExit("Server failed to delete shared memory segment");

  // elimino il set di semafori per la memoria condivisa
  if (semctl(shmsem_id, 0, IPC_RMID, NULL) == -1)
    errExit("Server failed to remove shm's semaphores set");

  exit(1);
}

//==============================================================================
int main (int argc, char *argv[]) {
  printf("Server ready!\n\n");

  // blocco tutti i signal tranne SIGTERM
  sigset_t signal_set;
  set_sigprocmask(&signal_set, SIGTERM);

  // creo il segmento di memoria condivisa
  crt_shm_segment();
  /* Dati importanti in variabili globali:
   * shared memory ID: shmid
   * puntatore alla prima struct della memoria condivisa: shmptr */


  // CREO KEYMANAGER ---------------------------------------------
  pid_t km_pid = fork();

  if (km_pid == -1)
    errExit("Server: fork() failed");

  else if (km_pid == 0){
    //----KEY MANAGER SECTION
    while (1);
  }
  else{
    //----PARENT SECTION

    // imposto il signal handler per SIGTERM
    if (signal(SIGTERM, close_all) == SIG_ERR)
      errExit("Server: signal handler setting failed");

    // creo un insieme di semafori per gestire la comunicaz su FIFO
    crt_fifo_semaphores();

    // creo e apro FIFOSERVER --------------------------------------
    fifoserv_pathname = "/tmp/FIFOSERVER";  // (var globale)
    char *fifocli_pathname = "/tmp/FIFOCLIENT";
    if (mkfifo(fifoserv_pathname, S_IRUSR | S_IWUSR) == -1)
      errExit("mkfifo (FIFOSERVER) failed");

    fifoserver = open(fifoserv_pathname, O_RDONLY);
    if (fifoserver == -1)
      errExit("Server failed to open FIFOSERVER in read-only mode");
    //--------------------------------------------------------------


    // continua a controllare richieste dei client------------------------------
    struct Request client_data;
    struct Response resp;
    int bR/*, entry_idx = 0*/;
    while (1){
      // blocco il server finché un client non crea FIFOCLIENT
      semOp(fifosem_id, SRVSEM, -1);

      // apro FIFOCLIENT
      fifoclient = open(fifocli_pathname, O_WRONLY);
      if (fifoclient == -1)
        errExit("Server failed to open FIFOCLIENT in write-only mode");

      // leggo i dati da fifoserver
      bR = read(fifoserver, &client_data, sizeof(struct Request));
      if (bR == -1) { errExit("Server failed to perdorm a read from FIFOSERVER"); }
      else if (bR != sizeof(struct Request)) { errExit("Looks like server didn't received a struct Request correctly"); }

      printf("%s - %s, sto generando una chiave di utilizzo...\n", client_data.user, client_data.service);

      //genero la chiave di utilizzo
      generate_key(&resp, &client_data);


      //creo i semafori per la memoria condivisa
      crt_shm_semaphores();


      // rispondo al client
      if (write(fifoclient, &resp, sizeof(struct Response)) != sizeof(struct Response))
        errExit("Server failed to write on FIFOCLIENT");

      printf("KEY = %lu\n\n", resp.key);

      // chiudo FIFOCLIENT
      if (close(fifoclient) == -1)
        errExit("Server failed to close FIFOCLIENT");

      // sblocco un client in attesa
      semOp(fifosem_id, CLIMUTEX, 1);
    } //------------------------------------- chiusura while -------------------

    /* non si esce mai da while a meno che non arrivi un SIGTERM, ma in tal caso
    il processo termina dopo aver eseguito close_all (signal handler) */
  }

  return 0;
}
