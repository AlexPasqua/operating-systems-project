#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "shmem.h"

#define HUNDREAD_THOUSANDS 100000
#define THOUSAND_BILLIONS 1000000000000

// dichiarazione funzioni
void set_sigprocmask(sigset_t*, int);  // imposta la mask dei signal del processo
void crt_shm_segment(); // crea il segmento di memoria condivisa e fa l'attach
void generate_key(struct Response*, struct Request*); // genera la chiave di utilizzo
void keyman_sigHand(int); // signal handler del KeyManager
void close_all(int);  // funz per le operazioni pre-chiusura (signal handler del server)
void expand_shm();

// variabili globali
int fifoserver, fifoclient, fifosem_id, shmsem_id, shmid, infoshm_id;
char *fifoserv_pathname;
Entry *shmptr;
pid_t km_pid;
struct my_shm_info *info_ptr;


//==============================================================================
int main (int argc, char *argv[]) {
  printf("Server ready!\n\n");

  // blocco tutti i signal tranne SIGTERM
  sigset_t signal_set;
  set_sigprocmask(&signal_set, SIGTERM);

  //creo i semafori per la memoria condivisa
  shmsem_id = get_semaphores(argv[0], SEMTYPE_SHM);
  semOp(shmsem_id, 0, -1);

  // creo la struct in mem condivisa contenente i dati sulla shm principale
  key_t infoshm_key = ftok("src/shmem.c", 'a');
  if (infoshm_key == -1) errExit("Server: ftok (infoshm_key) failed");

  infoshm_id = shmget(infoshm_key, sizeof(struct my_shm_info), IPC_CREAT | S_IRUSR | S_IWUSR);
  if (infoshm_id == -1) errExit("Server: shmget (infoshm_id) failed");

  info_ptr = (struct my_shm_info *) shmat(infoshm_id, NULL, 0);
  if (info_ptr == (void *)(-1)) errExit("Server: shmat (info_ptr) failed");

  info_ptr->SHM_DIM = 350;
  info_ptr->key_proj = 'a';
  //-----------------------------------------------------------------

  // creo il segmento di memoria condivisa e faccio l'attach
  crt_shm_segment();
  /* Dati importanti in variabili globali:
   * shared memory ID: shmid
   * puntatore alla prima struct della memoria condivisa: shmptr */
  semOp(shmsem_id, 0, 1);


  // CREO KEYMANAGER ---------------------------------------------
  km_pid = fork();
  if (km_pid == -1)
    errExit("Server: fork() failed");

  else if (km_pid == 0){
    //----KEY MANAGER SECTION

    // modifico la signal mask per sbloccare la ricez di SIGALRM
    if (sigdelset(&signal_set, SIGALRM) == -1 || sigdelset(&signal_set, SIGUSR1) == -1)
      errExit("KeyManager: sigdelset failed");

    if (sigprocmask(SIG_SETMASK, &signal_set, NULL) == -1)
      errExit("KeyManager: sigprocmask failed");

    //imposto i signal handler per KeyManager
    if (signal(SIGTERM, keyman_sigHand) == SIG_ERR ||
        signal(SIGALRM, keyman_sigHand) == SIG_ERR ||
        signal(SIGUSR1, keyman_sigHand) == SIG_ERR)
    {
      errExit("KeyManager: signal handler setting failed");
    }
    //-----------------------------------------------------------

    while (1){
      alarm(30);
      pause();
    }
  }

  else{
    //----PARENT SECTION

    // imposto il signal handler per SIGTERM
    if (signal(SIGTERM, close_all) == SIG_ERR)
      errExit("Server: signal handler setting failed");

    // creo i semafori per gestire la comunicazione su FIFO
    fifosem_id = get_semaphores(argv[0], SEMTYPE_FIFO);

    // creo e apro FIFOSERVER --------------------------------------
    fifoserv_pathname = "/tmp/FIFOSERVER";  // (var globale)
    if (mkfifo(fifoserv_pathname, S_IRUSR | S_IWUSR) == -1)
      errExit("mkfifo (FIFOSERVER) failed");

    fifoserver = open(fifoserv_pathname, O_RDONLY);
    if (fifoserver == -1)
      errExit("Server failed to open FIFOSERVER in read-only mode");
    //--------------------------------------------------------------


    // continua a controllare richieste dei client------------------------------
    char *fifocli_pathname = "/tmp/FIFOCLIENT";
    struct Request client_data;
    struct Response resp;
    int bR;
    unsigned int offset = 0;

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


      // scrivo in memoria condivisa -----------------------------
      semOp(shmsem_id, 0, -1);
      int SHM_DIM = info_ptr->SHM_DIM;
      short times = 0;

      //trovo una entry libera
      for (offset = 0; ((shmptr + offset)->key != 0 && offset < SHM_DIM) || offset >= SHM_DIM; offset++){
        if (offset >= SHM_DIM){
          if (++times < 2){
            offset = -1; //viene incrementato alla fine del ciclo

            /* se entra nell'if significa che tutte le entry sono piene, quindi sblocco
            il semaforo della shm e mando un SIGALRM a KeyManager per vedere se sia
            possibile eliminare già subito qualche entry, poi riprovo la scrittura*/
            semOp(shmsem_id, 0, 1);
            if (kill(km_pid, SIGALRM) == -1)
              errExit("server: failed to send SIGALRM to KeyManager");

            semOp(shmsem_id, 0, -1);
          }

          else {
            expand_shm();
            break;
          }
        }
      }

      // una volta trovato il posto in cui scrivere, scrivo
      strcpy((shmptr + offset)->user, client_data.user);
      (shmptr + offset)->key = resp.key;
      (shmptr + offset)->timestamp = time(NULL);
      semOp(shmsem_id, 0, 1);
      times = 0;
      //----------------------------------------------------------


      // rispondo al client
      if (write(fifoclient, &resp, sizeof(struct Response)) != sizeof(struct Response))
        errExit("Server failed to write on FIFOCLIENT");

      printf("KEY = %lu\n\n", resp.key);

      // chiudo FIFOCLIENT
      if (close(fifoclient) == -1)
        errExit("Server failed to close FIFOCLIENT");
    } //------------------------------------- chiusura while -------------------

    /* non si esce mai da while a meno che non arrivi un SIGTERM, ma in tal caso
    il processo termina dopo aver eseguito close_all (signal handler) */
  }

  return 0;
}



