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

#include "errExit.h"
#include "myfifo.h"
#include "semaphores.h"


// funz per controllare l'inserimento del servizio richiesto
bool check_service_input(char *service){
  char *avail_servs[] = {"stampa", "salva", "invia"};

  for (int i = 0; i < 3; i++)
    if (strcmp(service, avail_servs[i]) == 0)
      return true;

  printf("\nErr. Il nome va scelto tra \"stampa\" - \"salva\" - \"invia\"\n");
  return false;
}


int main (int argc, char *argv[]) {
    printf("Benvenuto! I servizi disponibili sono i seguenti:\n - stampa\n - salva\n - invia");

    struct Request req;

    // inserimento user
    printf("\n\nInserire codice identificativo utente: ");
    scanf("%s", req.user);

    // selezione servizio
    do {
      printf("Inserire il nome del servizio richiesto: ");
      scanf("%s", req.service);
    }
    while (!check_service_input(req.service));


    // ottengo l'insieme di semafori creato dal server
    key_t sem_key = ftok("src/semaphores.c", 'a');
    if (sem_key == -1)
      errExit("Client failed to create a key fot the semaphores set");

    int semid = semget(sem_key, 2, S_IRUSR | S_IWUSR);
    if (semid == -1)
      errExit("Client failed to perform semget");


    semOp(semid, CLIMUTEX, -1); //blocco gli altri client mentre uno sta comunicando


    // creo FIFOCLIENT, apro FIFOSERVER, apro FIFOCLIENT
    char *fifocli_pathname = "/tmp/FIFOCLIENT";
    char *fifoserv_pathname = "/tmp/FIFOSERVER";
    if (mkfifo(fifocli_pathname, S_IRUSR | S_IWUSR) == -1)
      errExit("mkfifo (FIFOCLIENT) failed");

    int fifoserver = open(fifoserv_pathname, O_WRONLY);
    if (fifoserver == -1)
      errExit("ClientReq failed to open FIFOSERVER in write-only mode");

    semOp(semid, SRVSEM, 1);  //sblocco il server in attesa di FIFOCLIENT
    int fifoclient = open(fifocli_pathname, O_RDONLY);
    if (fifoclient == -1)
      errExit("ClientReq failed to open FIFOCLIENT in read-only mode");



    // invio i dati al server
    if (write(fifoserver, &req, sizeof(struct Request)) != sizeof(struct Request))
      errExit("Client failed to write correctly on FIFOSERVER");



    // chiudo FIFOSERVER, chiudo ed elimino FIFOCLIENT
    if (close(fifoserver) == -1)
      errExit("ClientReq failed to close FIFOSERVER");

    if (close(fifoclient) == -1)
      errExit("ClientReq failed to close FIFOCLIENT");

    if (unlink(fifocli_pathname) != 0)
      errExit("ClientReq failed to unlink FIFOCLIENT");

    return 0;
}
