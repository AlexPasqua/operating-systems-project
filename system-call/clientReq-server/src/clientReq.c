#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>

#include "errExit.h"
#include "myfifo.h"
#include "semaphores.h"

// dichiarazione funzioni
bool check_service_input(char *service);  // per controllare l'inserimento del servizio richiesto
void print_recap(struct Request, struct Response);  // funz per stampare il riepilogo dei dati
void close_all(void); // chiude/unlinka le FIFO
void sigHand(int sig);  // signal handler per SIGINT

//variabili globali
int fifoserver = -1, fifoclient = -1;
char *fifocli_pathname = "";


//==============================================================================
int main (int argc, char *argv[]) {
    // imposto un exit handler ed un signal handler per SIGINT (ctrl-C)
    if (atexit(close_all) != 0)
      errExit("ClientReq: atexit() failed");

    if (signal(SIGINT, sigHand) == SIG_ERR)
      errExit("ClientReq: signal() failed");


    printf("Benvenuto! I servizi disponibili sono i seguenti:\n - stampa\n - salva\n - invia");

    struct Request req;

    // inserimento dati ------------------------------------------
    // inserimento user
    printf("\n\nInserire codice identificativo utente: ");
    scanf("%s", req.user);

    // selezione servizio
    do {
      printf("Inserire il nome del servizio richiesto: ");
      scanf("%s", req.service);
    }
    while (!check_service_input(req.service));
    //------------------------------------------------------------


    // ottengo l'insieme di semafori creato dal server -----------
    key_t sem_key = ftok("src/semaphores.c", 'a');
    if (sem_key == -1)
      errExit("clientReq failed to create a key fot the semaphores set");

    int semid = semget(sem_key, 2, S_IRUSR | S_IWUSR);
    if (semid == -1)
      errExit("clientReq failed to perform semget");
    //------------------------------------------------------------


    semOp(semid, CLIMUTEX, -1); //blocco gli altri client mentre uno sta comunicando


    // creo FIFOCLIENT, apro FIFOSERVER, apro FIFOCLIENT ---------
    fifocli_pathname = "/tmp/FIFOCLIENT";
    char *fifoserv_pathname = "/tmp/FIFOSERVER";
    if (mkfifo(fifocli_pathname, S_IRUSR | S_IWUSR) == -1)
      errExit("mkfifo (FIFOCLIENT) failed");

    fifoserver = open(fifoserv_pathname, O_WRONLY);
    if (fifoserver == -1)
      errExit("ClientReq failed to open FIFOSERVER in write-only mode");

    semOp(semid, SRVSEM, 1);  //sblocco il server in attesa di FIFOCLIENT

    fifoclient = open(fifocli_pathname, O_RDONLY);
    if (fifoclient == -1)
      errExit("ClientReq failed to open FIFOCLIENT in read-only mode");
    //------------------------------------------------------------


    // invio i dati al server ------------------------------------
    if (write(fifoserver, &req, sizeof(struct Request)) != sizeof(struct Request))
      errExit("clientReq failed to write correctly on FIFOSERVER");


    // leggo la risposta del server (chiave) ---------------------
    struct Response resp;
    int bR = read(fifoclient, &resp, sizeof(struct Response));
    if (bR == -1) { errExit("Client failed to read key from FIFOCLIENT"); }
    else if (bR != sizeof(struct Response)) { errExit("Looks like clientReq didn't received a key correctly"); }

    print_recap(req, resp); //stampa riepilogo dati
    //------------------------------------------------------------

    exit(EXIT_SUCCESS);

    return 0;
}


//==============================================================================
bool check_service_input(char *service){
  char *services[] = {"stampa", "salva", "invia"};

  for (int i = 0; i < 3; i++)
    if (strcmp(service, services[i]) == 0)
      return true;

  printf("\nErr. Il nome va scelto tra \"stampa\" - \"salva\" - \"invia\"\n");
  return false;
}

//==============================================================================
void print_recap(struct Request req, struct Response resp){
  printf("\n\ncodice identificativo: %s\nservizio: %s\n", req.user, req.service);
  printf("chiave rilasciata dal server: %lu\n\n", resp.key);
}

//==============================================================================
void sigHand(int sig){
  printf("\n\nClosing ClientReq...\n");
  exit(EXIT_SUCCESS);
}

//==============================================================================
void close_all(){
  // chiudo FIFOSERVER, chiudo ed elimino FIFOCLIENT
  if (close(fifoserver) == -1 && fifoserver != -1)  // fifoserver != -1 indica che è stata aperta la FIFO
    errExit("ClientReq failed to close FIFOSERVER");

  if (close(fifoclient) == -1 && fifoclient != -1)
    errExit("ClientReq failed to close FIFOCLIENT");

  if (unlink(fifocli_pathname) != 0 && strcmp(fifocli_pathname, "") != 0)
    errExit("ClientReq failed to unlink FIFOCLIENT");
}