//##############################################################################
void expand_shm(){
  // sono già protetto da semafori per l'accesso alla shm

  //imposta nuova dim e proj
  info_ptr->SHM_DIM *= 2;
  info_ptr->key_proj ++;
  unsigned int SHM_DIM = info_ptr->SHM_DIM;
  char proj = info_ptr->key_proj;


  //crea nuova shm
  key_t new_key = ftok("src/server.c", proj);
  if (new_key == -1) errExit("Server: ftok (in 'expand_shm') failed");

  int new_shmid = shmget(new_key, SHM_DIM * sizeof(Entry), IPC_CREAT | S_IRUSR | S_IWUSR);
  if (new_shmid == -1) errExit("Server: shmget (in 'expand_shm') failed");

  Entry *new_shmptr = (Entry *) shmat(new_shmid, NULL, 0);
  if (new_shmptr == (void *)(-1)) errExit("Server: shmat (in 'expand_shm') failed");


  //copia contenuto di quella vecchia
  for (unsigned int i = 0; i < (SHM_DIM / 2); i++){
    strcpy((new_shmptr + i)->user, (shmptr + i)->user);
    (new_shmptr + i)->key = (shmptr + i)->key;
    (new_shmptr + i)->timestamp = (shmptr + i)->timestamp;
  }


  //fai fare il detach al KeyManager (signal user) e l'attach di quella nuova
  if (kill(km_pid, SIGUSR1) == -1)
    errExit("Server: kill (SIGUSR1) failed");


  //detach di quella vecchia
  if (shmdt(shmptr) == -1) errExit("Server: shmdt (in 'expand_shm') failed");
  if (shmctl(shmid, IPC_RMID, NULL) == -1) errExit("Server: shmat (in 'expand_shm') failed");


  //imposto i nuovi puntatori
  shmid = new_shmid;
  shmptr = new_shmptr;
}

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
void crt_shm_segment(){
  key_t key = ftok("src/server.c", info_ptr->key_proj);
  if (key == -1)
    errExit("Server failed to create a key for the shared mem segment");

  shmid = shmget(key, info_ptr->SHM_DIM * sizeof(Entry), IPC_CREAT | S_IRUSR | S_IWUSR);
  if (shmid == -1)
    errExit("Server: shmget failed");

  // "attach" della memoria condivisa
  shmptr = (Entry *) shmat(shmid, NULL, 0);
  if (shmptr == (void *)(-1))
    errExit("Server: shmat failed");
}

