#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errExit.h"


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

    // inserimento user_id
    char user_id[50];
    printf("\n\nInserire codice identificativo utente: ");
    scanf("%s", user_id);

    // selezione servizio
    char service[50];
    do {
      printf("Inserire il nome del servizio richiesto: ");
      scanf("%s", service);
    }
    while (!check_service_input(service));


    // creo FIFOCLIENT, apro FIFOSERVER, apro FIFOCLIENT
    char *fifocli_pathname = "/tmp/FIFOCLIENT";
    char *fifoserv_pathname = "/tmp/FIFOSERVER";
    if (mkfifo(fifocli_pathname, S_IRUSR | S_IWUSR) == -1)
      errExit("mkfifo (FIFOCLIENT) failed");

    int fifoserver = open(fifoserv_pathname, O_WRONLY);
    if (fifoserver == -1)
      errExit("ClientReq failed to open FIFOSERVER in write-only mode");

    int fifoclient = open(fifocli_pathname, O_RDONLY);
    if (fifoclient == -1)
      errExit("ClientReq failed to open FIFOCLIENT in read-only mode");


    /*
     *
     *
     * COMUNICAZIONE CLIENT - SERVER
     *
     *
    */


    // chiudo FIFOSERVER, chiudo ed elimino FIFOCLIENT
    if (close(fifoserver) == -1)
      errExit("ClientReq failed to close FIFOSERVER");

    if (close(fifoclient) == -1)
      errExit("ClientReq failed to close FIFOCLIENT");

    if (unlink(fifocli_pathname) != 0)
      errExit("ClientReq failed to unlink FIFOCLIENT");

    return 0;
}