//==============================================================================
// genera la chiave di utilizzo
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
// signal handler del KeyManager
void keyman_sigHand(int sig) {
  switch (sig){
    case SIGALRM: {
      cus_t time_limit = 300; //5min

      semOp(shmsem_id, 0, -1);
      del_old_entries(shmptr, time_limit, info_ptr->SHM_DIM);
      semOp(shmsem_id, 0, 1);

      break;
    }

    // signal inviato dal server durante l'espansione della shm
    case SIGUSR1: {
      if (shmdt(shmptr) == -1) errExit("KeyManager: shmdt failed");

      // non ho bisogno di semafori, il server blocca già tutti
      key_t key = ftok("src/server.c", info_ptr->key_proj);
      if (key == -1) errExit("KeyManager: ftok failed (while expanding shm)");

      shmid = shmget(key, 0, S_IRUSR | S_IWUSR);
      if (shmid == -1) errExit("KeyManager: shmget failed (while expanding shm)");

      shmptr = (Entry *) shmat(shmid, NULL, 0);
      if (shmptr == (void *)(-1)) errExit("KeyManager: shmat failed (while expanding shm)");

      break;
    }

    case SIGTERM:
      // detach memoria condivisa
      if (shmdt(shmptr) != 0 || shmdt(info_ptr) != 0)
        errExit("KeyManager: shmdt failed");

      exit(EXIT_SUCCESS);
      break;

    default: break;
  }
}

//==============================================================================
// funz per le operazioni pre-chiusura (signal handler del server)
void close_all(int sig){
  // inoltro SIGTERM al KeyManager
  if (kill(km_pid, SIGTERM) == -1)
    errExit("Server: kill failed");

  if (wait(NULL) == -1)
    errExit("Server: wait failed");

  // chiudo FIFOCLIENT
  close(fifoclient);  //niente controllo perché potrebbe essere già stata chiusa nel while

  // chiudo ed elimino FIFOSERVER
  close(fifoserver);
  /*if (close(fifoserver) == -1)
    errExit("Server failed to close FIFOSERVER");*/

  if (unlink(fifoserv_pathname) != 0)
    errExit("Server failed to unlink FIFOSERVER");

  // elimino il set di semafori per le FIFO
  if (semctl(fifosem_id, 0, IPC_RMID, NULL) == -1)
    errExit("Server failed to remove fifoes' semaphores set");

  // detach & delete memorie condivise
  if (shmdt(shmptr) != 0 || shmdt(info_ptr) != 0)
    errExit("Server: shmdt failed");

  if (shmctl(shmid, IPC_RMID, NULL) != 0 || shmctl(infoshm_id, IPC_RMID, NULL) != 0)
    errExit("Server failed to delete shared memory segment");

  // elimino il set di semafori per la memoria condivisa
  if (semctl(shmsem_id, 0, IPC_RMID, NULL) == -1)
    errExit("Server failed to remove shm's semaphores set");

  exit(EXIT_SUCCESS);
}
